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

import QtGraphicalEffects 1.0 as QGE
import QtQuick 2.13
import QtQuick.Controls 2.13

import FotoStation 1.0
import FotoStation.assets 1.0
import FotoStation.widgets 1.0

FocusScope {
    id: root

    /*! This property holds instance of SynoAlbum */
    property var synoAlbum: null

    /*! This property holds id of selected image */
    readonly property var selectedImageId: _view.currentItem ? _view.currentItem.imageId : null

    focus: true

    MouseArea {
        anchors.fill: _view
        onPressed: {
            _view.forceActiveFocus();
        }
    }

    GridView {
        id: _view

        readonly property int border: 1
        readonly property int visibleItemCount: width * height / cellWidth / cellHeight

        anchors.fill: parent
        anchors.margins: 4

        cellWidth: 150
        cellHeight: 120 + _albumTitleMetrics.height + border * 2

        focus: true

        snapMode: GridView.SnapToRow

        ScrollBar.vertical: ScrollBar {
            active: true
        }

        Keys.onPressed: {
            if (event.key === Qt.Key_Home) {
                currentIndex = 0;
                event.accepted = true;
            } else if (event.key === Qt.Key_End) {
                currentIndex = Math.max(0, count - 1);
                event.accepted = true;
            } else if (event.key === Qt.Key_PageUp) {
                currentIndex = Math.max(0, currentIndex - visibleItemCount);
            } else if (event.key === Qt.Key_PageDown) {
                currentIndex = Math.min(_view.count - 1, currentIndex + visibleItemCount);
            }
        }

        model: root.synoAlbum
        delegate: Rectangle {
            id: _delegate

            readonly property var imageId: model.synoData.id

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
                image.sourceSize.width: width
                image.source: Facade.coverUrl(imageId)
            }

            Rectangle {
                color: index === _view.currentIndex ? "lightblue" : "white"
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
                text: model.display !== "" ? model.display : qsTr("Loading...")
                elide: _albumTitleMetrics.elide
                wrapMode: Text.Wrap
                maximumLineCount: 2
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    _view.currentIndex = index;
                    _view.positionViewAtIndex(index, GridView.Contain);
                }
            }
        }

        onCountChanged: {
            if (currentIndex === -1 && count > 0) {
                currentIndex = 0;
                positionViewAtBeginning();
            }
        }

        TextMetrics {
            id: _albumTitleMetrics
            elide: Qt.ElideRight
            text: "MMM"
        }

        AnimatedImage {
            id: _loadingIndicator
            source: _view.count === 0 ? Assets.animated.roller_64 : ""
            visible: false
        }

        QGE.ColorOverlay {
            anchors.centerIn: parent
            width: _loadingIndicator.width
            height: _loadingIndicator.height
            source: _loadingIndicator
            color: Assets.colors.iconDefault
            visible: _loadingIndicator.source !== ""
        }
    }
}
