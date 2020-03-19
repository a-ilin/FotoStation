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
    Q_PROPERTY(bool isIntrusive READ isIntrusive WRITE setIsIntrusive NOTIFY isIntrusiveChanged)

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

    bool isIntrusive() const;
    void setIsIntrusive(bool value);

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
    void isIntrusiveChanged();
    void finished();

private:
    void parseContentType();

private slots:
    void onReplyFinished();

private:
    QPointer<SynoConn> m_conn;
    QMetaObject::Connection m_callbackConnection;
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
    bool m_intrusive;
};

#endif // SYNOREPLY_H
