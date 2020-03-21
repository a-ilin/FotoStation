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
                enabled: !Facade.isConnecting
            }

            Item {
                width: 1
                visible: _wrongHostNameLabel.visible
            }

            Label {
                id: _wrongHostNameLabel
                text: !Facade.isConnecting && !internal.isHostNameValid ? qsTr("Enter host name or IP") : ""
                color: 'red'
            }

            Label {
                text: qsTr("PhotoStation path:")
            }

            FSTextField {
                id: _psPath
                // default path
                text: "/photo"
                enabled: !Facade.isConnecting
            }

            Item {
                width: 1
                visible: _wrongPsPathLabel.visible
            }

            Label {
                id: _wrongPsPathLabel
                text: !Facade.isConnecting && !internal.isPsPathValid ? qsTr("Enter PhotoStation path (default is '/photo')") : ""
                color: 'red'
            }

            Label {
                text: qsTr("User name:")
            }

            FSTextField {
                id: _username
                placeholderText: SynoPS.conn.auth.isCookieAvailable ? "<Saved>" : ""
                enabled: !Facade.isConnecting
            }

            Item {
                width: 1
                visible: _wrongUsernameLabel.visible
            }

            Label {
                id: _wrongUsernameLabel
                text: !Facade.isConnecting && !internal.isUsernameValid ? qsTr("Enter user name") : ""
                color: 'red'
            }

            Label {
                text: qsTr("Password:")
            }

            FSTextField {
                id: _password
                enabled: !Facade.isConnecting
                echoMode: TextInput.Password
                inputMethodHints: Qt.ImhHiddenText | Qt.ImhSensitiveData | Qt.ImhNoPredictiveText
                placeholderText: SynoPS.conn.auth.isCookieAvailable ? "<Saved>" : ""
            }

            Item {
                width: 1
                visible: _wrongPasswordLabel.visible
            }

            Label {
                id: _wrongPasswordLabel
                text: !Facade.isConnecting && !internal.isPasswordValid ? qsTr("Enter password") : ""
                color: 'red'
            }

            Item {
                width: 1
            }

            CheckBox {
                id: _secureConnection
                enabled: !Facade.isConnecting && SynoPS.conn.sslConfig.isSslAvailable

                text: qsTr("Secure connection")
                checkState: SynoPS.conn.sslConfig.isSslAvailable ? Qt.Checked : Qt.Unchecked
            }

            Item {
                width: 1
                visible: _unsecureConnectionLabel.visible
            }

            Label {
                id: _unsecureConnectionLabel
                text: SynoPS.conn.sslConfig.isSslAvailable
                      ? qsTr("Always use secure connection\nwhen accessing PhotoStation over Internet")
                      : qsTr("SSL is not available. Always use secure connection\nwhen accessing PhotoStation over Internet")
                color: 'red'
                visible: !Facade.isConnecting && _secureConnection.checkState !== Qt.Checked
            }

            Item {
                width: 1
            }

            CheckBox {
                id: _rememberCredentials
                enabled: !Facade.isConnecting

                text: qsTr("Remember credentials")
            }

            Item {
                width: 1
                height: _connectButton.height * 0.5
                Layout.columnSpan: 2
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            }

            Button {
                id: _connectButton
                visible: !Facade.isConnecting && SynoPS.conn.status === SynoConn.NONE
                enabled: internal.isFormValid

                Layout.columnSpan: 2
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter

                text: qsTr("Connect")

                onClicked: {
                    internal.connectToSyno();
                }
            }

            Button {
                id: _disconnectButton
                visible: !_connectButton.visible

                Layout.columnSpan: 2
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter

                text: Facade.isConnecting ? qsTr("Abort connection") : qsTr("Disconnect")

                onClicked: {
                    internal.abortConnection();
                }
            }
        }
    }

    Connections {
        target: SynoPS.conn
        onStatusChanged: {
            if (SynoPS.conn.status === SynoConn.API_LOADED) {
                SynoPS.conn.auth.authorizeWithCookie();
            }
        }
    }

    Connections {
        target: SynoPS.conn.auth
        onStatusChanged: {
            if (SynoPS.conn.auth.status === SynoAuth.WAIT_USER) {
                if (_username.text.length > 0 && _password.text.length > 0) {
                    SynoPS.conn.auth.authorizeWithCredentials(_username.text, _password.text);
                } else {
                    internal.abortConnection();
                }
            }
        }
    }

    Connections {
        target: SynoPS.conn.sslConfig
        onConfirmedSslExceptions: {
            internal.connectToSyno(true);
        }
    }

    QtObject {
        id: internal

        readonly property bool isHostNameValid: _host.text.length > 0
        readonly property bool isPsPathValid: _psPath.text.length > 0
        readonly property bool isUsernameValid: _username.text.length > 0
        readonly property bool isPasswordValid: _password.text.length > 0

        readonly property bool isFormValid: isHostNameValid &&
                                            isPsPathValid &&
                                            isUsernameValid &&
                                            isPasswordValid

        function connectToSyno(autologin) {
            if (!autologin) {
                // token and cookies should be cleared
                SynoPS.conn.auth.closeSession(true);
            }

            let synoUrl = urlFromForm();
            if (synoUrl) {
                SynoPS.conn.auth.keepCookies = _rememberCredentials.checked;
                if (_rememberCredentials.checked) {
                    _settings.saveSecure("synoUrl", synoUrl);
                } else {
                    _settings.deleteSecure("synoUrl");
                }

                Facade.lastUsedSynoUrl = synoUrl;
                SynoPS.conn.connectToSyno(synoUrl);
            }
        }

        function abortConnection() {
            Facade.autoLoginAllowed = false;
            SynoPS.conn.disconnectFromSyno();
            SynoPS.conn.auth.closeSession(true);
        }

        function readStoredSynoUrl() {
            _settings.readSecure("synoUrl", function(key, secureValue) {
                processSynoUrl(secureValue);
            }, function() {
                processSynoUrl();
            });
        }

        function processSynoUrl(synoUrl) {
            if (!!synoUrl && synoUrl.length > 0) {
                urlToForm(synoUrl);
            }

            if (Facade.autoLoginAllowed) {
                SynoPS.conn.auth.reloadCookies(function() {
                    let isCookieAvailable = SynoPS.conn.auth.isCookieAvailableForUrl(urlFromForm());
                    if (Facade.autoLoginAllowed && isCookieAvailable) {
                        connectToSyno(true);
                    }
                    Facade.autoLoginAllowed = false;
                });
            }
        }

        /*! This function parses given url into form fields */
        function urlToForm(synoUrl) {
            let urlObj = SynoPS.urlToMap(synoUrl);
            console.warn("URL: ", JSON.stringify(urlObj))

            _secureConnection.checked = urlObj.protocol === "https";

            if (urlObj.port !== -1) {
                _host.text = urlObj.hostname + ":" + Number(urlObj.port);
            } else {
                _host.text = urlObj.hostname;
            }

            _psPath.text = urlObj.pathname;

            _rememberCredentials.checked = true;
        }

        /*! This function returns url from form, or undefined if form is incomplete */
        function urlFromForm() {
            if (_host.text.length > 0 && _psPath.text.length > 0) {
                var urlObj = {};
                urlObj["protocol"] = _secureConnection.checked ? "https" : "http";
                urlObj["pathname"] = _psPath.text;

                let portIdx = _host.text.indexOf(":");
                if (portIdx !== -1) {
                    urlObj["hostname"] = _host.text.slice(0, portIdx);
                    urlObj["port"] = _host.text.slice(portIdx + 1);
                } else {
                    urlObj["hostname"] = _host.text;
                }

                return SynoPS.urlFromMap(urlObj);
            }
        }

        Component.onCompleted: {
            if (Facade.autoLoginAllowed) {
                Facade.autoLoginAllowed = _settings.value("autologin", true);
            }

            if (Facade.lastUsedSynoUrl && Facade.lastUsedSynoUrl.length > 0) {
                processSynoUrl(Facade.lastUsedSynoUrl);
            } else {
                internal.readStoredSynoUrl();
            }
        }
    }
}
