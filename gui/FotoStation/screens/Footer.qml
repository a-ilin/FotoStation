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

import FotoStation 1.0
import FotoStation.assets 1.0

Rectangle {
    id: root

    implicitHeight: Math.max(_connectionProgressImage.height, _fm.height)
                    + internal.borderMargin * 2 + _underline.height
    implicitWidth: parent.width

    color: palette.button

    Rectangle {
        id: _underline
        height: 1
        width: parent.width
        color: internal.borderColor
    }

    Row {
        id: _layout
        anchors.top: _underline.bottom
        anchors.left: parent.left
        anchors.right: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: internal.borderMargin

        spacing: internal.borderMargin

        AnimatedImage {
            id: _connectionProgressImage
            height: 16
            width: 16
            source: SynoPS.conn.isConnecting ? Assets.animated.roller_16 : ""
        }

        Rectangle {
            height: root.height
            y: -internal.borderMargin
            width: 1
            color: internal.borderColor
        }

        Label {
            id: _connectionStatus
            Layout.preferredWidth: root.width * 0.25
            Layout.maximumWidth: root.width * 0.25
            elide: Text.ElideRight
            width: root.width * 0.25

            text: {
                if (SynoPS.conn.isConnecting) {
                    return qsTr("Connecting");
                } else if(SynoPS.conn.status === SynoConn.AUTHORIZED) {
                    return qsTr("Logged in");
                } else {
                    return qsTr("Not connected");
                }
            }
        }

        Rectangle {
            height: root.height
            y: -internal.borderMargin
            width: 1
            color: internal.borderColor
        }
    }

    TextMetrics {
        id: _fm
        text: "M"
    }

    QtObject {
        id: internal

        readonly property string borderColor: palette.mid
        readonly property int borderMargin: _fm.height * 0.3
    }
}
