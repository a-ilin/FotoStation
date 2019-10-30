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

#include <QObject>
#include <QSettings>
#include <QVariant>

#include <memory>

class SynoSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString group READ group WRITE setGroup NOTIFY groupChanged)

public:
    SynoSettings(const QString& group = QString());

    QString group() const;
    void setGroup(const QString& group);

    Q_INVOKABLE bool contains(const QString& key) const;
    Q_INVOKABLE QVariant value(const QString& key, const QVariant& defaultValue = QVariant()) const;
    Q_INVOKABLE void setValue(const QString& key, const QVariant& value);

    /*!
     * \brief Returns TRUE if the application is installed into system directory
     */
    static bool isAppInstalled();

signals:
    void groupChanged();

private:
    void endGroups();

private:
    std::unique_ptr<QSettings> m_settings;
};

#endif // SYNOSETTINGS_H
