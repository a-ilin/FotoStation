/*
 * GNU General Public License (GPL)
 * Copyright (c) 2020 by Aleksei Ilin
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

import QtQuick 2.15

import FotoStation 1.0
import FotoStation.widgets 1.0

FocusScope {
    id: root

    property var albumView

    signal opened();
    signal closed();

    function open() {
        _panel.forceActiveFocus();
        opened();
    }

    function close() {
        closed();
    }

    focus: true

    Keys.forwardTo: [_panel, albumView]

    Rectangle {
        anchors.fill: parent
        color: "black"
    }

    FSCoverArt {
        id: _imagePrev

        anchors.fill: parent

        sourceSizeHeight: height
        sourceSizeWidth: width
        fillMode: Image.PreserveAspectFit
    }

    FSCoverArt {
        id: _image

        anchors.fill: parent

        sourceSizeHeight: height
        sourceSizeWidth: width
        fillMode: Image.PreserveAspectFit

        source: Facade.coverFullUrl(root.albumView.selectedImageId)
        backupSource: Facade.coverThumbUrl(root.albumView.selectedImageId)

        onIsLoadedChanged: {
            if (isLoaded) {
                _imagePrev.source = source;
            }
        }

        onIsBackupLoadedChanged: {
            if (isBackupLoaded) {
                _imagePrev.backupSource = backupSource;
            }
        }
    }

    Item {
        id: _panel
        anchors.fill: parent

        Keys.onPressed: {
            if (event.key === Qt.Key_Escape
             || event.key === Qt.Key_Return
             || event.key === Qt.Key_Enter) {
                event.accepted = true;
                root.close();
            }
        }
    }
}
