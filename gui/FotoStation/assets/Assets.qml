pragma Singleton

import QtQuick 2.15

QtObject {

    readonly property QtObject animated: QtObject {
        readonly property QtObject roller: QtObject {
            readonly property url asset_16: Qt.resolvedUrl("animated/roller_16.gif")
            readonly property url asset_32: Qt.resolvedUrl("animated/roller_32.gif")
            readonly property url asset_64: Qt.resolvedUrl("animated/roller_64.gif")
        }
    }

    readonly property QtObject icons: QtObject {
        readonly property QtObject broken_document: QtObject {
            readonly property url asset_12: Qt.resolvedUrl("icons/broken_document_12.png")
            readonly property url asset_16: Qt.resolvedUrl("icons/broken_document_16.png")
            readonly property url asset_32: Qt.resolvedUrl("icons/broken_document_32.png")
            readonly property url asset_48: Qt.resolvedUrl("icons/broken_document_48.png")
            readonly property url asset_64: Qt.resolvedUrl("icons/broken_document_64.png")
            readonly property url asset_128: Qt.resolvedUrl("icons/broken_document_128.png")
            readonly property url asset_256: Qt.resolvedUrl("icons/broken_document_256.png")
            readonly property url asset_512: Qt.resolvedUrl("icons/broken_document_512.png")
        }
    }

    readonly property QtObject colors: QtObject {
        readonly property color iconDefault: Assets.appPalette.alternateBase
    }

    /*! This property holds reference to application window system palette */
    property var appPalette: null

    /*! This method returns asset for specified size */
    function assetForSize(assetObj, sz) {
        var bestAsset;

        if (assetObj.asset_512) {
            bestAsset = assetObj.asset_512;

            if (sz > 512) {
                return bestAsset;
            }
        }

        if (assetObj.asset_256) {
            bestAsset = assetObj.asset_256;

            if (sz > 256) {
                return bestAsset;
            }
        }

        if (assetObj.asset_128) {
            bestAsset = assetObj.asset_128;

            if (sz > 128) {
                return bestAsset;
            }
        }

        if (assetObj.asset_64) {
            bestAsset = assetObj.asset_64;

            if (sz > 64) {
                return bestAsset;
            }
        }

        if (assetObj.asset_32) {
            bestAsset = assetObj.asset_32;

            if (sz > 32) {
                return bestAsset;
            }
        }

        if (assetObj.asset_16) {
            bestAsset = assetObj.asset_16;

            if (sz > 16) {
                return bestAsset;
            }
        }

        if (assetObj.asset_12) {
            bestAsset = assetObj.asset_12;

            if (sz > 12) {
                return bestAsset;
            }
        }

        return bestAsset;
    }
}
