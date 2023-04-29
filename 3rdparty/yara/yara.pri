INCLUDEPATH += $$PWD/src/include
DEPENDPATH += $$PWD/src/include

win32-g++ {
    contains(QT_ARCH, i386) {
        LIBS += $$PWD/libs/win32-g++/libyara.a
    } else {
        LIBS += $$PWD/libs/win64-g++/libyara.a
    }
}
win32-msvc* {
    contains(QMAKE_TARGET.arch, x86_64) {
        LIBS += $$PWD/libs/win64-msvc/yara.lib
    } else {
        LIBS += $$PWD/libs/win32-msvc/yara.lib
    }

    LIBS += -ladvapi32
}

unix:!macx {
    BITSIZE = $$system(getconf LONG_BIT)
    if (contains(BITSIZE, 64)) {
        LIBS +=  $$PWD/libs/lin64/libyara.a
    }
    if (contains(BITSIZE, 32)) {
        LIBS +=  $$PWD/libs/lin32/libyara.a
    }
}
unix:macx {
    LIBS +=  $$PWD/libs/mac/libyara.a
}

