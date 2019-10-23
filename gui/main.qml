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
