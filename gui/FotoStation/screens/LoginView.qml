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
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.13

import FotoStation 1.0
import FotoStation.widgets 1.0

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
                text: internal.storedHostName()
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
                text: internal.storedPsPath()
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
                text: internal.storedUsername()
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

                Component.onCompleted: {
                    internal.readStoredPassword();
                }
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
            _settings.saveSecure(secureKey(_host.text, _psPath.text, _username.text), _password.text);
        }

        function storedHostName() {
            return _settings.value("hostname") || "";
        }

        function storedPsPath() {
            return _settings.value("psPath") || "/photo";
        }

        function storedUsername() {
            return _settings.value("username") || "";
        }

        function readStoredPassword() {
            _settings.readSecure(secureKey(storedHostName(), storedPsPath(), storedUsername()), function(key, password) {
                // check that form is consistent with initial request, and no password is specified
                if (_password.text.length === 0
                        && key === secureKey(_host.text, _psPath.text, _username.text)) {
                    _password.text = password;
                }
            });
        }

        function secureKey(hostname, psPath, username) {
            return username + "@" + hostname + "^" + psPath;
        }

        readonly property bool isFormValid: !_wrongHostNameLabel.visible &&
                                            !_wrongPsPathLabel.visible &&
                                            !_wrongUsernameLabel.visible &&
                                            !_wrongPasswordLabel.visible
    }
}
