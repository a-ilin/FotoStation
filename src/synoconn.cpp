/*
 * The MIT License (MIT)
 * Copyright (c) 2019 by Aleksei Ilin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "synoconn.h"

SynoConn::SynoConn(QObject *parent)
    : QObject(parent)
    , m_status(SynoConn::DISCONNECTED)
    , m_isConnecting(false)
{
    m_apiPath = "webapi";
}

void SynoConn::connectToSyno(const QUrl& synoUrl)
{
    disconnectFromSyno();
    setIsConnecting(true);
    m_synoUrl = synoUrl;
    if (synoUrl.scheme() == "https") {
        m_networkManager.connectToHostEncrypted(synoUrl.host(), static_cast<quint16>(synoUrl.port(443)));
    } else {
        m_networkManager.connectToHost(synoUrl.host(), static_cast<quint16>(synoUrl.port(80)));
    }

    populateApiMap();
}

void SynoConn::disconnectFromSyno()
{
    for (QNetworkReply* reply : m_pendingReplies) {
        reply->abort();
        reply->deleteLater();
    }
    m_pendingReplies.clear();
    m_networkManager.clearConnectionCache();
    m_networkManager.clearAccessCache();
    setStatus(SynoConn::DISCONNECTED);
    setIsConnecting(false);
}

void SynoConn::authorize(const QString& username, const QString& password)
{
    QByteArrayList formData;
    formData << "method=login";
    formData << "version=1";
    formData << "enable_syno_token=true";
    formData << "username=" + username.toUtf8();
    formData << "password=" + password.toUtf8();

    sendRequest("SYNO.PhotoStation.Auth", formData, [=](QNetworkReply* /*reply*/, const QJsonDocument& json) {
        QJsonValue jsonSuccess = json.object()["success"];
        bool authSuccess = jsonSuccess.toBool();
        if (authSuccess) {
            qWarning() << "Authorization successful!";
            setStatus(SynoConn::AUTHORIZED);
        } else {
            qWarning() << "Authorization failed!";
        }
        setIsConnecting(false);
    }, [=](QNetworkReply* /*reply*/, const QJsonDocument& /*json*/) {
            qWarning() << "Authorization failed!";
            setIsConnecting(false);
        });
}

SynoConn::SynoConnStatus SynoConn::status() const
{
    return m_status;
}

bool SynoConn::isConnecting() const
{
    return m_isConnecting;
}

void SynoConn::sendRequest(const QByteArray& api,
                         const QByteArrayList& formData,
                         std::function<RequestCallback> callbackSuccess,
                         std::function<RequestCallback> callbackFailure)
{
    if (!m_apiMap.contains(api)) {
        qWarning() << __FUNCTION__ << ": Cannot send request: Unknown API: " + api;
        return;
    }

    QUrl url(m_synoUrl);

    QStringList path;
    path << url.path()
         << m_apiPath
         << QString::fromLatin1(m_apiMap[api]);
    url.setPath(path.join('/'));

    QNetworkRequest req;
    req.setUrl(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QByteArray body;
    body.reserve(4096);
    body += "api=" + api + '&' + formData.join('&');

    QNetworkReply* reply = m_networkManager.post(req, body);
    m_pendingReplies.insert(reply);
    connect(reply, &QNetworkReply::finished, this, [this, reply, callbackSuccess, callbackFailure]() {
        QString errorString;
        if (!reply->size()) {
            if (QNetworkReply::NoError != reply->error()) {
                errorString = reply->errorString();
            } else {
                errorString = "Unknown error";
            }
        }

        QJsonDocument json;
        QByteArray replyBody = reply->readAll();

        if (errorString.isEmpty()) {
            QJsonParseError jsonParseError;
            json = QJsonDocument::fromJson(replyBody, &jsonParseError);
            if (QJsonParseError::NoError != jsonParseError.error) {
                errorString = jsonParseError.errorString();
            }
        }

        if (errorString.isEmpty()) {
            QJsonValue jsonError = json.object()["error"];
            if (!jsonError.isUndefined() && !jsonError.isNull()) {
                int errorCode = jsonError.toObject()["code"].toInt();
                errorString = QStringLiteral("SYNO error: ") + QString::number(errorCode);
            }
        }

        qDebug() << "Body: " << replyBody;

        if (!errorString.isEmpty()) {
            qWarning() << "HTTP request " << reply->request().url()
                       << " failed: " << errorString;
        }

        if (errorString.isEmpty()) {
            if (callbackSuccess) {
                callbackSuccess(reply, json);
            }
        } else {
            if (callbackFailure) {
                callbackFailure(reply, json);
            }
        }

        reply->close();
        m_pendingReplies.remove(reply);
        reply->deleteLater();
    });
}

void SynoConn::populateApiMap()
{
    m_apiMap.clear();
    m_apiMap["SYNO.API.Info"] = "query.php";

    QByteArrayList formData;
    formData << "query=all";
    formData << "method=query";
    formData << "version=1";
    formData << "ps_username=";

    sendRequest("SYNO.API.Info", formData, [=](QNetworkReply* /*reply*/, const QJsonDocument& json) {
        QJsonValue jsonData = json.object()["data"];
        QJsonObject jsonDataObj = jsonData.toObject();
        if (jsonData.isUndefined() || !jsonDataObj.size()) {
            qWarning() << "Received empty API map!";
            setStatus(SynoConn::DISCONNECTED);
            setIsConnecting(false);
        } else {
            for (auto iter = jsonDataObj.constBegin(); iter != jsonDataObj.constEnd(); ++iter) {
                QString path = iter.value().toObject()["path"].toString();
                if (path.isEmpty()) {
                    qWarning() << "Received empty path for API: " << iter.key();
                } else {
                    m_apiMap[iter.key().toUtf8()] = path.toUtf8();
                }
            }

            setStatus(SynoConn::API_LOADED);
        }
    }, [=](QNetworkReply* /*reply*/, const QJsonDocument& /*json*/){
        setStatus(SynoConn::DISCONNECTED);
        setIsConnecting(false);
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
