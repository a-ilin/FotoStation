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

#ifndef SYNOREPLY_H
#define SYNOREPLY_H

#include <functional>

#include <QJSValue>
#include <QMimeType>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QPointer>

class SynoConn;

class SynoRequest : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QByteArray contentEncoding READ contentEncoding NOTIFY contentTypeChanged)
    Q_PROPERTY(QMimeType contentMimeType READ contentMimeType NOTIFY contentTypeChanged)
    Q_PROPERTY(ContentType contentType READ contentType NOTIFY contentTypeChanged)
    Q_PROPERTY(QString errorString READ errorString WRITE setErrorString NOTIFY errorStringChanged)

public:
    enum ContentType
    {
        UNKNOWN = 0,
        TEXT = 1,
        IMAGE_JPEG = 2,
        IMAGE_OTHER
    };
    Q_ENUM(ContentType)

public:
    SynoRequest(const QByteArray& api, const QByteArrayList& formData, SynoConn* conn);
    ~SynoRequest();

    const QByteArray& api() const;
    const QByteArrayList& formData() const;

    QNetworkRequest& request();
    const QNetworkRequest& request() const;

    QNetworkReply* reply() const;
    void setReply(QNetworkReply* reply);

    const QByteArray& replyBody() const;

    QString errorString() const;
    void setErrorString(const QString& err);

    const QByteArray& contentMimeTypeRaw() const;
    QMimeType contentMimeType() const;
    ContentType contentType() const;
    const QByteArray& contentEncoding() const;

public slots:
    void send();
    void send(QObject* context, std::function<void()> callback = std::function<void()>());
    void send(QJSValue callback);
    void cancel();

signals:
    void contentTypeChanged();
    void errorStringChanged();
    void finished();

private:
    void parseContentType();

private slots:
    void onReplyFinished();

private:
    QPointer<SynoConn> m_conn;
    QByteArray m_api;
    QByteArrayList m_formData;
    QNetworkRequest m_request;
    QPointer<QNetworkReply> m_reply;
    QString m_errorString;
    QByteArray m_contentMimeTypeRaw;
    QMimeType m_contentMimeType;
    ContentType m_contentType;
    QByteArray m_contentEncoding;
    QByteArray m_replyBody;
};

#endif // SYNOREPLY_H
