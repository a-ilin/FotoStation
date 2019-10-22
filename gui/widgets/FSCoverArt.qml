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
import QtGraphicalEffects 1.0

import assets 1.0

Item {
    id: root

    property alias image: _coverImage

    readonly property bool isError: _coverImage.status === Image.Error
    readonly property bool isLoaded: _coverImage.status === Image.Ready

    implicitHeight: _coverImage.height
    implicitWidth: _coverImage.width

    Image {
        id: _coverImage
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        clip: true
        source: Facade.coverUrlForThumb(model.synoData.id)
    }

    FSIcon {
        id: _errorIcon
        anchors.centerIn: parent
        source: Assets.icons.broken_document_64
        visible: root.isError
    }

    AnimatedImage {
        id: _loadingIndicator
        source: _coverImage.status === Image.Loading ? Assets.animated.roller_64 : ""
        visible: false
    }

    ColorOverlay {
        anchors.centerIn: parent
        width: _loadingIndicator.width
        height: _loadingIndicator.height
        source: _loadingIndicator
        color: Assets.colors.iconDefault
    }
}
