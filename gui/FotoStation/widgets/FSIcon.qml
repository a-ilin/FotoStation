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
import QtGraphicalEffects 1.0

import FotoStation.assets 1.0

Item {
    id: root

    property alias color: _effect.color
    property alias source: _srcImage.source

    implicitHeight: _srcImage.height
    implicitWidth: _srcImage.width

    Image {
        id: _srcImage
        visible: false
    }

    ColorOverlay {
        id: _effect
        anchors.centerIn: parent
        height: _srcImage.height
        width: _srcImage.width
        source: _srcImage
        color: Assets.colors.iconDefault
    }
}
