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

    /*! This enum identifies the status of connection to the service */
    enum SynoConnStatus
    {
        /*! No connection was made, or was unsuccessful */
        DISCONNECTED = 0,
        /*! Socket is connected, and API is loaded */
        API_LOADED = 1,
        /*! Socket is connected, API is loaded, and user is authorized */
        AUTHORIZED = 2
    };
    Q_ENUM(SynoConnStatus)

public:
    explicit SynoConn(QObject *parent = nullptr);
    ~SynoConn();

    SynoConnStatus status() const;

    /*!
     * \brief This method returns the status of connection process.
     *
     * \returns TRUE when is attempting to connect, FALSE else
     */
    bool isConnecting() const;

    /*!
     * \brief Returns list of loaded API
     *
     * This method exists for debug purposes only, due to performance impact.
     */
    QStringList apiList() const;

    QString errorString() const;
    void setErrorString(const QString& err);

    /*!
     *  \brief Creates request with specified parameters
     *
     *  The caller is responsible for request release.
     *  This method is intended to be used from QML only.
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
    /*! Set of active requests */
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
