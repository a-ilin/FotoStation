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
#include "synoerror.h"


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

    sendRequest(QByteArrayLiteral("SYNO.PhotoStation.Auth"), formData, [=](const QJsonObject& json) {
        qWarning() << QStringLiteral("Authorization successful!");
        m_synoToken = json[QStringLiteral("sid")].toString().toUtf8();
        setStatus(SynoConn::AUTHORIZED);
        setIsConnecting(false);
    }, [=]() {
        qWarning() << QStringLiteral("Authorization failed!");
        setIsConnecting(false);
    });
}

void SynoConn::checkAuth()
{
    QByteArrayList formData;
    formData << QByteArrayLiteral("method=checkauth");
    formData << QByteArrayLiteral("version=1");

    sendRequest(QByteArrayLiteral("SYNO.PhotoStation.Auth"), formData, [=](const QJsonObject& /*json*/) {
        setIsConnecting(false);
    }, [=]() {
        qWarning() << QStringLiteral("Check auth failed!");
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

QStringList SynoConn::apiList() const
{
    QStringList apiList;
    apiList.reserve(m_apiMap.size());
    std::for_each(m_apiMap.keyBegin(), m_apiMap.keyEnd(), [&](const QByteArray& api) {
        apiList << QString::fromLatin1(api);
    });
    return apiList;
}

void SynoConn::sendRequest(const QByteArray& api,
                           const QByteArrayList& formData,
                           std::function<RequestCallbackSuccess> callbackSuccess,
                           std::function<RequestCallbackFailure> callbackFailure)
{
    if (!m_apiMap.contains(api)) {
        qWarning() << __FUNCTION__
                   << QStringLiteral(": Cannot send request: Unknown API:")
                   << api;
        callbackFailure();
        return;
    }

    QNetworkRequest req;

    QUrl url(m_synoUrl);
    QUrlQuery urlQuery;

    QStringList path;
    path << url.path()
         << m_apiPath
         << QString::fromLatin1(m_apiMap[api]);
    url.setPath(path.join('/'));

    if (!m_synoToken.isEmpty()) {
        req.setRawHeader(QByteArrayLiteral("X-SYNO-TOKEN"), m_synoToken);
        urlQuery.addQueryItem(QByteArrayLiteral("SynoToken"), m_synoToken);
    }

    url.setQuery(urlQuery);
    req.setUrl(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/x-www-form-urlencoded"));

    QByteArray body;
    body.reserve(4096);
    body += QByteArrayLiteral("api=") + api;

    if (!m_synoToken.isEmpty()) {
        body += QByteArrayLiteral("&SynoToken=") + m_synoToken;
    }

    if (!formData.isEmpty()) {
        body += '&' + formData.join('&');
    }

    QNetworkReply* reply = m_networkManager.post(req, body);
    m_pendingReplies.insert(reply);
    connect(reply, &QNetworkReply::finished, this, [this, reply, callbackSuccess, callbackFailure]() {
        QString errorString;
        if (!reply->size()) {
            if (QNetworkReply::NoError != reply->error()) {
                errorString = reply->errorString();
            } else {
                errorString = QStringLiteral("Unknown network error");
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
            QJsonValue jsonError = json.object()[QStringLiteral("error")];
            if (!jsonError.isUndefined() && !jsonError.isNull()) {
                QJsonObject jsonErrorObject = jsonError.toObject();
                auto errorCodeIter = jsonErrorObject.find(QStringLiteral("code"));
                if (errorCodeIter != jsonErrorObject.end()) {
                    int errorCode = errorCodeIter.value().toInt(-1);

                    if (const char* errorCodeEnum = SynoErrorGadget::metaEnum().valueToKey(errorCode)) {
                        errorString = QStringLiteral("SYNO error: %1 (%2)").arg(errorCodeEnum).arg(errorCode);
                    } else {
                        errorString = QStringLiteral("SYNO error: UNKNOWN (%1)").arg(errorCode);
                    }
                } else {
                    errorString = QStringLiteral("SYNO error: UNKNOWN");
                }
            }
        }

        QJsonValue jsonSuccess = json.object()[QStringLiteral("success")];
        if (!jsonSuccess.toBool() && errorString.isEmpty()) {
            errorString = QStringLiteral("SYNO success: FALSE");
        }

        //qDebug() << "Headers: " << reply->rawHeaderPairs();
        qDebug() << "Body: " << replyBody;

        if (!errorString.isEmpty()) {
            qWarning() << QStringLiteral("HTTP request ") << reply->request().url()
                       << QStringLiteral(" failed: ") << errorString;
        }

        if (errorString.isEmpty()) {
            if (callbackSuccess) {
                callbackSuccess(json.object()[QStringLiteral("data")].toObject());
            }
        } else {
            if (callbackFailure) {
                callbackFailure();
            }
        }

        reply->close();
        m_pendingReplies.remove(reply);
        reply->deleteLater();
    });
}

void SynoConn::sendRequest(const QString& api, const QStringList& formData, QJSValue callbackSuccess, QJSValue callbackFailure)
{
    QByteArrayList baFormData;
    baFormData.reserve(formData.size());
    std::for_each(formData.cbegin(), formData.cend(), [&](const QString& formEntry) {
        baFormData << formEntry.toUtf8();
    });

    auto errorLogger = [](const QString& contextMsg, const QJSValue& result) {
        if (result.isError()) {
            qWarning() << QStringLiteral("Error during JS callback execution: ")
                       << contextMsg
                       << QStringLiteral("Script Error: ")
                       << result.toString();
        }
    };

    sendRequest(api.toLatin1(), baFormData,
                [=](const QJsonObject& jsonDataObject) mutable {
                    QJsonDocument jsonDoc(jsonDataObject);
                    QString jsonSerialized = QString::fromUtf8(jsonDoc.toJson(QJsonDocument::Indented));
                    if (callbackSuccess.isCallable()) {
                        QJSValue result = callbackSuccess.call(QJSValueList() << QJSValue(jsonSerialized));
                        errorLogger(QStringLiteral("Success callback"), result);
                    }
                },
                [=]() mutable {
                    if (callbackFailure.isCallable()) {
                        QJSValue result = callbackFailure.call();
                        errorLogger(QStringLiteral("Error callback"), result);
                    }
                });
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

    sendRequest(QByteArrayLiteral("SYNO.API.Info"), formData, [=](const QJsonObject& jsonDataObj) {
        if (!jsonDataObj.size()) {
            qWarning() << QStringLiteral("Received empty API map!");
            setStatus(SynoConn::DISCONNECTED);
            setIsConnecting(false);
        } else {
            for (auto iter = jsonDataObj.constBegin(); iter != jsonDataObj.constEnd(); ++iter) {
                QString path = iter.value().toObject()["path"].toString();
                if (path.isEmpty()) {
                    qWarning() << QStringLiteral("Received empty path for API: ") << iter.key();
                } else {
                    m_apiMap[iter.key().toUtf8()] = path.toUtf8();
                }
            }

            emit apiListChanged();
            setStatus(SynoConn::API_LOADED);
        }
    }, [=](){
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
