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
#include "synorequest.h"

#include <QDebug>
#include <QMimeDatabase>

SynoRequest::SynoRequest(const QByteArray& api, const QByteArrayList& formData, SynoConn* conn)
    : QObject()
    , m_conn(conn)
    , m_api(api)
    , m_formData(formData)
    , m_reply(nullptr)
    , m_contentType(UNKNOWN)
{
    Q_ASSERT(conn);
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
    m_reply = reply;
    connect(reply, &QNetworkReply::finished, this, &SynoRequest::onReplyFinished);
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

SynoRequest::ContentType SynoRequest::contentType() const
{
    return m_contentType;
}

QString SynoRequest::contentEncoding() const
{
    return m_contentEncoding;
}

void SynoRequest::send()
{
    if (m_conn) {
        m_conn->sendRequest(this);
    } else {
        setErrorString(tr("Cannot send request. Connection object was destroyed or not set."));
    }
}

void SynoRequest::send(QObject* context, std::function<void ()> callback)
{
    Q_ASSERT(context);
    Q_ASSERT(callback);
    connect(this, &SynoRequest::finished, context, callback);
    send();
}

void SynoRequest::send(QJSValue callback)
{
    connect(this, &SynoRequest::finished, [callback]() mutable {
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
        m_conn->cancelRequest(this);
    } else {
        setErrorString(tr("Cannot cancel request. Connection object was destroyed or not set."));
    }
}

void SynoRequest::parseContentType()
{
    QStringList contentTypeList(m_reply->header(QNetworkRequest::ContentTypeHeader).toString().split(';'));
    QString contentType;
    if (contentTypeList.size() > 0) {
        contentType = contentTypeList[0];
    }

    if (contentTypeList.size() > 1) {
        QString contentEncoding = contentTypeList[1].trimmed();
        QString charsetPrefix(QStringLiteral("charset="));
        if (contentEncoding.startsWith(charsetPrefix)) {
            m_contentEncoding = contentEncoding.mid(charsetPrefix.size());
            emit contentEncodingChanged();
        }
    }

    QMimeDatabase mimeDb;
    QMimeType mimeType = mimeDb.mimeTypeForName(contentType);
    if (mimeType.inherits(QStringLiteral("text/plain"))) {
        m_contentType = TEXT;
    } else if (mimeType.inherits(QStringLiteral("image/jpeg"))) {
        m_contentType = IMAGE_JPEG;
    } else if (mimeType.inherits(QStringLiteral("image"))) {
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

        qDebug() << "Headers: " << m_reply->rawHeaderPairs();
        qDebug() << "Body: " << m_replyBody;
    } else {
        setErrorString(tr("Network reply was destroyed"));
    }

    emit finished();
}
