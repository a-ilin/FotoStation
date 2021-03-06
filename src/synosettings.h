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

#ifndef SYNOSETTINGS_H
#define SYNOSETTINGS_H

#include <QJSValue>
#include <QObject>
#include <QQmlEngine>
#include <QSettings>
#include <QVariant>

#include <memory>

namespace QKeychain {
class Job;
class WritePasswordJob;
class ReadPasswordJob;
class DeletePasswordJob;
}

class SynoSettings : public QObject
{
    Q_OBJECT

    QML_ELEMENT

    Q_PROPERTY(QString group READ group WRITE setGroup NOTIFY groupChanged)

public:
    using SecureSuccessCallback = std::function<void(const QString& key)>;
    using SecureFailureCallback = std::function<void(const QString& key, const QString& error)>;

public:
    SynoSettings(const QString& group = QString());

    QString group() const;
    void setGroup(const QString& group);

    Q_INVOKABLE bool contains(const QString& key) const;
    Q_INVOKABLE QVariant value(const QString& key, const QVariant& defaultValue = QVariant()) const;
    Q_INVOKABLE void setValue(const QString& key, const QVariant& value);

    /*!
     * \brief Reads data for specified key from secure storage
     *
     * \param key Identifier of the data
     * \param callbackSuccess JS callback to be executed on success. Expected signature: void(string key, string secureValue)
     * \param callbackSuccess JS callback to be executed on failure or undefined. Expected signature: void(string key, string errorString)
     */
    Q_INVOKABLE void readSecure(const QString& key, QJSValue callbackSuccess, QJSValue callbackFailure = QJSValue());
    void readSecureC(const QString& key, QObject* context, std::function<void(const QString& key, const QString& secureValue)> callbackSuccess,
                     SecureFailureCallback callbackFailure = SecureFailureCallback());

    /*!
     * \brief Writes data for specified key to secure storage
     *
     * \param key Identifier of the data
     * \param secureValue Data to store
     * \param callbackSuccess JS callback to be executed on success or undefined. Expected signature: void(string key)
     * \param callbackSuccess JS callback to be executed on failure or undefined. Expected signature: void(string key, string errorString)
     */
    Q_INVOKABLE void saveSecure(const QString& key, const QString& secureValue, QJSValue callbackSuccess = QJSValue(), QJSValue callbackFailure = QJSValue());
    void saveSecureC(const QString& key, const QString& secureValue, QObject* context = nullptr,
                     SecureSuccessCallback callbackSuccess = SecureSuccessCallback(),
                     SecureFailureCallback callbackFailure = SecureFailureCallback());

    /*!
     * \brief Deletes data for specified key from secure storage
     *
     * \param key Identifier of the data
     * \param callbackSuccess JS callback to be executed on success or undefined. Expected signature: void(string key)
     * \param callbackSuccess JS callback to be executed on failure or undefined. Expected signature: void(string key, string errorString)
     */
    Q_INVOKABLE void deleteSecure(const QString& key, QJSValue callbackSuccess = QJSValue(), QJSValue callbackFailure = QJSValue());
    void deleteSecureC(const QString& key, QObject* context = nullptr,
                       SecureSuccessCallback callbackSuccess = SecureSuccessCallback(),
                       SecureFailureCallback callbackFailure = SecureFailureCallback());

    /*!
     * \brief Returns TRUE if the application is installed into system directory
     */
    static bool isAppInstalled();

signals:
    void groupChanged();

private:
    void endGroups();
    void executeKeyChainJob(QKeychain::Job* job, QObject* context,
                            SecureSuccessCallback callbackSuccess,
                            SecureFailureCallback callbackFailure);
    SecureSuccessCallback jsCallbackSuccess(QJSValue callbackSuccess);
    SecureFailureCallback jsCallbackFailure(QJSValue callbackFailure);

    static QString securelyHashedKey(const QString& key);

private:
    std::unique_ptr<QSettings> m_settings;
};

#endif // SYNOSETTINGS_H
