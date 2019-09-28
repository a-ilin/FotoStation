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

#include <memory>

#include <QNetworkAccessManager>
#include <QObject>
#include <QPointer>
#include <QSet>
#include <QUrl>

class SynoRequest;

class SynoConn : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SynoConn)

    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)
    Q_PROPERTY(SynoConnStatus status READ status NOTIFY statusChanged)
    Q_PROPERTY(bool isConnecting READ isConnecting NOTIFY isConnectingChanged)
    Q_PROPERTY(QStringList apiList READ apiList NOTIFY apiListChanged)

public:
    enum SynoConnStatus
    {
        DISCONNECTED = 0,
        API_LOADED = 1,
        AUTHORIZED = 2
    };
    Q_ENUM(SynoConnStatus)

public:
    explicit SynoConn(QObject *parent = nullptr);

    SynoConnStatus status() const;
    bool isConnecting() const;

    QStringList apiList() const;

    QString errorString() const;
    void setErrorString(const QString& err);

    /*!
     *  \brief Creates request with specified parameters
     *
     *  The caller is responsible for request release.
     *  This method is intended to be used from QML.
     */
    Q_INVOKABLE SynoRequest* createRequest(const QString& api,
                                           const QStringList& formData);

    /*!
     *  \brief Creates request with specified parameters
     *
     *  This method is intended to be used from C++.
     */
    Q_INVOKABLE std::shared_ptr<SynoRequest> createRequest(const QByteArray& api,
                                                           const QByteArrayList& formData);

signals:
    void errorStringChanged();
    void statusChanged();
    void isConnectingChanged();
    void apiListChanged();

public slots:
    void connectToSyno(const QUrl& synoUrl);
    void disconnectFromSyno();
    void authorize(const QString& username, const QString& password);
    void checkAuth();
    void sendRequest(SynoRequest* request);
    void cancelRequest(SynoRequest* request);
    void cancelAllRequests();

private:
    QString apiPath(const QByteArray& api) const;
    void populateApiMap();
    void setStatus(SynoConnStatus status);
    void setIsConnecting(bool status);

private:
    QString m_errorString;
    QNetworkAccessManager m_networkManager;
    QSet< QPointer<SynoRequest> > m_pendingRequests;
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

uint qHash(const QPointer<SynoRequest>& req);

#endif // SYNOCONN_H
