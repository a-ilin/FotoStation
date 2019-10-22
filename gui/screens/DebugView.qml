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
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.13
import QtQuick.Window 2.13

import FotoStation 1.0
import assets 1.0
import widgets 1.0

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
                    height: 16
                    width: 16
                    source: internal.isAwaitingReply ? Assets.roller_16 : ""
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
