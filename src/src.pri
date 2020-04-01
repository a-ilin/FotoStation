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

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Refer to the documentation for the
# deprecated API to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/cache.h \
    $$PWD/colorhandler.h \
    $$PWD/colorhandler_p.h \
    $$PWD/qmlimageadvanced.h \
    $$PWD/qmlobjectwrapper.h \
    $$PWD/synoalbum.h \
    $$PWD/synoalbumcache.h \
    $$PWD/synoalbumdata.h \
    $$PWD/synoalbumfactory.h \
    $$PWD/synoauth.h \
    $$PWD/synoconn.h \
    $$PWD/synoerror.h \
    $$PWD/synoimagecache.h \
    $$PWD/synoimageprovider.h \
    $$PWD/synoimageprovider_p.h \
    $$PWD/synops.h \
    $$PWD/synoreplyjson.h \
    $$PWD/synorequest.h \
    $$PWD/synosettings.h \
    $$PWD/synosize.h \
    $$PWD/synosslconfig.h \
    $$PWD/synotraits.h

SOURCES += \
    $$PWD/colorhandler.cpp \
    $$PWD/main.cpp \
    $$PWD/qmlimageadvanced.cpp \
    $$PWD/qmlobjectwrapper.cpp \
    $$PWD/synoalbum.cpp \
    $$PWD/synoalbumcache.cpp \
    $$PWD/synoalbumdata.cpp \
    $$PWD/synoalbumfactory.cpp \
    $$PWD/synoauth.cpp \
    $$PWD/synoconn.cpp \
    $$PWD/synoerror.cpp \
    $$PWD/synoimagecache.cpp \
    $$PWD/synoimageprovider.cpp \
    $$PWD/synops.cpp \
    $$PWD/synoreplyjson.cpp \
    $$PWD/synorequest.cpp \
    $$PWD/synosettings.cpp \
    $$PWD/synosize.cpp \
    $$PWD/synosslconfig.cpp

# MSVC section
msvc: {
    # Use STD value for __cplusplus on MSVC
    QMAKE_CXXFLAGS += /Zc:__cplusplus
    # Generate ASM output
    QMAKE_CXXFLAGS += /FAs
}

# Windows
win32: {
    SOURCES += \
        $$PWD/colorhandler_win.cpp

    LIBS += Gdi32.lib User32.lib
}
else: {
    SOURCES += \
        $$PWD/colorhandler_dummy.cpp
}

config_lcms2: {
    HEADERS += \
        $$PWD/colorhandlercms.h

    SOURCES += \
        $$PWD/colorhandlercms.cpp
}
