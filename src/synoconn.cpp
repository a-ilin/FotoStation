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

#include "synoauth.h"
#include "synoconn.h"
#include "synoerror.h"
#include "synoreplyjson.h"
#include "synorequest.h"
#include "synosslconfig.h"
#include "synotraits.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QThread>
#include <QUrlQuery>

SynoConn::SynoConn(QObject *parent)
    : QObject(parent)
    , m_status(SynoConn::NONE)
    , m_sslConfig(new SynoSslConfig(&m_networkManager, this))
    , m_auth(new SynoAuth(&m_networkManager, this))
{
    m_apiPath = QByteArrayLiteral("webapi");
}

SynoConn::~SynoConn()
{
    disconnectFromSyno();
}

void SynoConn::connectToSyno(const QUrl& synoUrl)
{
    disconnectFromSyno();

    m_sslConfig->clearErrors();

    if (m_synoUrl != synoUrl) {
        m_synoUrl = synoUrl;

        // reload SSL config
        m_sslConfig->loadExceptionsFromStorage();

        emit synoUrlChanged();
    }

    if (synoUrl.scheme() == QStringLiteral("https")) {
        if (m_sslConfig->isSslAvailable()) {
            m_sslConfig->connectToHostEncrypted(synoUrl.host(), static_cast<quint16>(synoUrl.port(443)));
        } else {
            setErrorString(tr("Cannot use encrypted connection: SSL not available."));
            return;
        }
    } else {
        m_networkManager.connectToHost(synoUrl.host(), static_cast<quint16>(synoUrl.port(80)));
    }

    sendApiMapRequest();
}

void SynoConn::disconnectFromSyno()
{
    cancelAllRequests();
    m_networkManager.clearConnectionCache();
    m_networkManager.clearAccessCache();
    m_auth->closeSession(false);
    setStatus(SynoConn::NONE);
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
    Q_ASSERT(request);
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

    const QByteArray& token = m_auth->synoToken();

    if (!token.isEmpty()) {
        request->request().setRawHeader(QByteArrayLiteral("X-SYNO-TOKEN"), token);
        urlQuery.addQueryItem(QByteArrayLiteral("SynoToken"), token);
    }

    url.setQuery(urlQuery);
    request->request().setUrl(url);
    request->request().setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/x-www-form-urlencoded"));

    QByteArray body;
    body.reserve(4096);
    body += QByteArrayLiteral("api=") + request->api();

    if (!token.isEmpty()) {
        body += QByteArrayLiteral("&SynoToken=") + token;
    }

    for (const QByteArray& formField : std::as_const(request->formData())) {
        body += '&';
        body += formField;
    }

    // invalidate old connection
    if (QNetworkReply* reply = request->reply()) {
        QObject::disconnect(reply, nullptr, this, nullptr);
    }

    QNetworkReply* reply = m_networkManager.post(request->request(), body);
    request->setReply(reply);

    m_pendingRequests.insert(request);
    connect(reply, &QNetworkReply::finished, this, std::bind(&SynoConn::onReplyFinished, this, request));
}

void SynoConn::cancelRequest(SynoRequest* request)
{
    Q_ASSERT(request);
    if (!request) {
        qWarning() << __FUNCTION__ << "nullptr is not allowed";
        return;
    }

    QNetworkReply* reply = request->reply();
    if (!reply) {
        qWarning() << __FUNCTION__ << "Nothing to cancel, the request has not been sent";
        return;
    }

    m_pendingRequests.remove(request);
    reply->abort();
}

void SynoConn::cancelAllRequests()
{
    QSet< SynoRequest* > requests;
    std::swap(requests, m_pendingRequests);

    for (SynoRequest* req : std::as_const(requests)) {
        if (QNetworkReply* reply = req->reply()) {
            reply->abort();
        }
    }
}

SynoConn::SynoConnStatus SynoConn::status() const
{
    return m_status;
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

const QUrl& SynoConn::synoUrl() const
{
    return m_synoUrl;
}

const QString& SynoConn::errorString() const
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

SynoSslConfig* SynoConn::sslConfig() const
{
    return m_sslConfig;
}

SynoAuth* SynoConn::auth() const
{
    return m_auth;
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

void SynoConn::sendApiMapRequest()
{
    setStatus(SynoConn::ATTEMPT_API);

    m_apiMap.clear();
    m_apiMap[QByteArrayLiteral("SYNO.API.Info")] = QByteArrayLiteral("query.php");
    emit apiListChanged();

    QByteArrayList formData;
    formData << QByteArrayLiteral("query=all");
    formData << QByteArrayLiteral("method=query");
    formData << QByteArrayLiteral("version=1");
    formData << QByteArrayLiteral("ps_username=");

    std::shared_ptr<SynoRequest> req = createRequest(QByteArrayLiteral("SYNO.API.Info"), formData);
    req->send(this, [this, req]() {
        if (processApiMapReply(req.get())) {
            setStatus(SynoConn::API_LOADED);
        } else {
            setStatus(SynoConn::NONE);
        }
    });
}

bool SynoConn::processApiMapReply(const SynoRequest* req)
{
    auto failure = [this](const QString& reason) {
        setStatus(SynoConn::NONE);
        this->setErrorString(tr("Cannot populate API map. %1").arg(reason));
        qDebug() << errorString();
    };

    if (!req->errorString().isEmpty()) {
        failure(req->errorString());
        return false;
    }

    SynoReplyJSON replyJSON(req);
    if (!replyJSON.errorString().isEmpty()) {
        failure(replyJSON.errorString());
        return false;
    }

    const QJsonObject& dataObject = replyJSON.dataObject();

    if (!dataObject.size()) {
        failure(tr("Received empty API map"));
        return false;
    }

    for (auto iter = replyJSON.dataObject().constBegin(); iter != replyJSON.dataObject().constEnd(); ++iter) {
        QString path = iter.value().toObject()[QStringLiteral("path")].toString();
        if (!path.isEmpty()) {
            m_apiMap[iter.key().toUtf8()] = path.toUtf8();
        } else {
            // TODO: collect list of API to fail if empty
            qWarning() << __FUNCTION__ << tr("Received empty path for API: ") << iter.key();
        }
    }

    emit apiListChanged();

    return true;
}

void SynoConn::setStatus(SynoConn::SynoConnStatus status)
{
    if (status != m_status) {
        m_status = status;
        qDebug() << __FUNCTION__ << "Status:" << m_status;
        emit statusChanged();
    }
}

void SynoConn::onReplyFinished(SynoRequest* request)
{
    m_pendingRequests.remove(request);

    QNetworkReply* reply = request->reply();

    if (reply->error() != QNetworkReply::NoError) {
        switch (reply->error()) {
        // fatal network layer errors:
        case QNetworkReply::ConnectionRefusedError:
        case QNetworkReply::RemoteHostClosedError:
        case QNetworkReply::HostNotFoundError:
        case QNetworkReply::TimeoutError:
        case QNetworkReply::SslHandshakeFailedError:
        case QNetworkReply::NetworkSessionFailedError:
        case QNetworkReply::BackgroundRequestNotAllowedError:
        case QNetworkReply::UnknownNetworkError:
        // fatal proxy errors:
        case QNetworkReply::ProxyConnectionRefusedError:
        case QNetworkReply::ProxyConnectionClosedError:
        case QNetworkReply::ProxyNotFoundError:
        case QNetworkReply::ProxyTimeoutError:
        case QNetworkReply::ProxyAuthenticationRequiredError:
        case QNetworkReply::UnknownProxyError:
            qWarning() << __FUNCTION__ << "Network fatal error:" << reply->error();
            disconnectFromSyno();
            break;
        default:
            break;
        }
    }
}
