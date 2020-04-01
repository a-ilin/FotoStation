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
import QtQuick.Controls 2.5

import FotoStation 1.0
import FotoStation.native 1.0

Dialog {
    id: root

    x: (Facade.overlayManager.width - width) / 2
    y: (Facade.overlayManager.height - height) / 2
    height: Facade.overlayManager.height * 0.9
    width: Math.max(Facade.overlayManager.width * 0.6, _tm.width)

    modal: true
    clip: true
    focus: true
    closePolicy: Popup.NoAutoClose

    title: qsTr("SSL error occured during connection to PhotoStation")

    footer: DialogButtonBox {
        standardButtons: DialogButtonBox.Ignore | DialogButtonBox.Abort

        onAccepted: {
            SynoPS.conn.sslConfig.confirmSslExceptions();
            if (_saveExceptionsCheckBox.checked) {
                SynoPS.conn.sslConfig.saveExceptionsToStorage();
            }
        }

        onRejected: {
            SynoPS.conn.disconnectFromSyno();
        }

        Component.onCompleted: {
            var ignoreButton = standardButton(DialogButtonBox.Ignore);
            ignoreButton.enabled = Qt.binding(function() { return _ignoreCheckBox.checked; });

            var abortButton = standardButton(DialogButtonBox.Abort);
            abortButton.forceActiveFocus();
        }
    }

    TextMetrics {
        id: _tm
        text: "M".repeat(70)
    }

    Item {
        id: _container

        width: parent.width
        height: parent.height

        Label {
            id: _warningText
            height: implicitHeight
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 22
            color: "red"
            text: qsTr("Your security could be compromised")
        }

        Rectangle {
            anchors.centerIn: _listView
            height: _listView.height + _listView.anchors.margins * 2
            width: _listView.width + _listView.anchors.margins * 2
            color: "red"
        }

        Rectangle {
            anchors.fill: _listView
            color: "yellow"
        }

        ListView {
            id: _listView

            anchors.margins: 1
            anchors.top: _warningText.bottom
            anchors.topMargin: _warningText.height / 2
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: _ignoreCheckBox.top

            boundsBehavior: Flickable.StopAtBounds
            clip: true

            ScrollBar.vertical: ScrollBar {
                active: true
            }

            model: SynoPS.conn.sslConfig.errorsModel
            spacing: 5

            delegate: Item {
                id: _delegate

                function certificateText() {
                    return qsTr("\
Certificate:\n\
Digest MD5: %1\n\
Digest SHA-1: %2\n\
Issuer: %3\n\
Subject: %4\n\
Effective date: %5\n\
Expiry date: %6")
                    .arg(model.certificateDigestMD5)
                    .arg(model.certificateDigestSha1)
                    .arg(model.certificateIssuer)
                    .arg(model.certificateSubject)
                    .arg(model.certificateEffectiveDate)
                    .arg(model.certificateExpiryDate);
                }

                width: _listView.width
                height: _column.height

                Column {
                    id: _column

                    readonly property int margins: 2

                    anchors.centerIn: parent

                    height: _certificateText.height + _errorText.height + spacing
                    width: parent.width - margins * 2

                    Text {
                        id: _certificateText
                        text: _delegate.certificateText()
                        height: implicitHeight
                        width: parent.width
                        wrapMode: Text.Wrap
                    }
                    Text {
                        id: _errorText
                        text: model.errorText
                        height: implicitHeight
                        width: parent.width
                        wrapMode: Text.Wrap
                        color: "red"
                        font.bold: true
                    }
                }
            }
        }

        CheckBox {
            id: _ignoreCheckBox

            anchors.bottom: _saveExceptionsCheckBox.top
            width: parent.width

            text: qsTr("I verified digest of all certificates, and I agree to ignore the errors")
            onCheckedChanged: {
                if (!checked) {
                    _saveExceptionsCheckBox.checked = false;
                }
            }
        }

        CheckBox {
            id: _saveExceptionsCheckBox

            anchors.bottom: parent.bottom
            width: parent.width

            enabled: _ignoreCheckBox.checked
            text: qsTr("Ignore errors for those certificates in future")
        }
    }
}
