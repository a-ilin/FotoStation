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
