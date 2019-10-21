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

#include <QCoreApplication>
#include <QDir>
#include <QSettings>

#include "synosettings.h"

SynoSettings::SynoSettings()
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

    m_settings->setIniCodec("UTF-16");
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

QVariant SynoSettings::value(const QString& key) const
{
    return m_settings->value(key);
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
