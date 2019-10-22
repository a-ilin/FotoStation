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
