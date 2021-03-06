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

FocusScope {
    id: root

    Loader {
        id: _sslExceptionsPopupLoader

        function resetActivation() {
            if (SynoPS.conn.sslConfig.isSslError) {
                active = true;
            }
        }

        active: false
        sourceComponent: SslExceptionsPopup {
            onClosed: {
                _sslExceptionsPopupLoader.active = false;
            }
        }

        onLoaded: {
            item.open();
        }

        Connections {
            target: SynoPS.conn.sslConfig
            function onIsSslErrorChanged() {
                _sslExceptionsPopupLoader.resetActivation();
            }
        }

        Component.onCompleted: {
            resetActivation();
        }
    }

    Component.onCompleted: {
        Facade.overlayManager = this;
    }
}
