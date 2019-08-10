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

#ifndef SYNOREPLYJSON_H
#define SYNOREPLYJSON_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>

class QJSEngine;
class QQmlEngine;
class SynoRequest;

class SynoReplyJSON : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString errorString READ errorString WRITE setErrorString NOTIFY errorStringChanged)
    Q_PROPERTY(QString text READ text)

public:
    SynoReplyJSON(const QByteArray& jsonText, QObject* parent = nullptr);
    SynoReplyJSON(const SynoRequest* synoRequest, QObject* parent = nullptr);

    QString errorString() const;
    void setErrorString(const QString& err);

    const QJsonDocument& document() const;
    const QJsonObject& dataObject() const;

    QString text() const;

signals:
    void errorStringChanged();

private:
    void parseStatus();

private:
    QString m_errorString;
    QJsonDocument m_json;
    QJsonObject m_dataObject;
};

class SynoReplyJSONFactory : public QObject
{
    Q_OBJECT

public:
    SynoReplyJSONFactory(QObject* parent = nullptr);

    static QObject* provider(QQmlEngine* engine, QJSEngine* scriptEngine);
    Q_INVOKABLE QObject* create(SynoRequest* synoRequest) const;
};

#endif // SYNOREPLYJSON_H
