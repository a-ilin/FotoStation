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
import FotoStation.widgets 1.0

Item {
    id: root

    /*! This property holds instance of SynoAlbum */
    property var synoAlbum: null

    GridView {
        id: _view

        readonly property int border: 1

        anchors.fill: parent

        cellWidth: 150
        cellHeight: 120 + _albumTitleMetrics.height + border * 2

        model: root.synoAlbum
        delegate: Rectangle {
            id: _delegate

            width: _view.cellWidth - 4
            height: _view.cellHeight - 4

            color: "black"

            FSCoverArt {
                id: _albumCover
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - _view.border * 2
                anchors.top: parent.top
                anchors.topMargin: _view.border
                anchors.bottom: _albumTitle.top
                anchors.bottomMargin: _view.border
                image.source: Facade.coverUrlForThumb(model.synoData.id)
            }

            Rectangle {
                color: "white"
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - _view.border * 2
                anchors.top: _albumCover.bottom
                anchors.topMargin: _view.border
                anchors.bottom: parent.bottom
                anchors.bottomMargin: _view.border
            }

            Text {
                id: _albumTitle
                anchors.bottom: parent.bottom
                anchors.bottomMargin: _view.border
                anchors.horizontalCenter: parent.horizontalCenter
                height: _albumTitleMetrics.height * maximumLineCount
                width: parent.width - _view.border * 2
                text: model.display
                elide: _albumTitleMetrics.elide
                wrapMode: Text.Wrap
                maximumLineCount: 2
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }

        TextMetrics {
            id: _albumTitleMetrics
            elide: Qt.ElideRight
            text: "MMM"
        }
    }

    Component.onCompleted: {
        if (!root.synoAlbum) {
            root.synoAlbum = SynoPS.getRootAlbum();
        }

        root.synoAlbum.refresh();
    }
}
