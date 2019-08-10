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
