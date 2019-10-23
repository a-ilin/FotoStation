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

#include "synoerror.h"
#include "synoreplyjson.h"

#include <QJsonObject>
#include <QJsonParseError>

#include "synorequest.h"

SynoReplyJSON::SynoReplyJSON(const QByteArray& jsonText, QObject* parent)
    : QObject(parent)
{
    QJsonParseError jsonParseError;
    m_json = QJsonDocument::fromJson(jsonText, &jsonParseError);
    if (QJsonParseError::NoError == jsonParseError.error) {
        parseStatus();
    } else {
        setErrorString(tr("Cannot parse JSON: %1").arg(jsonParseError.errorString()));
    }
}

SynoReplyJSON::SynoReplyJSON(const SynoRequest* synoRequest, QObject* parent)
    : SynoReplyJSON(synoRequest->replyBody(), parent)
{
}

QString SynoReplyJSON::errorString() const
{
    return m_errorString;
}

void SynoReplyJSON::setErrorString(const QString& err)
{
    m_errorString = err;
    emit errorStringChanged();
}

const QJsonDocument& SynoReplyJSON::document() const
{
    return m_json;
}

const QJsonObject& SynoReplyJSON::dataObject() const
{
    return m_dataObject;
}

QString SynoReplyJSON::text() const
{
    return QString::fromUtf8(m_json.toJson(QJsonDocument::Indented));
}

void SynoReplyJSON::parseStatus()
{
    QJsonValue jvError = m_json.object()[QStringLiteral("error")];
    if (!jvError.isUndefined() && !jvError.isNull()) {
        QJsonObject joError = jvError.toObject();
        auto joiErrorCode = joError.find(QStringLiteral("code"));
        if (joiErrorCode != joError.end()) {
            int errorCode = joiErrorCode.value().toInt(-1);
            if (const char* errorCodeEnum = SynoErrorGadget::metaEnum().valueToKey(errorCode)) {
                setErrorString(tr("SYNO error: %1 (%2)").arg(errorCodeEnum).arg(errorCode));
            } else {
                setErrorString(tr("SYNO error: UNKNOWN (%1)").arg(errorCode));
            }
        } else {
            setErrorString(tr("SYNO error: UNKNOWN"));
        }
    } else {
        QJsonValue jvSuccess = m_json.object()[QStringLiteral("success")];
        if (jvSuccess.toBool()) {
            m_dataObject = m_json.object()[QStringLiteral("data")].toObject();
        } else {
            setErrorString(tr("SYNO success: FALSE"));
        }
    }
}

SynoReplyJSONFactory::SynoReplyJSONFactory(QObject* parent)
    : QObject (parent)
{
}

QObject* SynoReplyJSONFactory::provider(QQmlEngine* engine, QJSEngine* scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    SynoReplyJSONFactory *factory = new SynoReplyJSONFactory();
    return factory;
}

QObject* SynoReplyJSONFactory::create(SynoRequest* synoRequest) const
{
    return new SynoReplyJSON(synoRequest);
}
