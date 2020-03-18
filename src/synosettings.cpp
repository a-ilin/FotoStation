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

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDebug>
#include <QDir>

#ifdef USE_KEYCHAIN
#include <qtkeychain/keychain.h>
#endif

#include "synosettings.h"

#define KEYCHAIN_SERVICE_NAME QStringLiteral("SynoFotoStation")

template <typename... Args>
bool executeJSCallback(QJSValue callback, Args... args) {
    if (!callback.isCallable()) {
        qWarning() << __FUNCTION__ << QObject::tr("JS callback is not callable.");
        return false;
    }

    QJSValue result = callback.call(QList<QJSValue>{ std::forward<Args>(args)... });
    if (result.isError()) {
        qWarning() << __FUNCTION__
                   << QObject::tr("Exception on JS callback. ")
                   << result.property(QStringLiteral("fileName")).toString()
                   << QStringLiteral(":")
                   << result.property(QStringLiteral("lineNumber")).toInt()
                   << result.property(QStringLiteral("message")).toString()
                   << QStringLiteral("Stack: [")
                   << result.property(QStringLiteral("stack")).toString()
                   << QStringLiteral("] ")
                   << result.toString();
        return false;
    }

    return true;
}

SynoSettings::SynoSettings(const QString& group)
{
    if (isAppInstalled()) {
        m_settings.reset(new QSettings(QSettings::IniFormat,
                                       QSettings::UserScope,
                                       QCoreApplication::organizationName(),
                                       QCoreApplication::applicationName()));
    } else {
        m_settings.reset(new QSettings(QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("FotoStation.ini"),
                                       QSettings::IniFormat));
    }

    m_settings->setIniCodec("UTF-8");

    setGroup(group);
}

QString SynoSettings::group() const
{
    return m_settings->group();
}

void SynoSettings::setGroup(const QString& group)
{
    if (m_settings->group() != group) {
        endGroups();
        m_settings->beginGroup(group);
        emit groupChanged();
    }
}

bool SynoSettings::contains(const QString& key) const
{
    return m_settings->contains(key);
}

QVariant SynoSettings::value(const QString& key, const QVariant& defaultValue) const
{
    QVariant v = m_settings->value(key, defaultValue);

    // convert "true" and "false" to true and false
    if (v.type() == QVariant::String) {
        QString vs = v.toString();
        if (vs == QStringLiteral("true")) {
            v = true;
        } else if (vs == QStringLiteral("false")) {
            v = false;
        }
    }

    return v;
}

void SynoSettings::setValue(const QString& key, const QVariant& value)
{
    m_settings->setValue(key, value);
}

void SynoSettings::readSecure(const QString& key, QJSValue callbackSuccess, QJSValue callbackFailure)
{
#ifdef USE_KEYCHAIN
    QKeychain::ReadPasswordJob* job = new QKeychain::ReadPasswordJob(KEYCHAIN_SERVICE_NAME);
    job->setKey(securelyHashedKey(key));

    connect(job, &QKeychain::Job::finished, this, [=] (QKeychain::Job*) {
        if (job->error() == QKeychain::NoError) {
            executeJSCallback(callbackSuccess, QJSValue(key), QJSValue(job->textData()));
        } else if (!callbackFailure.isUndefined()) {
            executeJSCallback(callbackFailure, QJSValue(key), QJSValue(job->errorString()));
        }
    });

    job->start();
#else
    Q_UNUSED(callbackSuccess)
    if (!callbackFailure.isUndefined()) {
        executeJSCallback(callbackFailure, QJSValue(key), QJSValue(tr("Keychain not enabled")));
    }
#endif
}

void SynoSettings::saveSecure(const QString& key, const QString& secureValue, QJSValue callbackSuccess, QJSValue callbackFailure)
{
#ifdef USE_KEYCHAIN
    QKeychain::WritePasswordJob* job = new QKeychain::WritePasswordJob(KEYCHAIN_SERVICE_NAME);
    job->setKey(securelyHashedKey(key));
    job->setTextData(secureValue);

    connect(job, &QKeychain::Job::finished, this, [=] (QKeychain::Job*) {
        if (job->error() == QKeychain::NoError && !callbackSuccess.isUndefined()) {
            executeJSCallback(callbackSuccess, QJSValue(key));
        } else if (!callbackFailure.isUndefined()) {
            executeJSCallback(callbackFailure, QJSValue(key), QJSValue(job->errorString()));
        }
    });

    job->start();
#else
    Q_UNUSED(callbackSuccess)
    Q_UNUSED(secureValue)
    if (!callbackFailure.isUndefined()) {
        executeJSCallback(callbackFailure, QJSValue(key), QJSValue(tr("Keychain not enabled")));
    }
#endif
}

void SynoSettings::deleteSecure(const QString& key, QJSValue callbackSuccess, QJSValue callbackFailure)
{
#ifdef USE_KEYCHAIN
    QKeychain::DeletePasswordJob* job = new QKeychain::DeletePasswordJob(KEYCHAIN_SERVICE_NAME);
    job->setKey(securelyHashedKey(key));

    connect(job, &QKeychain::Job::finished, this, [=] (QKeychain::Job*) {
        if (job->error() == QKeychain::NoError && !callbackSuccess.isUndefined()) {
            executeJSCallback(callbackSuccess, QJSValue(key));
        } else if (!callbackFailure.isUndefined()) {
            executeJSCallback(callbackFailure, QJSValue(key), QJSValue(job->errorString()));
        }
    });

    job->start();
#else
    Q_UNUSED(callbackSuccess)
    if (!callbackFailure.isUndefined()) {
        executeJSCallback(callbackFailure, QJSValue(key), QJSValue(tr("Keychain not enabled")));
    }
#endif
}

bool SynoSettings::isAppInstalled()
{
    return QDir(QCoreApplication::applicationDirPath()).exists(".installed");
}

void SynoSettings::endGroups()
{
    while (!m_settings->group().isEmpty()) {
        m_settings->endGroup();
    }
}

QString SynoSettings::securelyHashedKey(const QString& key)
{
    return KEYCHAIN_SERVICE_NAME + ":" + QCryptographicHash::hash(key.toUtf8(), QCryptographicHash::Sha1).toHex();
}
