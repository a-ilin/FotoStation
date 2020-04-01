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
import QtQuick.Controls 2.15
import QtQuick.Controls.Universal 2.12

import FotoStation 1.0
import FotoStation.native 1.0

Item {
    id: root

    property var appWindow: null

    readonly property bool isReady: internal.applied

    onAppWindowChanged: {
        internal.applyStyle();
    }

    SynoSettings {
        id: _settings
        group: "style"
    }

    QtObject {
        id: internal

        property bool applied: false

        function applyStyle() {
            if (root.appWindow && !applied) {
                root.appWindow.Universal.theme = _settings.value("universalTheme", "System");
                root.appWindow.Universal.accent = _settings.value("universalAccent", "Cobalt");

                // accent property is being converted to color on assignment
                root.appWindow.palette.highlight = root.appWindow.Universal.accent;

                applied = true;
            }
        }
    }

    Component.onCompleted: {
        internal.applyStyle();
    }
}
