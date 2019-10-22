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

#ifndef SYNOPS_H
#define SYNOPS_H

#include <functional>

#include <QObject>

class QJSEngine;
class QQmlEngine;

class SynoAlbum;
class SynoConn;
class SynoPSPrivate;

class SynoPS : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(SynoPS)
    Q_DISABLE_COPY(SynoPS)

    Q_PROPERTY(SynoConn* conn READ conn NOTIFY connChanged)

public:
    static SynoPS& instance();
    static QObject* fromQmlEngine(QQmlEngine* engine, QJSEngine* scriptEngine);

    Q_INVOKABLE SynoAlbum* getRootAlbum();

    SynoConn* conn();

    static void registerQmlTypes();

    Q_INVOKABLE static QString toString(const QVariant& value);

protected:
    explicit SynoPS(QObject *parent = nullptr);

signals:
    // this signal is never emitted, it is added to suppress
    // Qt warning about non-NOTIFYable properties
    void connChanged();
};

#endif // SYNOPS_H
