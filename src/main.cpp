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

#include <QDebug>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlPropertyMap>
#include <QQuickWindow>

#include "src/synops.h"
#include "src/synoimageprovider.h"

static void populateRootContext(QQmlContext* ctx)
{
    QQmlPropertyMap* runtimeMap = new QQmlPropertyMap(ctx);
    runtimeMap->setObjectName(QStringLiteral("RuntimeObject"));

    bool isDebug = QCoreApplication::arguments().contains(QStringLiteral("--debug"));
    if (isDebug) {
        qInfo() << QStringLiteral("Debug mode is enabled");
    }
    runtimeMap->insert(QStringLiteral("isDebug"), isDebug);

    ctx->setContextProperty(QStringLiteral("Runtime"), runtimeMap);
}

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);

    QCoreApplication::setApplicationName("FotoStation");
    QCoreApplication::setOrganizationName("FotoStation");

    QGuiApplication app(argc, argv);

    // ClearType text looks terrible without this
    QQuickWindow::setTextRenderType(QQuickWindow::NativeTextRendering);

    QQmlApplicationEngine engine;
    SynoPS::registerQmlTypes();
    engine.addImageProvider("syno", new SynoImageProvider());
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
    engine.load(url);

    return app.exec();
}
