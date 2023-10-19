INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD/3rdparty/yara/include
DEPENDPATH += $$PWD/3rdparty/yara/include

HEADERS += \
    $$PWD/xyara.h

SOURCES += \
    $$PWD/xyara.cpp

!contains(XCONFIG, yara) {
    XCONFIG += yara
    include($$PWD/3rdparty/yara/yara.pri)
}

!contains(XCONFIG, xbinary) {
    XCONFIG += xbinary
    include($$PWD/../Formats/xbinary.pri)
}

win32-msvc* {
    LIBS += Advapi32.lib
}

DISTFILES += \
    $$PWD/xyara.cmake
