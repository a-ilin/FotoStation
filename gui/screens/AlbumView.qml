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

import FotoStation 1.0

Item {
    id: root

    /*! This property holds instance of SynoPS */
    property var synoPS: null

    /*! This property holds instance of SynoAlbum */
    property var synoAlbum: null

    GridView {
        id: _view

        anchors.fill: parent

        model: root.synoAlbum
        delegate: Rectangle {
            color: Qt.rgba(Math.random(), Math.random(), Math.random(), 1.0)

            Column {
                Text {
                    text: index
                }
                Text {
                    text: model.display
                }
                Text {
                    text: model.synoData ? model.synoData.type : ""
                }
            }
        }
    }

    Component.onCompleted: {
        if (!root.synoAlbum) {
            root.synoAlbum = synoPS.getRootAlbum();
        }

        root.synoAlbum.refresh();
    }
}
