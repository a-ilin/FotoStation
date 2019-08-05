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

#include "synops.h"

#include <QtCore/private/qobject_p.h>
#include <QtQml>

#include "synoalbum.h"
#include "synoconn.h"

class SynoPSPrivate : public QObjectPrivate
{
public:
    SynoPSPrivate()
        : QObjectPrivate()
    {
    }

    SynoConn conn;
};

SynoPS::SynoPS(QObject *parent)
    : QObject(*new SynoPSPrivate(), parent)
{
}

SynoAlbum* SynoPS::getRootAlbum()
{
    return new SynoAlbum(conn(), this);
}

SynoConn* SynoPS::conn()
{
    Q_D(SynoPS);
    return &d->conn;
}

void SynoPS::registerQmlTypes()
{
    const char* qmlUrl = "FotoStation";

    qmlRegisterType<SynoPS>(qmlUrl, 1, 0, "SynoPS");
    qmlRegisterUncreatableType<SynoAlbum>(qmlUrl, 1, 0, "SynoAlbum", "");
    qmlRegisterUncreatableType<SynoConn>(qmlUrl, 1, 0, "SynoConn", "");
}

