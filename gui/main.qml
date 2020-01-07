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

import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.13
import QtQuick.Window 2.13

import FotoStation 1.0

ApplicationWindow {
    id: root

    width: 800
    minimumWidth: 640
    height: 600
    minimumHeight: 480

    title: qsTr("FotoStation")

    menuBar: MenuBar {
        Menu {
            title: qsTr("&File")
            Action { text: qsTr("&New...") }
            Action { text: qsTr("&Open...") }
            Action { text: qsTr("&Save") }
            Action { text: qsTr("Save &As...") }
            MenuSeparator { }
            Action { text: qsTr("&Quit") }
        }
        Menu {
            title: qsTr("&Edit")
            Action { text: qsTr("Cu&t") }
            Action { text: qsTr("&Copy") }
            Action { text: qsTr("&Paste") }
        }
        Menu {
            title: qsTr("&Help")
            Action { text: qsTr("&About") }
        }
    }

    footer: Loader {
        Component.onCompleted: {
            setSource(internal.footerUrl);
        }
    }

    onClosing: {
        internal.saveWindowGeometry();
    }

    SynoSettings {
        id: _settings
        group: "main"
    }

    FocusScope {
        anchors.fill: parent
        focus: true

        Keys.onPressed: {
            if (Runtime.isDebug && event.key === Qt.Key_D && (event.modifiers & Qt.ControlModifier)) {
                event.accepted = true;
                internal.showDebugWindow();
            }
        }

        Loader {
            id: _loader
            anchors.fill: parent
        }
    }

    Connections {
        target: SynoPS.conn
        onStatusChanged: {
            if (SynoPS.conn.status === SynoConn.AUTHORIZED) {
                internal.showBaseScreenViewForm();
            } else if (SynoPS.conn.status === SynoConn.DISCONNECTED) {
                internal.showAuthorizationForm();
            }
        }
    }

    QtObject {
        id: internal

        property var debugWindow: null

        readonly property url debugViewUrl: Qt.resolvedUrl("FotoStation/screens/DebugView.qml")
        readonly property url baseScreenViewDesktopUrl: Qt.resolvedUrl("FotoStation/screens/BaseScreenViewDesktop.qml")
        readonly property url footerUrl: Qt.resolvedUrl("FotoStation/screens/Footer.qml")
        readonly property url loginViewUrl: Qt.resolvedUrl("FotoStation/screens/LoginView.qml")

        function showAuthorizationForm() {
            _loader.setSource(internal.loginViewUrl);
        }

        function showBaseScreenViewForm() {
            if (Runtime.isMobile) {
                // TBD: implement
            } else {
                _loader.setSource(internal.baseScreenViewDesktopUrl);
            }
        }

        function showDebugWindow() {
            if (!internal.debugWindow) {
                var comp = Qt.createComponent(internal.debugViewUrl);
                var window = comp.createObject(root);
                if (window) {
                    internal.debugWindow = window;
                    internal.debugWindow.show();
                }
            } else {
                internal.debugWindow.show();
                internal.debugWindow.raise();
                internal.debugWindow.requestActivate();
            }
        }

        function saveWindowGeometry() {
            _settings.setValue("windowRect", Qt.rect(root.x, root.y,
                                                     root.width, root.height));
            _settings.setValue("isWindowMaximized", root.visibility === Window.Maximized);
        }

        function restoreWindowGeometry() {
            if (_settings.contains("windowRect")) {
                var rect = _settings.value("windowRect");
                root.width = Math.max(root.minimumWidth,
                                      Math.min(rect.width, root.screen.desktopAvailableWidth));
                root.height = Math.max(root.minimumHeight,
                                       Math.min(rect.height, root.screen.desktopAvailableHeight));

                if (rect.y < root.screen.desktopAvailableHeight &&
                    rect.y + root.height > 0 &&
                    rect.x < root.screen.desktopAvailableWidth &&
                    rect.x + root.width > 0) {

                    root.x = rect.x;
                    root.y = rect.y;
                }
            }

            if (_settings.value("isWindowMaximized")) {
                root.showMaximized();
            } else {
                root.showNormal();
            }
        }
    }

    Component.onCompleted: {
        internal.showAuthorizationForm();
        internal.restoreWindowGeometry();
        root.visible = true;
    }
}
