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

#ifndef SYNOPS_H
#define SYNOPS_H

#include <QObject>
#include <QUrl>
#include <QVariantMap>

class QJSEngine;
class QQmlEngine;

class SynoConn;
class SynoPSPrivate;

class SynoPS : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(SynoPS)
    Q_DISABLE_COPY(SynoPS)

    Q_PROPERTY(SynoConn* conn READ conn NOTIFY connChanged)

public:

    /*!
     * \brief This method returns instance of connection object
     */
    static SynoPS* instance();
    static QObject* fromQmlEngine(QQmlEngine* engine, QJSEngine* scriptEngine);

    SynoConn* conn();

    static void registerTypes();
    static void registerQmlTypes();

    /*!
     * \brief This method parses url fields into variant map
     *
     * This method is intended to be used from QML.
     */
    Q_INVOKABLE static QVariantMap urlToMap(const QUrl& value);

    /*!
     * \brief This method creates url object from variant map
     *
     * This method is intended to be used from QML.
     */
    Q_INVOKABLE static QUrl urlFromMap(const QVariantMap& value);

    ~SynoPS();

signals:
    // this signal is never emitted, it is added to suppress
    // Qt warning about non-NOTIFYable properties
    void connChanged();

protected:
    SynoPS();

    friend int main(int, char**);
};

#endif // SYNOPS_H
