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

import QtQuick 2.15
import QtGraphicalEffects 1.0

import FotoStation.assets 1.0
import FotoStation.native 1.0

Item {
    id: root

    property alias image: _coverImage

    readonly property bool isError: _coverImage.status === Image.Error
    readonly property bool isLoaded: _coverImage.status === Image.Ready

    ImageAdvanced {
        id: _coverImage
        anchors.fill: parent
        fillMode: Image.PreserveAspectCrop
        clip: true
        smooth: true
        asynchronous: true
        windowColorSpace: colorHandler.colorSpace
    }

    FSIcon {
        id: _errorIcon
        anchors.centerIn: parent
        source: Assets.icons.broken_document_32
        visible: root.isError
    }

    AnimatedImage {
        id: _loadingIndicator
        asynchronous: true
        source: !root.isError && !root.isLoaded ? Assets.animated.roller_32 : ""
        visible: false
    }

    ColorOverlay {
        anchors.centerIn: parent
        width: _loadingIndicator.width
        height: _loadingIndicator.height
        source: _loadingIndicator
        color: Assets.colors.iconDefault
        visible: _loadingIndicator.source !== ""
    }
}
