QT += core core-private network quick widgets
CONFIG += c++17 no_private_qt_headers_warning

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Refer to the documentation for the
# deprecated API to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH = gui

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH = gui

# MSVC section
msvc: {
  # Use STD value for __cplusplus on MSVC
  QMAKE_CXXFLAGS += /Zc:__cplusplus
  # Generate ASM output
  QMAKE_CXXFLAGS += /FAs
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    src/cache.h \
    src/qmlobjectwrapper.h \
    src/synoalbum.h \
    src/synoalbumcache.h \
    src/synoalbumdata.h \
    src/synoalbumfactory.h \
    src/synoauth.h \
    src/synoconn.h \
    src/synoerror.h \
    src/synoimagecache.h \
    src/synoimageprovider.h \
    src/synoimageprovider_p.h \
    src/synops.h \
    src/synoreplyjson.h \
    src/synorequest.h \
    src/synosettings.h \
    src/synosslconfig.h \
    src/synotraits.h

SOURCES += \
    src/main.cpp \
    src/qmlobjectwrapper.cpp \
    src/synoalbum.cpp \
    src/synoalbumcache.cpp \
    src/synoalbumdata.cpp \
    src/synoalbumfactory.cpp \
    src/synoauth.cpp \
    src/synoconn.cpp \
    src/synoerror.cpp \
    src/synoimagecache.cpp \
    src/synoimageprovider.cpp \
    src/synops.cpp \
    src/synoreplyjson.cpp \
    src/synorequest.cpp \
    src/synosettings.cpp \
    src/synosslconfig.cpp

CONFIG(release, debug|release): {
    DEFINES += GUI_PREFIX_PATH=\"\\\"qrc:\\\"\"
    RESOURCES += gui/resources.qrc
}

CONFIG(debug, debug|release): {
    DEFINES += GUI_PREFIX_PATH=\"\\\"file:///"$$_PRO_FILE_PWD_"/gui\\\"\"
}

# Enable keychain by default
DEFINES += USE_KEYCHAIN
contains(DEFINES, USE_KEYCHAIN) {
    !include($$_PRO_FILE_PWD_/3rdParty/qtkeychain/qt5keychain.pri) {
        error("Submodule qtkeychain not found. Please run command 'git submodule update --init --recursive' to initialize submodules")
    }
}
