pragma Singleton

import QtQuick 2.15

QtObject {

    readonly property QtObject animated: QtObject {
        readonly property url roller_16: Qt.resolvedUrl("animated/roller_16.gif")
        readonly property url roller_32: Qt.resolvedUrl("animated/roller_32.gif")
        readonly property url roller_64: Qt.resolvedUrl("animated/roller_64.gif")
    }

    readonly property QtObject icons: QtObject {
        readonly property url broken_document_12: Qt.resolvedUrl("icons/broken_document_12.png")
        readonly property url broken_document_16: Qt.resolvedUrl("icons/broken_document_16.png")
        readonly property url broken_document_32: Qt.resolvedUrl("icons/broken_document_32.png")
        readonly property url broken_document_48: Qt.resolvedUrl("icons/broken_document_48.png")
        readonly property url broken_document_64: Qt.resolvedUrl("icons/broken_document_64.png")
        readonly property url broken_document_128: Qt.resolvedUrl("icons/broken_document_128.png")
        readonly property url broken_document_256: Qt.resolvedUrl("icons/broken_document_256.png")
        readonly property url broken_document_512: Qt.resolvedUrl("icons/broken_document_512.png")
    }

    readonly property QtObject colors: QtObject {
        readonly property color iconDefault: Assets.appPalette.alternateBase
    }

    /*! This property holds reference to application window system palette */
    property var appPalette: null
}
