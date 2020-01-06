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

#include "synoconn.h"
#include "synorequest.h"

#include <QDebug>
#include <QMetaObject>
#include <QMimeDatabase>
#include <QThread>

#include <functional>

SynoRequest::SynoRequest(const QByteArray& api, const QByteArrayList& formData, SynoConn* conn)
    : QObject()
    , m_conn(conn)
    , m_api(api)
    , m_formData(formData)
    , m_reply(nullptr)
    , m_contentType(UNKNOWN)
{
    Q_ASSERT(conn);

    // ensure the slot execution is synchronized to connection object
    if (QThread::currentThread() != m_conn->thread()) {
        moveToThread(m_conn->thread());
    }
}

SynoRequest::~SynoRequest()
{
    if (m_reply) {
        m_reply->deleteLater();
    }
}

const QByteArray& SynoRequest::api() const
{
    return m_api;
}

const QByteArrayList& SynoRequest::formData() const
{
    return m_formData;
}

QNetworkRequest& SynoRequest::request()
{
    return m_request;
}

const QNetworkRequest& SynoRequest::request() const
{
    return m_request;
}

QNetworkReply* SynoRequest::reply() const
{
    return m_reply;
}

void SynoRequest::setReply(QNetworkReply* reply)
{
    if (m_reply) {
        QObject::disconnect(reply, &QNetworkReply::finished, this, &SynoRequest::onReplyFinished);
        m_reply->deleteLater();
    }

    m_reply = reply;

    if (m_reply) {
        QObject::connect(reply, &QNetworkReply::finished, this, &SynoRequest::onReplyFinished);
    }
}

const QByteArray& SynoRequest::replyBody() const
{
    return m_replyBody;
}

QString SynoRequest::errorString() const
{
    return m_errorString;
}

void SynoRequest::setErrorString(const QString& err)
{
    m_errorString = err;
    emit errorStringChanged();
}

const QByteArray& SynoRequest::contentMimeTypeRaw() const
{
    return m_contentMimeTypeRaw;
}

QMimeType SynoRequest::contentMimeType() const
{
    return m_contentMimeType;
}

SynoRequest::ContentType SynoRequest::contentType() const
{
    return m_contentType;
}

const QByteArray& SynoRequest::contentEncoding() const
{
    return m_contentEncoding;
}

void SynoRequest::send()
{
    if (m_conn) {
        if (QThread::currentThread() == m_conn->thread()) {

#ifdef QT_DEBUG
            qDebug() << QStringLiteral("RQ:API: ") << m_api;
            qDebug() << QStringLiteral("RQ:FormData: ") << m_formData;
#endif

            m_conn->sendRequest(this);
        } else {
            QMetaObject::invokeMethod(this, std::bind(qOverload<void>(&SynoRequest::send), this),
                                      Qt::QueuedConnection, nullptr);
        }
    } else {
        setErrorString(tr("Cannot send request. Connection object was destroyed or not set."));
    }
}

void SynoRequest::send(QObject* context, std::function<void ()> callback)
{
    Q_ASSERT(context);
    Q_ASSERT(callback);
    QObject::disconnect(m_callbackConnection);
    m_callbackConnection = QObject::connect(this, &SynoRequest::finished, context, callback);
    send();
}

void SynoRequest::send(QJSValue callback)
{
    QObject::disconnect(m_callbackConnection);
    m_callbackConnection = QObject::connect(this, &SynoRequest::finished, [callback]() mutable {
        if (callback.isCallable()) {
            QJSValue result = callback.call();
            if (result.isError()) {
                qWarning() << __FUNCTION__ << tr("Error during JS callback execution: %1")
                              .arg(result.toString());
            }
        } else {
            qWarning() << __FUNCTION__ << tr("JS Callback is not callable");
        }
    });
    send();
}

void SynoRequest::cancel()
{
    if (m_conn) {
        if (QThread::currentThread() == m_conn->thread()) {
            QObject::disconnect(m_callbackConnection);
            QObject::disconnect(m_reply, &QNetworkReply::finished, this, &SynoRequest::onReplyFinished);
            m_conn->cancelRequest(this);
        } else {
            QMetaObject::invokeMethod(this, std::bind(&SynoRequest::cancel, this), Qt::QueuedConnection);
        }
    } else {
        setErrorString(tr("Cannot cancel request. Connection object was destroyed or not set."));
    }
}

void SynoRequest::parseContentType()
{
    m_contentMimeTypeRaw = m_reply->header(QNetworkRequest::ContentTypeHeader).toByteArray();
    QList<QByteArray> contentTypeList(m_contentMimeTypeRaw.split(';'));
    QByteArray contentType;
    if (contentTypeList.size() > 0) {
        contentType = contentTypeList[0];
    }

    if (contentTypeList.size() > 1) {
        QByteArray contentEncoding = contentTypeList[1].trimmed();
        QByteArray charsetPrefix(QByteArrayLiteral("charset="));
        if (contentEncoding.startsWith(charsetPrefix)) {
            m_contentEncoding = contentEncoding.mid(charsetPrefix.size());
        }
    }

    QMimeDatabase mimeDb;
    m_contentMimeType = mimeDb.mimeTypeForName(contentType);
    if (m_contentMimeType.inherits(QStringLiteral("text/plain"))) {
        m_contentType = TEXT;
    } else if (m_contentMimeType.inherits(QStringLiteral("image/jpeg"))) {
        m_contentType = IMAGE_JPEG;
    } else if (m_contentMimeType.inherits(QStringLiteral("image"))) {
        m_contentType = IMAGE_OTHER;
    }

    emit contentTypeChanged();
}

void SynoRequest::onReplyFinished()
{
    if (m_reply) {
        if (m_reply->size()) {
            m_replyBody = m_reply->readAll();
            m_reply->close();
            parseContentType();
        } else {
            if (QNetworkReply::NoError != m_reply->error()) {
                setErrorString(tr("Network error: %1").arg(m_reply->errorString()));
            } else {
                setErrorString(tr("Unknown network error"));
            }
        }

#ifdef QT_DEBUG
        qDebug() << QStringLiteral("RP:Headers: ") << m_reply->rawHeaderPairs();
        qDebug() << QStringLiteral("RP:Body: ") << m_replyBody;
#endif

    } else {
        setErrorString(tr("Network reply was destroyed"));
    }

    emit finished();
}
