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
#include <QDir>

#include "synosettings.h"

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

QVariant SynoSettings::value(const QString& key, const QVariant& defaultValue) const
{
    return m_settings->value(key, defaultValue);
}

void SynoSettings::setValue(const QString& key, const QVariant& value)
{
    m_settings->setValue(key, value);
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
