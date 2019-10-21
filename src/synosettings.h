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

#ifndef SYNOSETTINGS_H
#define SYNOSETTINGS_H

#include <QObject>
#include <QVariant>

#include <memory>

class QSettings;

class SynoSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString group READ group WRITE setGroup NOTIFY groupChanged)

public:
    SynoSettings();

    QString group() const;
    void setGroup(const QString& group);

    Q_INVOKABLE QVariant value(const QString& key) const;
    Q_INVOKABLE void setValue(const QString& key, const QVariant& value);

    /*! Returns true if the application is installed into system directory */
    static bool isAppInstalled();

    /*! Initialize before the first use */
    static void initialize();

signals:
    void groupChanged();

private:
    void endGroups();

private:
    std::unique_ptr<QSettings> m_settings;
};

#endif // SYNOSETTINGS_H
