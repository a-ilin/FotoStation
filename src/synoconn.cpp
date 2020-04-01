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

#include "synoconn_p.h"
#include "synoerror.h"
#include "synoreplyjson.h"
#include "synotraits.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QThread>
#include <QUrlQuery>

SynoConn::SynoConn(QObject* parent)
    : QObject(*(new SynoConnPrivate()), parent)
{
    Q_D(SynoConn);

    d->apiDir = QByteArrayLiteral("webapi");
    d->status = SynoConn::NONE;
    d->auth = new SynoAuth(&d->networkManager, this);
    d->sslConfig = new SynoSslConfig(&d->networkManager, this);
}

SynoConn::~SynoConn()
{
    disconnectFromSyno();
}

void SynoConn::connectToSyno(const QUrl& synoUrl)
{
    Q_D(SynoConn);

    disconnectFromSyno();

    d->sslConfig->clearErrors();

    if (d->synoUrl != synoUrl) {
        d->synoUrl = synoUrl;

        // reload SSL config
        d->sslConfig->loadExceptionsFromStorage();

        emit synoUrlChanged();
    }

    if (synoUrl.scheme() == QStringLiteral("https")) {
        if (d->sslConfig->isSslAvailable()) {
            d->sslConfig->connectToHostEncrypted(synoUrl.host(), static_cast<quint16>(synoUrl.port(443)));
        } else {
            d->setErrorString(tr("Cannot use encrypted connection: SSL not available."));
            return;
        }
    } else {
        d->networkManager.connectToHost(synoUrl.host(), static_cast<quint16>(synoUrl.port(80)));
    }

    d->sendApiMapRequest();
}

void SynoConn::disconnectFromSyno()
{
    Q_D(SynoConn);

    cancelAllRequests();
    d->networkManager.clearConnectionCache();
    d->networkManager.clearAccessCache();
    d->auth->closeSession(false);
    d->setStatus(SynoConn::NONE);
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
    Q_D(SynoConn);

    Q_ASSERT(request);

    QString path = d->pathForAPI(request->api());
    Q_ASSERT(!path.isEmpty());
    if (path.isNull()) {
        request->setErrorString(tr("Unknown API: %1").arg(QString::fromUtf8(request->api())));
        return;
    }

    QUrl url(d->synoUrl);
    url.setPath(path);

    QUrlQuery urlQuery;

    const QByteArray& token = d->auth->synoToken();

    if (!token.isEmpty()) {
        request->request().setRawHeader(QByteArrayLiteral("X-SYNO-TOKEN"), token);
        urlQuery.addQueryItem(QByteArrayLiteral("SynoToken"), token);
    }

    url.setQuery(urlQuery);
    request->request().setUrl(url);
    request->request().setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/x-www-form-urlencoded"));

    QByteArray body;
    body.reserve(512);
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

    QNetworkReply* reply = d->networkManager.post(request->request(), body);
    request->setReply(reply);

    d->pendingRequests.insert(request);
    connect(reply, &QNetworkReply::finished, this, std::bind(&SynoConnPrivate::onReplyFinished, d, request));
}

void SynoConn::cancelRequest(SynoRequest* request)
{
    Q_D(SynoConn);

    Q_ASSERT(request);

    if (QNetworkReply* reply = request->reply()) {
        d->pendingRequests.remove(request);
        reply->abort();
    } else {
        qWarning() << __FUNCTION__ << QStringLiteral("Nothing to cancel, the request has not been sent");
    }
}

void SynoConn::cancelAllRequests()
{
    Q_D(SynoConn);

    QSet< SynoRequest* > requests;
    std::swap(requests, d->pendingRequests);

    for (SynoRequest* req : std::as_const(requests)) {
        if (QNetworkReply* reply = req->reply()) {
            reply->abort();
        }
    }
}

SynoConn::SynoConnStatus SynoConn::status() const
{
    Q_D(const SynoConn);

    return d->status;
}

QStringList SynoConn::apiList() const
{
    Q_D(const SynoConn);

    QStringList apiList;
    apiList.reserve(d->apiMap.size());
    std::for_each(d->apiMap.keyBegin(), d->apiMap.keyEnd(), [&](const QByteArray& api) {
        apiList << QString::fromUtf8(api);
    });
    return apiList;
}

const QUrl& SynoConn::synoUrl() const
{
    Q_D(const SynoConn);

    return d->synoUrl;
}

const QString& SynoConn::errorString() const
{
    Q_D(const SynoConn);

    return d->errorString;
}

void SynoConnPrivate::setErrorString(const QString& err)
{
    Q_Q(SynoConn);

    if (err != errorString) {
        errorString = err;

        if (!errorString.isEmpty()) {
            qWarning() << errorString;
        }

        emit q->errorStringChanged();
    }
}

SynoSslConfig* SynoConn::sslConfig() const
{
    Q_D(const SynoConn);

    return d->sslConfig;
}

SynoAuth* SynoConn::auth() const
{
    Q_D(const SynoConn);

    return d->auth;
}

QString SynoConnPrivate::pathForAPI(const QByteArray& api) const
{
    QString apiPath = apiMap.value(api);
    QString fullPath;

    if (!apiPath.isEmpty()) {
        fullPath.reserve(256);

        fullPath += synoUrl.path();
        fullPath += '/';
        fullPath += apiDir;
        fullPath += '/';
        fullPath += apiPath;
    }

    return fullPath;
}

void SynoConnPrivate::sendApiMapRequest()
{
    Q_Q(SynoConn);

    setStatus(SynoConn::ATTEMPT_API);

    apiMap.clear();
    apiMap[QByteArrayLiteral("SYNO.API.Info")] = QStringLiteral("query.php");
    emit q->apiListChanged();

    QByteArrayList formData;
    formData << QByteArrayLiteral("query=all");
    formData << QByteArrayLiteral("method=query");
    formData << QByteArrayLiteral("version=1");
    formData << QByteArrayLiteral("ps_username=");

    std::shared_ptr<SynoRequest> req = q->createRequest(QByteArrayLiteral("SYNO.API.Info"), formData);
    req->send(q, [this, req]() {
        if (processApiMapReply(req.get())) {
            setStatus(SynoConn::API_LOADED);
        } else {
            setStatus(SynoConn::NONE);
        }
    });
}

bool SynoConnPrivate::processApiMapReply(const SynoRequest* req)
{
    Q_Q(SynoConn);

    auto failure = [this](const QString& reason) {
        setStatus(SynoConn::NONE);
        setErrorString(QObject::tr("Cannot populate API map. %1").arg(reason));
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
        failure(QObject::tr("Received empty API map"));
        return false;
    }

    for (auto iter = replyJSON.dataObject().constBegin(); iter != replyJSON.dataObject().constEnd(); ++iter) {
        QString path = iter.value().toObject()[QStringLiteral("path")].toString();
        if (!path.isEmpty()) {
            apiMap[iter.key().toUtf8()] = path;
        } else {
            // TODO: collect list of API to fail if empty
            qWarning() << __FUNCTION__ << QObject::tr("Received empty path for API: ") << iter.key();
        }
    }

    emit q->apiListChanged();

    return true;
}

void SynoConnPrivate::setStatus(SynoConn::SynoConnStatus neuStatus)
{
    Q_Q(SynoConn);

    if (neuStatus != status) {
        status = neuStatus;
        qDebug() << __FUNCTION__ << "Status:" << status;
        emit q->statusChanged();
    }
}

void SynoConnPrivate::onReplyFinished(SynoRequest* request)
{
    Q_Q(SynoConn);

    pendingRequests.remove(request);

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
            setErrorString(QObject::tr("Network fatal error: %1").arg(reply->error()));
            q->disconnectFromSyno();
            break;
        default:
            break;
        }
    }
}
