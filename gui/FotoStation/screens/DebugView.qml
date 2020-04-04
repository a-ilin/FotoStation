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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

import FotoStation 1.0
import FotoStation.assets 1.0
import FotoStation.native 1.0
import FotoStation.widgets 1.0

Window {
    id: root

    width: 640
    height: 480

    Pane {
        anchors.fill: parent

        ColumnLayout {
            anchors.fill: parent
            spacing: 10

            RowLayout {
                width: parent.width
                spacing: 10

                Label {
                    text: qsTr("API:")
                    height: parent.height
                    verticalAlignment: Text.AlignVCenter
                }

                ComboBox {
                    id: _apiSelector
                    model: SynoPS.conn.apiList
                    Layout.fillWidth: true
                }
            }

            RowLayout {
                Layout.fillHeight: true

                GroupBox {
                    title: qsTr("Form data")

                    Layout.fillHeight: true
                    Layout.preferredWidth: 240

                    FSFlickable {
                        id: _formDataFlick
                        anchors.fill: parent

                        flickableContent: TextEdit {
                            id: _formDataText
                            focus: true
                            text: "version=1\n"
                            wrapMode: TextEdit.Wrap
                            onCursorRectangleChanged: _formDataFlick.ensureVisible(cursorRectangle)
                        }
                    }
                }

                GroupBox {
                    title: qsTr("Reply")

                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    FSFlickable {
                        id: _replyFlick
                        anchors.fill: parent

                        flickableContent: TextEdit {
                            id: _replyText
                            wrapMode: TextEdit.Wrap
                            onCursorRectangleChanged: _replyFlick.ensureVisible(cursorRectangle)
                        }
                    }
                }
            }

            RowLayout {
                spacing: 10

                Button {
                    enabled: !internal.isAwaitingReply
                    text: qsTr("Send request")
                    onClicked: {
                        internal.sendRequest();
                    }
                }

                AnimatedImage {
                    id: _requestProgressImage
                    asynchronous: true
                    height: 16
                    width: 16
                    source: internal.isAwaitingReply ? Assets.assetForSize(Assets.animated.roller, 16) : ""
                }
            }
        }
    }

    QtObject {
        id: internal

        property var request: null
        readonly property bool isAwaitingReply: request !== null

        function sendRequest() {
            var formData = _formDataText.text.split('\n');
            internal.request = SynoPS.conn.createRequest(_apiSelector.currentText, formData);
            internal.request.send(function() {
                if (!internal.request.errorString) {
                    if (internal.request.contentType === SynoRequest.TEXT) {
                        var replyJSON = SynoReplyJSONFactory.create(internal.request);
                        if (!replyJSON.errorString) {
                            _replyText.text = replyJSON.text;
                        } else {
                            _replyText.text = qsTr("Failed. ") + replyJSON.errorString;
                        }
                    } else {
                        _replyText.text = qsTr("Content type: ") + Number(internal.request.contentType);
                    }
                } else {
                    _replyText.text = qsTr("Failed. ") + internal.request.errorString;
                }

                internal.request = null;
            });
        }
    }
}
