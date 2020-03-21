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

class SynoAuth;
class SynoRequest;
class SynoSslConfig;

class SynoConn : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SynoConn)

    Q_PROPERTY(QUrl synoUrl READ synoUrl NOTIFY synoUrlChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)
    Q_PROPERTY(SynoConnStatus status READ status NOTIFY statusChanged)
    Q_PROPERTY(QStringList apiList READ apiList NOTIFY apiListChanged)
    Q_PROPERTY(SynoSslConfig* sslConfig READ sslConfig NOTIFY sslConfigChanged)
    Q_PROPERTY(SynoAuth* auth READ auth NOTIFY authChanged)

public:

    /*! This enum identifies the status of connection to the service */
    enum SynoConnStatus
    {
        /*! No connection was made, or was unsuccessful */
        NONE = 0,
        /*! Attempting API loading */
        ATTEMPT_API,
        /*! API is loaded */
        API_LOADED
    };
    Q_ENUM(SynoConnStatus)

public:
    explicit SynoConn(QObject *parent = nullptr);
    ~SynoConn();

    /*!
     * \brief Returns list of loaded API
     *
     * This method exists for debug purposes only, due to performance impact.
     */
    QStringList apiList() const;

    SynoConnStatus status() const;
    const QUrl& synoUrl() const;
    const QString& errorString() const;
    SynoSslConfig* sslConfig() const;
    SynoAuth* auth() const;

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
    void synoUrlChanged();
    void errorStringChanged();
    void statusChanged();
    void apiListChanged();
    void sslConfigChanged();
    void authChanged();

public slots:
    void connectToSyno(const QUrl& synoUrl);
    void disconnectFromSyno();

    void sendRequest(SynoRequest* request);
    void cancelRequest(SynoRequest* request);
    void cancelAllRequests();

private:
    void setErrorString(const QString& err);
    QString apiPath(const QByteArray& api) const;
    void sendApiMapRequest();
    bool processApiMapReply(const SynoRequest* req);
    void setStatus(SynoConnStatus status);
    void onReplyFinished(SynoRequest* request);

private:
    QString m_errorString;
    QNetworkAccessManager m_networkManager;
    /*! Set of active requests */
    QSet< SynoRequest* > m_pendingRequests;
    /*! Url containing protocol, host name, port, and path to PS */
    QUrl m_synoUrl;
    /*! Map of API query to API path */
    QMap<QByteArray, QByteArray> m_apiMap;
    /*! Path to API directory */
    QString m_apiPath;
    /*! Connection status */
    SynoConnStatus m_status;
    /*! Authorization handler */
    SynoAuth* m_auth;
    /*! SSL config */
    SynoSslConfig* m_sslConfig;
};

#endif // SYNOCONN_H
