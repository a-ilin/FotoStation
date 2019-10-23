/*
 * GNU General Public License (GPL)
 * Copyright (c) 2019 by Aleksei Ilin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "synoconn.h"
#include "synoerror.h"
#include "synoreplyjson.h"
#include "synorequest.h"
#include "synotraits.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QThread>
#include <QUrlQuery>

uint qHash(const QPointer<SynoRequest>& req)
{
    return reinterpret_cast<uint>(req.data());
}

SynoConn::SynoConn(QObject *parent)
    : QObject(parent)
    , m_status(SynoConn::DISCONNECTED)
    , m_isConnecting(false)
{
    m_apiPath = QByteArrayLiteral("webapi");
}

void SynoConn::connectToSyno(const QUrl& synoUrl)
{
    disconnectFromSyno();
    setIsConnecting(true);
    m_synoUrl = synoUrl;
    if (synoUrl.scheme() == QStringLiteral("https")) {
#ifndef QT_NO_SSL
        m_networkManager.connectToHostEncrypted(synoUrl.host(), static_cast<quint16>(synoUrl.port(443)));
#else
        setErrorString(tr("Cannot use encrypted connection: SSL not available."));
        return;
#endif
    } else {
        m_networkManager.connectToHost(synoUrl.host(), static_cast<quint16>(synoUrl.port(80)));
    }

    populateApiMap();
}

void SynoConn::disconnectFromSyno()
{
    cancelAllRequests();
    m_networkManager.clearConnectionCache();
    m_networkManager.clearAccessCache();
    setStatus(SynoConn::DISCONNECTED);
    setIsConnecting(false);
    m_synoToken.clear();
}

void SynoConn::authorize(const QString& username, const QString& password)
{
    QByteArrayList formData;
    formData << QByteArrayLiteral("method=login");
    formData << QByteArrayLiteral("version=1");
    formData << QByteArrayLiteral("enable_syno_token=true");
    formData << QByteArrayLiteral("username=") + username.toUtf8();
    formData << QByteArrayLiteral("password=") + password.toUtf8();
    formData << QByteArrayLiteral("remember_me=true");

    std::shared_ptr<SynoRequest> req = createRequest(QByteArrayLiteral("SYNO.PhotoStation.Auth"), formData);
    req->send(this, [this, req]() {
        auto failure = [this](const QString& reason) {
            this->setErrorString(tr("Authorization failed. %1").arg(reason));
        };

        if (req->errorString().isEmpty()) {
            SynoReplyJSON replyJSON(req.get());
            if (replyJSON.errorString().isEmpty()) {
                qInfo() << tr("Authorization successful");
                m_synoToken = replyJSON.dataObject()[QStringLiteral("sid")].toString().toUtf8();
                setStatus(SynoConn::AUTHORIZED);
            } else {
                failure(replyJSON.errorString());
            }
        } else {
            failure(req->errorString());
        }

        setIsConnecting(false);
    });
}

void SynoConn::checkAuth()
{
    QByteArrayList formData;
    formData << QByteArrayLiteral("method=checkauth");
    formData << QByteArrayLiteral("version=1");

    std::shared_ptr<SynoRequest> req = createRequest(QByteArrayLiteral("SYNO.PhotoStation.Auth"), formData);
    req->send(this, [this, req]() {
        if (!req->errorString().isEmpty()) {
            this->setErrorString(tr("Authorization check failed. %1").arg(req->errorString()));
        }

        setIsConnecting(false);
    });
}

SynoRequest* SynoConn::createRequest(const QString& api, const QStringList& formData)
{
    // this method should never be called outside of main thread
    Q_ASSERT(QThread::currentThread() == thread());

    QByteArrayList baFormData;
    baFormData.reserve(formData.size());
    std::for_each(formData.cbegin(), formData.cend(), [&](const QString& formEntry) {
        baFormData << formEntry.toUtf8();
    });
    return new SynoRequest(api.toLatin1(), baFormData, this);
}

std::shared_ptr<SynoRequest> SynoConn::createRequest(const QByteArray& api,
                                                     const QByteArrayList& formData)
{
    return std::shared_ptr<SynoRequest>(new SynoRequest(api, formData, this), [](QObject* o) {
        o->deleteLater();
    });
}

void SynoConn::sendRequest(SynoRequest* request)
{
    if (!request) {
        qWarning() << __FUNCTION__ << "nullptr is not allowed";
        return;
    }

    QString path = apiPath(request->api());
    if (path.isNull()) {
        request->setErrorString(tr("Unknown API"));
        return;
    }

    QUrl url(m_synoUrl);
    url.setPath(path);

    QUrlQuery urlQuery;

    if (!m_synoToken.isEmpty()) {
        request->request().setRawHeader(QByteArrayLiteral("X-SYNO-TOKEN"), m_synoToken);
        urlQuery.addQueryItem(QByteArrayLiteral("SynoToken"), m_synoToken);
    }

    url.setQuery(urlQuery);
    request->request().setUrl(url);
    request->request().setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/x-www-form-urlencoded"));

    QByteArray body;
    body.reserve(4096);
    body += QByteArrayLiteral("api=") + request->api();

    if (!m_synoToken.isEmpty()) {
        body += QByteArrayLiteral("&SynoToken=") + m_synoToken;
    }

    if (!request->formData().isEmpty()) {
        body += '&' + request->formData().join('&');
    }

    QNetworkReply* reply = m_networkManager.post(request->request(), body);
    request->setReply(reply);
    m_pendingRequests.insert(request);
    connect(reply, &QNetworkReply::finished, this, [this, request]() {
        m_pendingRequests.remove(request);
    });
}

void SynoConn::cancelRequest(SynoRequest* request)
{
    if (!request) {
        qWarning() << __FUNCTION__ << "nullptr is not allowed";
        return;
    }

    QNetworkReply* reply = request->reply();
    if (!reply) {
        qWarning() << __FUNCTION__ << "Nothing to cancel, the request has not been sent";
        return;
    }

    reply->abort();
    m_pendingRequests.remove(request);
}

void SynoConn::cancelAllRequests()
{
    for (SynoRequest* req : std::as_const(m_pendingRequests)) {
        if (!req) {
            qWarning() << __FUNCTION__ << "nullptr discovered, skipping";
            continue;
        }
        QNetworkReply* reply = req->reply();
        if (!reply) {
            qWarning() << __FUNCTION__ << "Nothing to cancel, the request has not been sent";
            return;
        }

        reply->abort();
    }

    m_pendingRequests.clear();
}

SynoConn::SynoConnStatus SynoConn::status() const
{
    return m_status;
}

bool SynoConn::isConnecting() const
{
    return m_isConnecting;
}

QStringList SynoConn::apiList() const
{
    QStringList apiList;
    apiList.reserve(m_apiMap.size());
    std::for_each(m_apiMap.keyBegin(), m_apiMap.keyEnd(), [&](const QByteArray& api) {
        apiList << QString::fromLatin1(api);
    });
    return apiList;
}

QString SynoConn::errorString() const
{
    return m_errorString;
}

void SynoConn::setErrorString(const QString& err)
{
    if (err != m_errorString) {
        m_errorString = err;
        emit errorStringChanged();
    }
}

QString SynoConn::apiPath(const QByteArray& api) const
{
    if (!m_apiMap.contains(api)) {
        return QString();
    }

    QStringList path;
    path << m_synoUrl.path()
         << m_apiPath
         << QString::fromLatin1(m_apiMap[api]);
    return path.join('/');
}

void SynoConn::populateApiMap()
{
    m_apiMap.clear();
    m_apiMap[QByteArrayLiteral("SYNO.API.Info")] = QByteArrayLiteral("query.php");
    emit apiListChanged();

    QByteArrayList formData;
    formData << QByteArrayLiteral("query=all");
    formData << QByteArrayLiteral("method=query");
    formData << QByteArrayLiteral("version=1");
    formData << QByteArrayLiteral("ps_username=");

    std::shared_ptr<SynoRequest> apiReq = createRequest(QByteArrayLiteral("SYNO.API.Info"), formData);
    apiReq->send(this, [this, apiReq]() {
        bool isSuccess = true;
        auto failure = [this, &isSuccess](const QString& reason) {
            isSuccess = false;
            this->setErrorString(tr("Cannot populate API map. %1").arg(reason));
        };

        if (apiReq->errorString().isEmpty()) {
            SynoReplyJSON replyJSON(apiReq.get());
            if (replyJSON.errorString().isEmpty()) {
                if (replyJSON.dataObject().size()) {
                    for (auto iter = replyJSON.dataObject().constBegin(); iter != replyJSON.dataObject().constEnd(); ++iter) {
                        QString path = iter.value().toObject()[QStringLiteral("path")].toString();
                        if (!path.isEmpty()) {
                            m_apiMap[iter.key().toUtf8()] = path.toUtf8();
                        } else {
                            // TODO: collect list of API to fail if empty
                            qWarning() << __FUNCTION__ << tr("Received empty path for API: ") << iter.key();
                        }
                    }
                } else {
                    failure(tr("Received empty API map"));
                }
            } else {
                failure(replyJSON.errorString());
            }
        } else {
            failure(apiReq->errorString());
        }

        if (isSuccess) {
            setStatus(SynoConn::API_LOADED);
        } else {
            setStatus(SynoConn::DISCONNECTED);
            setIsConnecting(false);
        }
    });
}

void SynoConn::setStatus(SynoConn::SynoConnStatus status)
{
    if (status != m_status) {
        m_status = status;
        emit statusChanged();
    }
}

void SynoConn::setIsConnecting(bool status)
{
    if (status != m_isConnecting) {
        m_isConnecting = status;
        emit isConnectingChanged();
    }
}
