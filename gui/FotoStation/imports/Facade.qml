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

pragma Singleton

import QtQuick 2.13

import FotoStation 1.0

QtObject {
    id: root

    /*! This property holds status when automatic login is allowed */
    property bool autoLoginAllowed: true

    /*! This property holds last used connection URL */
    property url lastUsedSynoUrl

    /*! This property holds OverlayManager instance */
    property var overlayManager: null

    /*! This property holds status of network connection */
    readonly property bool isConnecting: ([SynoConn.ATTEMPT_API].indexOf(SynoPS.conn.status) !== -1)
                                         || ([SynoAuth.ATTEMPT_COOKIE,
                                              SynoAuth.ATTEMPT_USER].indexOf(SynoPS.conn.auth.status) !== -1)
                                         || autoLoginAllowed

    function coverUrl(albumId) {
        if (albumId && albumId !== "") {
            return "image://syno/" + SynoPS.toString(albumId);
        }
        return "";
    }

}
