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

import FotoStation 1.0
import FotoStation.native 1.0

Loader {
    id: root

    function ensureStyleApplied(appWindow, callback) {
        internal.callback = callback;

        if (internal.appStylerUrl) {
            setSource(internal.appStylerUrl, { "appWindow": appWindow });
        }

        internal.processStatusChange();
    }

    Connections {
        target: root
        function onStatusChanged() {
            internal.processStatusChange();
        }
    }

    Connections {
        target: root.item
        function onIsReadyChanged() {
            internal.processStatusChange();
        }
    }

    QtObject {
        id: internal

        property var callback: null

        readonly property url appStylerUrl: {
            if (Runtime.isPlatformWindows) {
                return Qt.resolvedUrl("windows/AppStylerWindows.qml");
            }

            return "";
        }

        function execCallback() {
            var cb = callback;
            callback = null;
            cb();
        }

        function processStatusChange() {
            if (!callback) {
                // nothing to do
                return;
            }

            if (root.status === Loader.Null) {
                console.info("No platform specific style available");
                execCallback();
                return;
            }

            if (root.status === Loader.Error) {
                console.info("Error on loading of platform specific style");
                execCallback();
                return;
            }

            if (root.status === Loader.Ready && root.item && root.item.isReady) {
                console.info("Platform specific style loaded");
                execCallback();
                return;
            }
        }
    }
}
