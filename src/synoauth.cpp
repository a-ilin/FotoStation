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
#include "synoreplyjson.h"
#include "synorequest.h"
#include "synosettings.h"
#include "synotraits.h"

#include <QDataStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkAccessManager>
#include <QNetworkCookie>
#include <QNetworkCookieJar>

class SynoCookieJar : public QNetworkCookieJar
{
public:
    SynoCookieJar(QObject* parent)
        : QNetworkCookieJar(parent)
        , m_settings(QStringLiteral("connection"))
    {
        loadFromStorage();
    }

    bool isEmpty() const
    {
        return allCookies().isEmpty();
    }

    void clear()
    {
        setAllCookies(QList<QNetworkCookie>());
    }

    void loadFromStorage(QObject* context = nullptr, std::function<void()> callback = std::function<void()>())
    {
        QPointer<QObject> contextPtr(context);
        m_settings.readSecureC(QStringLiteral("cookies"), this,
                             [this, contextPtr, callback](const QString&, const QString& secureValue) {
            QByteArray ba = QByteArray::fromHex(secureValue.toUtf8());
            QDataStream ds(&ba, QIODevice::ReadOnly);

            QList< QByteArray > cookiesRawList;
            ds >> cookiesRawList;

            for (const QByteArray& cookieRaw : std::as_const(cookiesRawList)) {
                QList<QNetworkCookie> cookieList = QNetworkCookie::parseCookies(cookieRaw);
                for (const QNetworkCookie& cookie : std::as_const(cookieList)) {
                    insertCookie(cookie);
                }
            }

            if (contextPtr && callback) {
                callback();
            }
        }, [contextPtr, callback](const QString&, const QString&) {
            if (contextPtr && callback) {
                callback();
            }
        });
    }

    void saveToStorage()
    {
        QList<QNetworkCookie> cookies = allCookies();

        if (cookies.isEmpty()) {
            deleteFromStorage();
        } else {
            QList<QByteArray> cookiesRawList;
            cookiesRawList.reserve(cookies.size());
            for (const QNetworkCookie& cookie : std::as_const(cookies)) {
                cookiesRawList << cookie.toRawForm();
            }

            QByteArray ba;
            ba.reserve(8192);

            {
                QDataStream ds(&ba, QIODevice::WriteOnly);
                ds << cookiesRawList;
            }

            QString secureValue = QString::fromUtf8(ba.toHex());

            m_settings.saveSecureC(QStringLiteral("cookies"), secureValue);
        }
    }

    void deleteFromStorage()
    {
        m_settings.deleteSecureC(QStringLiteral("cookies"));
    }

private:
    SynoSettings m_settings;
};

SynoAuth::SynoAuth(QNetworkAccessManager* nma, SynoConn* parent)
    : QObject(parent)
    , m_status(SynoAuth::NONE)
    , m_conn(parent)
    , m_nma(nma)
    , m_keepCookies(false)
{
    Q_ASSERT(nma);
    Q_ASSERT(parent);

    nma->setCookieJar(new SynoCookieJar(this));
}

SynoAuth::SynoAuthStatus SynoAuth::status() const
{
    return m_status;
}

const QString& SynoAuth::username() const
{
    return m_username;
}

const QByteArray& SynoAuth::synoToken() const
{
    return m_synoToken;
}

const QString& SynoAuth::errorString() const
{
    return m_errorString;
}

bool SynoAuth::isCookieAvailable() const
{
    return isCookieAvailableForUrl(m_conn->synoUrl());
}

bool SynoAuth::isCookieAvailableForUrl(const QUrl& url) const
{
    SynoCookieJar* jar = cookieJar();
    QList<QNetworkCookie> cookies = jar->cookiesForUrl(url);
    return !cookies.isEmpty();
}

bool SynoAuth::keepCookies() const
{
    return m_keepCookies;
}

void SynoAuth::setKeepCookies(bool value)
{
    if (value != m_keepCookies) {
        m_keepCookies = value;
        emit keepCookiesChanged();
    }
}

void SynoAuth::reloadCookies(QJSValue callback)
{
    cookieJar()->loadFromStorage(this, [callback]() {
        executeJSCallback(callback);
    });
}

void SynoAuth::closeSession(bool clearCookies)
{
    setStatus(SynoAuth::NONE);

    m_synoToken.clear();
    SetUsername(QString());

    if (clearCookies) {
        SynoCookieJar* jar = cookieJar();
        jar->clear();
        jar->saveToStorage();
        emit isCookieAvailableChanged();
    }
}

void SynoAuth::setStatus(SynoAuth::SynoAuthStatus status)
{
    if (status != m_status) {
        m_status = status;
        qDebug() << __FUNCTION__ << "Status:" << m_status;
        emit statusChanged();
    }
}

void SynoAuth::SetUsername(const QString& value)
{
    if (value != m_username) {
        m_username = value;
        emit usernameChanged();
    }
}

void SynoAuth::setErrorString(const QString& err)
{
    if (err != m_errorString) {
        m_errorString = err;
        emit errorStringChanged();
    }
}

void SynoAuth::checkAuthorizationWithCookie()
{
    setStatus(SynoAuth::ATTEMPT_COOKIE);

    QByteArrayList formData;
    formData << QByteArrayLiteral("method=checkauth");
    formData << QByteArrayLiteral("version=1");

    std::shared_ptr<SynoRequest> req = m_conn->createRequest(QByteArrayLiteral("SYNO.PhotoStation.Auth"), formData);
    req->send(this, [this, req]() {
        if (processAuthorizeReply(req.get())) {
            qInfo() << tr("Cookie authorization successful");
        } else {
            qInfo() << tr("Cookie authorization failed, credentials should be submitted");
            setStatus(SynoAuth::WAIT_USER);
        }
    });
}

SynoCookieJar* SynoAuth::cookieJar() const
{
    Q_ASSERT(dynamic_cast<SynoCookieJar*>(m_nma->cookieJar()));
    SynoCookieJar* jar = static_cast<SynoCookieJar*>(m_nma->cookieJar());
    return jar;
}

void SynoAuth::authorizeWithCredentials(const QString& username, const QString& password)
{
    setStatus(SynoAuth::ATTEMPT_USER);

    QByteArrayList formData;
    formData << QByteArrayLiteral("method=login");
    formData << QByteArrayLiteral("version=1");
    formData << QByteArrayLiteral("enable_syno_token=true");
    formData << QByteArrayLiteral("username=") + username.toUtf8();
    formData << QByteArrayLiteral("password=") + password.toUtf8();
    formData << QByteArrayLiteral("remember_me=true");

    std::shared_ptr<SynoRequest> req = m_conn->createRequest(QByteArrayLiteral("SYNO.PhotoStation.Auth"), formData);
    req->send(this, [this, req]() {
        if (processAuthorizeReply(req.get())) {
            qInfo() << tr("Credentials authorization successful");
        } else {
            qInfo() << tr("Credentials authorization failed");
            // authorization failed
            setStatus(SynoAuth::NONE);
            emit authorizationFailed();
        }
    });
}

void SynoAuth::authorizeWithCookie()
{
    setStatus(SynoAuth::ATTEMPT_COOKIE);

    if (!isCookieAvailable()) {
        cookieJar()->loadFromStorage(this, [this]() {
            if (!isCookieAvailable()) {
                qDebug() << tr("Cookie jar has no cookie for current URL, credentials should be submitted");
                setStatus(SynoAuth::WAIT_USER);
            } else {
                qDebug() << tr("Cookie jar is not empty, attempting authorization");
                checkAuthorizationWithCookie();
            }
        });
    } else {
        qDebug() << tr("Cookie jar is not empty, attempting authorization");
        checkAuthorizationWithCookie();
    }
}

bool SynoAuth::processAuthorizeReply(const SynoRequest* req)
{
    auto failure = [this](const QString& reason) {
        // not authorized, close session and clear cookies
        closeSession(true);
        this->setErrorString(tr("Authorization failed. %1").arg(reason));
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

    m_synoToken = replyJSON.dataObject()[QStringLiteral("sid")].toString().toUtf8();
    SetUsername(replyJSON.dataObject()[QStringLiteral("username")].toString());

    if (m_synoToken.isEmpty()) {
        failure(tr("Session ID is not set."));
        return false;
    }

    if (m_username.isEmpty()) {
        failure(tr("Username is not set."));
        return false;
    }

    SynoCookieJar* jar = cookieJar();
    if (m_keepCookies) {
        jar->saveToStorage();
    } else {
        // preserve cookies for active session, but remove from persistent storage
        jar->deleteFromStorage();
    }

    setStatus(SynoAuth::AUTHORIZED);

    emit isCookieAvailableChanged();

    return true;
}
