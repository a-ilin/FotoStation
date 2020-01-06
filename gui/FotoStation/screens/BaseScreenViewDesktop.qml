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
import QtQuick.Controls 1.4 as Controls_1
import QtQuick.Controls 2.13

import FotoStation 1.0
import FotoStation.assets 1.0
import FotoStation.widgets 1.0

FocusScope {
    id: root

    /*! This property holds instance of current SynoAlbum */
    property alias synoAlbum: _albumView.synoAlbum

    focus: true

    Rectangle {
        anchors.fill: parent
        color: Assets.appPalette.light
    }

    MouseArea {
        anchors.fill: parent
        onPressed: {
            parent.forceActiveFocus();
        }
    }

    SplitView {
        id: _mainView

        anchors.fill: parent

        orientation: Qt.Horizontal

        SplitView {
            id: _sideView

            SplitView.minimumWidth: 50
            SplitView.preferredWidth: root.width / 3

            orientation: Qt.Vertical

            Controls_1.TreeView {

            }

            Rectangle {
                color: Assets.appPalette.shadow

                SplitView.preferredHeight: root.height / 2

                FSCoverArt {
                    anchors.fill: parent
                    image.sourceSize.width: width
                    image.source: Facade.coverUrl(_albumView.selectedImageId)
                    image.fillMode: Image.PreserveAspectFit
                }
            }
        }

        AlbumView {
            id: _albumView

            SplitView.preferredWidth: root.width * 2 / 3

            Component.onCompleted: {
                if (!synoAlbum) {
                    synoAlbum = SynoPS.createAlbumForPath();
                }

                synoAlbum.refresh();
            }
        }
    }

    SynoSettings {
        id: _settings
        group: "baseScreen"
    }

    Component.onCompleted: {
        _mainView.restoreState(_settings.value("splitViewMain"));
        _sideView.restoreState(_settings.value("splitViewSide"));
    }

    Component.onDestruction: {
        _settings.setValue("splitViewMain", _mainView.saveState());
        _settings.setValue("splitViewSide", _sideView.saveState());
    }
}
