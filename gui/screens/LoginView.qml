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

import FotoStation 1.0
import widgets 1.0

Item {
    id: root

    implicitHeight: _groupBox.height
    implicitWidth: _groupBox.width

    SynoSettings {
        id: _settings
        group: "connection"
    }

    GroupBox {
        id: _groupBox

        anchors.centerIn: parent

        contentHeight: _layout.height
        contentWidth: _layout.width

        title: qsTr("Connect to PhotoStation")

        GridLayout {
            id: _layout

            anchors.centerIn: parent

            columns: 2

            Label {
                text: qsTr("Host name or IP:")
            }

            FSTextField {
                id: _host
                enabled: !SynoPS.conn.isConnecting
                text: _settings.value("hostname") || ""
            }

            Item {
                width: 1
                visible: _wrongHostNameLabel.visible
            }

            Label {
                id: _wrongHostNameLabel
                text: qsTr("Enter host name or IP")
                color: 'red'
                visible: _host.text.length === 0
            }

            Label {
                text: qsTr("PhotoStation path:")
            }

            FSTextField {
                id: _psPath
                enabled: !SynoPS.conn.isConnecting
                text: _settings.value("psPath") || "/photo"
            }

            Item {
                width: 1
                visible: _wrongPsPathLabel.visible
            }

            Label {
                id: _wrongPsPathLabel
                text: qsTr("Enter PhotoStation path")
                color: 'red'
                visible: _psPath.text.length === 0
            }

            Label {
                text: qsTr("User name:")
            }

            FSTextField {
                id: _username
                enabled: !SynoPS.conn.isConnecting
                text: _settings.value("username") || ""
            }

            Item {
                width: 1
                visible: _wrongUsernameLabel.visible
            }

            Label {
                id: _wrongUsernameLabel
                text: qsTr("Enter user name")
                color: 'red'
                visible: _username.text.length === 0
            }

            Label {
                text: qsTr("Password:")
            }

            FSTextField {
                id: _password
                enabled: !SynoPS.conn.isConnecting
                echoMode: TextInput.Password
                inputMethodHints: Qt.ImhHiddenText | Qt.ImhSensitiveData | Qt.ImhNoPredictiveText
            }

            Item {
                width: 1
                visible: _wrongPasswordLabel.visible
            }

            Label {
                id: _wrongPasswordLabel
                text: qsTr("Enter password")
                color: 'red'
                visible: _password.text.length === 0
            }

            Item {
                width: 1
            }

            CheckBox {
                id: _secureConnection
                enabled: !SynoPS.conn.isConnecting

                text: qsTr("Secure connection")
                checkState: Qt.Checked
            }

            Item {
                width: 1
                visible: _unsecureConnectionLabel.visible
            }

            Label {
                id: _unsecureConnectionLabel
                text: qsTr("Always use secure connection\nwhen accessing PhotoStation over Internet")
                color: 'red'
                visible: _secureConnection.checkState !== Qt.Checked
            }

            Item {
                width: 1
            }

            CheckBox {
                id: _rememberCredentials
                enabled: !SynoPS.conn.isConnecting

                text: qsTr("Remember credentials")
                checkState: Qt.Checked
            }

            Item {
                width: 1
                height: _connectButton.height * 0.5
                Layout.columnSpan: 2
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            }

            Button {
                id: _connectButton
                visible: !SynoPS.conn.isConnecting && SynoPS.conn.status === SynoConn.DISCONNECTED
                enabled: internal.isFormValid

                Layout.columnSpan: 2
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter

                text: qsTr("Connect")

                onClicked: {
                    var scheme = _secureConnection.checked ? "https://" : "http://";
                    var url = scheme + _host.text + "/" + _psPath.text;
                    SynoPS.conn.connectToSyno(url);
                }
            }

            Button {
                id: _disconnectButton
                visible: !_connectButton.visible

                Layout.columnSpan: 2
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter

                text: SynoPS.conn.isConnecting ? qsTr("Abort connection") : qsTr("Disconnect")

                onClicked: {
                    SynoPS.conn.disconnectFromSyno();
                }
            }
        }
    }

    Connections {
        target: SynoPS.conn
        onStatusChanged: {
            if (SynoPS.conn.status === SynoConn.API_LOADED) {
                if (_rememberCredentials.checked) {
                    internal.saveCredentials();
                }
                SynoPS.conn.authorize(_username.text, _password.text);
            }
        }
    }

    QtObject {
        id: internal

        function saveCredentials() {
            _settings.setValue("hostname", _host.text);
            _settings.setValue("psPath", _psPath.text);
            _settings.setValue("username", _username.text);
        }

        readonly property bool isFormValid: !_wrongHostNameLabel.visible &&
                                            !_wrongPsPathLabel.visible &&
                                            !_wrongUsernameLabel.visible &&
                                            !_wrongPasswordLabel.visible
    }
}
