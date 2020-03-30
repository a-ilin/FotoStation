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

#include "synops.h"

#include <QtCore/private/qobject_p.h>
#include <QtQml>

#include "synoalbum.h"
#include "synoalbumfactory.h"
#include "synoauth.h"
#include "synoconn.h"
#include "synorequest.h"
#include "synoreplyjson.h"
#include "synosettings.h"
#include "synosize.h"
#include "synosslconfig.h"

static SynoPS* g_synoPS = nullptr;

class SynoPSPrivate : public QObjectPrivate
{
public:
    SynoPSPrivate()
        : QObjectPrivate()
    {
    }

    SynoConn conn;
};

SynoPS::SynoPS()
    : QObject(*new SynoPSPrivate(), nullptr)
{
    g_synoPS = this;
}

SynoPS::~SynoPS()
{
    g_synoPS = nullptr;
}

SynoPS* SynoPS::instance()
{
    return g_synoPS;
}

QObject* SynoPS::fromQmlEngine(QQmlEngine* engine, QJSEngine* scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    SynoPS* synoPS = SynoPS::instance();
    QQmlEngine::setObjectOwnership(synoPS, QQmlEngine::CppOwnership);
    return synoPS;
}

SynoConn* SynoPS::conn()
{
    Q_D(SynoPS);
    return &d->conn;
}

void SynoPS::registerTypes()
{
    qRegisterMetaType<SynoAlbumData>();
}

void SynoPS::registerQmlTypes()
{
    const char* qmlUrl = "FotoStation";

    qmlRegisterSingletonType<SynoPS>(qmlUrl, 1, 0, "SynoPS", SynoPS::fromQmlEngine);
    qmlRegisterSingletonType<SynoReplyJSONFactory>(qmlUrl, 1, 0, "SynoReplyJSONFactory", SynoReplyJSONFactory::fromQmlEngine);
    qmlRegisterSingletonType<SynoAlbumFactory>(qmlUrl, 1, 0, "SynoAlbumFactory", SynoAlbumFactory::fromQmlEngine);
    qmlRegisterSingletonType<SynoSizeGadget>(qmlUrl, 1, 0, "SynoSize", SynoSizeGadget::fromQmlEngine);
    qmlRegisterType<SynoSettings>(qmlUrl, 1, 0, "SynoSettings");
    qmlRegisterUncreatableType<SynoAlbum>(qmlUrl, 1, 0, "SynoAlbum", "");
    qmlRegisterUncreatableType<SynoConn>(qmlUrl, 1, 0, "SynoConn", "");
    qmlRegisterUncreatableType<SynoRequest>(qmlUrl, 1, 0, "SynoRequest", "");
    qmlRegisterUncreatableType<SynoSslConfig>(qmlUrl, 1, 0, "SynoConn", "");
    qmlRegisterUncreatableType<SynoAuth>(qmlUrl, 1, 0, "SynoAuth", "");
}

QString SynoPS::toString(const QVariant& value)
{
    if (value.isNull()) {
        return QString();
    }

    if (value.type() == QVariant::ByteArray) {
        // expect it to be utf-8
        return QString::fromUtf8(value.toByteArray());
    }

    // use general conversion
    return value.toString();
}

QVariantMap SynoPS::urlToMap(const QUrl& value)
{
    QVariantMap result;

    result[QStringLiteral("protocol")] = value.scheme();
    result[QStringLiteral("hostname")] = value.host();
    result[QStringLiteral("port")] = value.port();
    result[QStringLiteral("pathname")] = value.path();

    return result;
}

QUrl SynoPS::urlFromMap(const QVariantMap& value)
{
    QUrl url;

    url.setScheme(value.value(QStringLiteral("protocol")).toString());
    url.setHost(value.value(QStringLiteral("hostname")).toString());
    url.setPort(value.value(QStringLiteral("port"), -1).toInt());

    QString pathname = value.value(QStringLiteral("pathname")).toString();
    int pathnameIdx = 0;
    for (; pathnameIdx < pathname.size(); ++pathnameIdx) {
        if (pathname[pathnameIdx] != '/') {
            break;
        }
    }
    pathname = QStringLiteral("/") + pathname.midRef(pathnameIdx);
    url.setPath(pathname);

    return url;
}
