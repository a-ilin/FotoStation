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

import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.13
import QtQuick.Window 2.13

import FotoStation 1.0

ApplicationWindow {
    id: root

    visible: true

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
                internal.showAlbumViewForm();
            } else if (SynoPS.conn.status === SynoConn.DISCONNECTED) {
                internal.showAuthorizationForm();
            }
        }
    }

    QtObject {
        id: internal

        property var debugWindow: null

        readonly property url debugViewUrl: Qt.resolvedUrl("FotoStation/screens/DebugView.qml")
        readonly property url albumViewUrl: Qt.resolvedUrl("FotoStation/screens/AlbumView.qml")
        readonly property url footerUrl: Qt.resolvedUrl("FotoStation/screens/Footer.qml")
        readonly property url loginViewUrl: Qt.resolvedUrl("FotoStation/screens/LoginView.qml")

        function showAuthorizationForm() {
            _loader.setSource(internal.loginViewUrl);
        }

        function showAlbumViewForm() {
            _loader.setSource(internal.albumViewUrl);
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
    }

    Component.onCompleted: {
        internal.showAuthorizationForm();
    }
}
