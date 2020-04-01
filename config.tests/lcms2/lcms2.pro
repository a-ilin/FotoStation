SOURCES = main.cpp
LIBS += -llcms2

win32: {
    DEFINES += CMS_DLL
    CONFIG += console
}

# handling for custom directory
LCMS2_ROOT = $$(LCMS2_ROOT)
!isEmpty(LCMS2_ROOT) {
    INCLUDEPATH += $${LCMS2_ROOT}/include
    win32: LIBS += -L$${LCMS2_ROOT}/bin
    unix:  LIBS += -L$${LCMS2_ROOT}/lib
}
