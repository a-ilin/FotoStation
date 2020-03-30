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

#include <QDebug>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlPropertyMap>
#include <QQuickStyle>
#include <QQuickWindow>

#include "src/synops.h"
#include "src/synoimageprovider.h"

#if defined(Q_OS_ANDROID)
#define IS_MOBILE 1
#else
#define IS_MOBILE 0
#endif

static void populateRootContext(QQmlContext* ctx)
{
    QQmlPropertyMap* runtimeMap = new QQmlPropertyMap(ctx);
    runtimeMap->setObjectName(QStringLiteral("RuntimeObject"));

    // set debug flag
    bool isDebug = QCoreApplication::arguments().contains(QStringLiteral("--debug"));
    if (isDebug) {
        qInfo() << QStringLiteral("Debug mode is enabled");
    }
    runtimeMap->insert(QStringLiteral("isDebug"), isDebug);

    // set mobile flag
    runtimeMap->insert(QStringLiteral("isMobile"), static_cast<bool>(IS_MOBILE));

    // set platform Windows flag
    bool isPlatformWindows = false;
#ifdef Q_OS_WIN
    isPlatformWindows = true;
#endif
    runtimeMap->insert(QStringLiteral("isPlatformWindows"), isPlatformWindows);

    ctx->setContextProperty(QStringLiteral("Runtime"), runtimeMap);
}

void applyStyle()
{
#ifdef Q_OS_WIN
    QQuickStyle::setStyle(QStringLiteral("Universal"));
#endif
}

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    if (!IS_MOBILE) {
        QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    }

    QCoreApplication::setApplicationName("FotoStation");
    QCoreApplication::setOrganizationName("FotoStation");

    QGuiApplication app(argc, argv);

    // ClearType text looks terrible without this
    QQuickWindow::setTextRenderType(QQuickWindow::NativeTextRendering);

    SynoPS synoPS;
    QQmlApplicationEngine engine;

    SynoPS::registerTypes();
    SynoPS::registerQmlTypes();
    engine.addImageProvider("syno", new SynoImageProvider(synoPS.conn()));
    engine.addImportPath(GUI_PREFIX_PATH);

    populateRootContext(engine.rootContext());

    const QUrl url(QStringLiteral(GUI_PREFIX_PATH "/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
                         if (!obj && url == objUrl) {
                             qCritical() << QStringLiteral("Main QML cannot be loaded!");
                             QCoreApplication::exit(-1);
                         }
                     }, Qt::QueuedConnection);

    applyStyle();

    engine.load(url);

    return app.exec();
}
