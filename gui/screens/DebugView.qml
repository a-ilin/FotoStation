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

import assets 1.0
import widgets 1.0

Window {
    id: root

    /*! This property holds instance of SynoPS */
    property var synoPS: null

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
                    model: synoPS.conn.apiList
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

        property bool isAwaitingReply: false

        function sendRequest() {
            var formData = _formDataText.text.split('\n');
            root.synoPS.conn.sendRequest(_apiSelector.currentText, formData,
                                         processReply, processError);
            isAwaitingReply = true;
        }

        function processReply(jsonText) {
            _replyText.text = jsonText;
            isAwaitingReply = false;
        }

        function processError() {
            _replyText.text = qsTr("<Error occured>");
            isAwaitingReply = false;
        }
    }
}
