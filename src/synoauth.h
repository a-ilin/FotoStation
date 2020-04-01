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

#ifndef SYNOAUTH_H
#define SYNOAUTH_H

#include <QByteArray>
#include <QJSValue>
#include <QObject>
#include <QQmlEngine>

class QNetworkAccessManager;

class SynoConn;
class SynoCookieJar;
class SynoRequest;

class SynoAuth : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SynoAuth)

    QML_ELEMENT
    QML_UNCREATABLE("Use SynoConn to obtain an instance")

    Q_PROPERTY(SynoAuthStatus status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString username READ username NOTIFY usernameChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)
    Q_PROPERTY(bool isCookieAvailable READ isCookieAvailable NOTIFY isCookieAvailableChanged)
    Q_PROPERTY(bool keepCookies READ keepCookies WRITE setKeepCookies NOTIFY keepCookiesChanged)

public:
    /*! This enum identifies the status of connection to the service */
    enum SynoAuthStatus
    {
        /*! Not authorized */
        NONE = 0,
        /*! Attempting authorization with cookie */
        ATTEMPT_COOKIE,
        /*! Cookie auth failed, waiting for user credentials */
        WAIT_USER,
        /*! Attempting authorization with user credentials */
        ATTEMPT_USER,
        /*! Authorized */
        AUTHORIZED
    };
    Q_ENUM(SynoAuthStatus)

public:
    explicit SynoAuth(QNetworkAccessManager* nma, SynoConn* parent);

    SynoAuthStatus status() const;
    const QString& username() const;
    const QByteArray& synoToken() const;
    const QString& errorString() const;
    bool isCookieAvailable() const;

    bool keepCookies() const;
    void setKeepCookies(bool value);

    /*!
     *  \brief This method reloads cookies from permanent storage
     *
     *  \param callback JS callback to be executed on method completion (success or failure). Expected signature: void()
     *
     *  Session authorization is lost after call to this method.
     *
     *  This method is intended to be used from QML.
     */
    Q_INVOKABLE void reloadCookies(QJSValue callback = QJSValue());

    /*!
     *  \brief This method returns availability of cookies for specified url
     *
     *  This method is intended to be used from QML.
     */
    Q_INVOKABLE bool isCookieAvailableForUrl(const QUrl& url) const;

public slots:
    void authorizeWithCredentials(const QString& username, const QString& password);
    void authorizeWithCookie();
    void closeSession(bool clearCookies);

signals:
    void statusChanged();
    void usernameChanged();
    void errorStringChanged();
    void isCookieAvailableChanged();
    void keepCookiesChanged();
    void authorizationFailed();

private:
    void setStatus(SynoAuthStatus status);
    void SetUsername(const QString& value);
    void setErrorString(const QString& err);
    void checkAuthorizationWithCookie();
    bool processAuthorizeReply(const SynoRequest* req);
    SynoCookieJar* cookieJar() const;

private:
    /*! Current status */
    SynoAuthStatus m_status;
    /*! Last error */
    QString m_errorString;
    /*! Connection object */
    SynoConn* m_conn;
    /*! Network access manager object*/
    QNetworkAccessManager* m_nma;
    /*! Session token */
    QByteArray m_synoToken;
    /*! Name of authorized user */
    QString m_username;
    /*! Keep cookies for future authorization */
    bool m_keepCookies;
};

#endif // SYNOAUTH_H
