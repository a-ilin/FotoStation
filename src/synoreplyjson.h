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

    static QObject* fromQmlEngine(QQmlEngine* engine, QJSEngine* scriptEngine);
    Q_INVOKABLE QObject* create(SynoRequest* synoRequest) const;
};

#endif // SYNOREPLYJSON_H
