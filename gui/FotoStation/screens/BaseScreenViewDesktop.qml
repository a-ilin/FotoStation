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
import QtQuick.Controls 1.4 as Controls_1
import QtQuick.Controls 2.15

import FotoStation 1.0
import FotoStation.assets 1.0
import FotoStation.native 1.0
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

            Rectangle {
                color: Assets.appPalette.shadow

                SplitView.minimumHeight: Math.min(root.height / 2, 200)

                Controls_1.TreeView {
                    anchors.fill: parent
                    anchors.margins: 1
                }
            }

            Rectangle {
                color: Assets.appPalette.shadow

                SplitView.preferredHeight: root.height / 2

                FSCoverArt {
                    anchors.fill: parent
                    anchors.margins: 1
                    sourceSizeHeight: height
                    sourceSizeWidth: width
                    source: Facade.coverThumbUrl(_albumView.selectedImageId)
                    fillMode: Image.PreserveAspectFit
                }
            }
        }

        Rectangle {
            color: Assets.appPalette.shadow

            SplitView.preferredWidth: root.width * 2 / 3

            Rectangle {
                color: Assets.appPalette.window

                anchors.fill: parent
                anchors.margins: 1
            }

            AlbumView {
                id: _albumView

                anchors.fill: parent
                anchors.margins: 1

                Component.onCompleted: {
                    setAlbumWrapper(SynoAlbumFactory.createAlbumForPath());
                    forceActiveFocus();
                }
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
