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
#include <QPointer>

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
    readSecureC(key, this, [callbackSuccess] (const QString& key, const QString& secureValue) {
        if (!callbackSuccess.isUndefined()) {
            executeJSCallback(callbackSuccess, QJSValue(key), QJSValue(secureValue));
        }
    }, jsCallbackFailure(callbackFailure));
}

void SynoSettings::readSecureC(const QString& key, QObject* context,
                               std::function<void(const QString& key, const QString& error)> callbackSuccess, SecureFailureCallback callbackFailure)
{
#ifdef USE_KEYCHAIN
    QKeychain::ReadPasswordJob* job = new QKeychain::ReadPasswordJob(KEYCHAIN_SERVICE_NAME);
    job->setSettings(m_settings.get());
    job->setKey(securelyHashedKey(key));

    QPointer<QObject> contextPtr(context);
    auto realCallbackSuccess = [contextPtr, job, callbackSuccess] (const QString& key) {
        if (contextPtr && callbackSuccess) {
            callbackSuccess(key, job->textData());
        }
    };

    executeKeyChainJob(job, context, realCallbackSuccess, callbackFailure);
#else
    Q_UNUSED(callbackSuccess)
    Q_UNUSED(key)
    if (context && callbackFailure) {
        callbackFailure(key, tr("Keychain not enabled"));
    }
#endif
}

void SynoSettings::saveSecure(const QString& key, const QString& secureValue,
                              QJSValue callbackSuccess, QJSValue callbackFailure)
{
    saveSecureC(key, secureValue, this, jsCallbackSuccess(callbackSuccess), jsCallbackFailure(callbackFailure));
}

void SynoSettings::saveSecureC(const QString& key, const QString& secureValue, QObject* context, SecureSuccessCallback callbackSuccess, SecureFailureCallback callbackFailure)
{
#ifdef USE_KEYCHAIN
    QKeychain::WritePasswordJob* job = new QKeychain::WritePasswordJob(KEYCHAIN_SERVICE_NAME);
    job->setSettings(m_settings.get());
    job->setKey(securelyHashedKey(key));
    job->setTextData(secureValue);
    executeKeyChainJob(job, context, callbackSuccess, callbackFailure);
#else
    Q_UNUSED(callbackSuccess)
    Q_UNUSED(secureValue)
    if (context && callbackFailure) {
        callbackFailure(key, tr("Keychain not enabled"));
    }
#endif
}

void SynoSettings::deleteSecure(const QString& key, QJSValue callbackSuccess, QJSValue callbackFailure)
{
    deleteSecureC(key, this, jsCallbackSuccess(callbackSuccess), jsCallbackFailure(callbackFailure));
}

void SynoSettings::deleteSecureC(const QString& key, QObject* context,
                                 SecureSuccessCallback callbackSuccess, SecureFailureCallback callbackFailure)
{
#ifdef USE_KEYCHAIN
    QKeychain::DeletePasswordJob* job = new QKeychain::DeletePasswordJob(KEYCHAIN_SERVICE_NAME);
    job->setSettings(m_settings.get());
    job->setKey(securelyHashedKey(key));
    executeKeyChainJob(job, context, callbackSuccess, callbackFailure);
#else
    Q_UNUSED(callbackSuccess)
    if (context && callbackFailure) {
        callbackFailure(key, tr("Keychain not enabled"));
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

void SynoSettings::executeKeyChainJob(QKeychain::Job* job, QObject* context,
                                      std::function<void(const QString& key)> callbackSuccess,
                                      std::function<void(const QString& key, const QString& error)> callbackFailure)
{
#ifdef USE_KEYCHAIN
    QPointer<QObject> contextPtr(context);
    connect(job, &QKeychain::Job::finished, this,
            [contextPtr, callbackSuccess, callbackFailure] (QKeychain::Job* job) {
        if (contextPtr) {
            if (job->error() == QKeychain::NoError) {
                if (callbackSuccess) {
                    callbackSuccess(job->key());
                }
            } else {
                qCritical() << tr("Keychain error: ") << job->errorString();
                if (callbackFailure) {
                    callbackFailure(job->key(), job->errorString());
                }
            }
        }
    });

    job->start();
#else
    Q_UNUSED(job)
    Q_UNUSED(context)
    Q_UNUSED(callbackSuccess)
    Q_UNUSED(callbackFailure)
#endif
}

SynoSettings::SecureSuccessCallback SynoSettings::jsCallbackSuccess(QJSValue callbackSuccess)
{
    return [callbackSuccess] (const QString& key) {
        if (!callbackSuccess.isUndefined()) {
            executeJSCallback(callbackSuccess, QJSValue(key));
        }
    };
}

SynoSettings::SecureFailureCallback SynoSettings::jsCallbackFailure(QJSValue callbackFailure)
{
    return [callbackFailure] (const QString& key, const QString& error) {
        if (!callbackFailure.isUndefined()) {
            executeJSCallback(callbackFailure, QJSValue(key), QJSValue(error));
        }
    };
}

QString SynoSettings::securelyHashedKey(const QString& key)
{
#if !defined(Q_OS_WIN) || defined(USE_CREDENTIAL_STORE)
    return KEYCHAIN_SERVICE_NAME + ":" + QCryptographicHash::hash(key.toUtf8(), QCryptographicHash::Sha1).toHex();
#else
    return key;
#endif
}
