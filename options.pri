#
# GNU General Public License (GPL)
# Copyright (c) 2020 by Aleksei Ilin
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

load(configure)

# KeyChain
include($$_PRO_FILE_PWD_/3rdParty/qtkeychain/qt5keychain.pri): {
    DEFINES += USE_KEYCHAIN
    !build_pass:message("KeyChain support: on")
} else {
    !build_pass:message("KeyChain support: off")
}

# Little CMS2
qtCompileTest(lcms2)
contains(CONFIG, config_lcms2): {
    DEFINES += USE_LCMS2
    LIBS += -llcms2

    # handling for custom directory
    LCMS2_ROOT = $$(LCMS2_ROOT)
    !isEmpty(LCMS2_ROOT) {
        INCLUDEPATH += $${LCMS2_ROOT}/include
        win32: LIBS += -L$${LCMS2_ROOT}/bin
        unix:  LIBS += -L$${LCMS2_ROOT}/lib
    }

    # use DLL
    win32: DEFINES += CMS_DLL

    !build_pass:message("LittleCMS2 support: on")
} else {
    !build_pass:message("LittleCMS2 support: off")
}
