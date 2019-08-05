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

#ifndef SYNOCONN_H
#define SYNOCONN_H

#include <functional>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QObject>
#include <QUrl>
#include <QUrlQuery>


class SynoConn : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SynoConn)

    Q_PROPERTY(SynoConnStatus status READ status NOTIFY statusChanged)
    Q_PROPERTY(bool isConnecting READ isConnecting NOTIFY isConnectingChanged)

public:
    typedef void RequestCallback(QNetworkReply* reply, const QJsonDocument& json);

    enum SynoConnStatus
    {
        DISCONNECTED = 0,
        API_LOADED = 1,
        AUTHORIZED = 2
    };
    Q_ENUM(SynoConnStatus)

public:
    explicit SynoConn(QObject *parent = nullptr);

    void sendRequest(const QByteArray& api,
                     const QByteArrayList& formData,
                     std::function<RequestCallback> callbackSuccess = std::function<RequestCallback>(),
                     std::function<RequestCallback> callbackFailure = std::function<RequestCallback>());

    SynoConnStatus status() const;
    bool isConnecting() const;

    Q_INVOKABLE void connectToSyno(const QUrl& synoUrl);
    Q_INVOKABLE void disconnectFromSyno();

    Q_INVOKABLE void authorize(const QString& username, const QString& password);

    Q_INVOKABLE void checkAuth();

signals:
    void statusChanged();
    void isConnectingChanged();

public slots:

private:
    void populateApiMap();
    void setStatus(SynoConnStatus status);
    void setIsConnecting(bool status);

private:
    QNetworkAccessManager m_networkManager;
    QSet<QNetworkReply*> m_pendingReplies;
    /*! Url containing protocol, host name, port, and path to PS */
    QUrl m_synoUrl;
    /*! Map of API query to API path */
    QMap<QByteArray, QByteArray> m_apiMap;
    /*! Path to API directory */
    QString m_apiPath;
    /*! Connection status */
    SynoConnStatus m_status;
    bool m_isConnecting;
    /*! Session token */
    QByteArray m_synoToken;
};

#endif // SYNOCONN_H
