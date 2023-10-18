INCLUDEPATH += $$PWD/src/include
DEPENDPATH += $$PWD/src/include

win32-g++ {
    LIBS += $$PWD/libs/libyara-win-$${QT_ARCH}.a
}
win32-msvc* {
    LIBS += $$PWD/libs/yara-win-$${QT_ARCH}.lib
}
unix:!macx {
    LIBS += $$PWD/libs/libyara-unix-$${QT_ARCH}.a
}
unix:macx {
    LIBS += $$PWD/libs/libyara-macos-$${QT_ARCH}.a
}
