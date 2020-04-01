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

TARGET = FotoStation

QT += concurrent core core-private network quick quick-private quickcontrols2
CONFIG += c++17 no_private_qt_headers_warning qmltypes

# Check required Qt version
REQUIRED_QT_VERSION=5.15.0
!versionAtLeast(QT_VERSION, $${REQUIRED_QT_VERSION}): error("Use at least Qt version $${REQUIRED_QT_VERSION}")

include(options.pri)

# QML import module name to be registered
QML_IMPORT_NAME = FotoStation.native
QML_IMPORT_MAJOR_VERSION = 1

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH = gui

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH = gui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

CONFIG(release, debug|release): {
    DEFINES += GUI_PREFIX_PATH=\"\\\"qrc:\\\"\"
    RESOURCES += gui/resources.qrc
}

CONFIG(debug, debug|release): {
    DEFINES += GUI_PREFIX_PATH=\"\\\"file:///"$$_PRO_FILE_PWD_"/gui\\\"\"
}

include(src/src.pri)
