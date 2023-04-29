QT       -= core gui

TARGET = yara
TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++11

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD/src
DEPENDPATH += $$PWD/src
INCLUDEPATH += $$PWD/src/include
DEPENDPATH += $$PWD/src/include

include(../../build.pri)

CONFIG(debug, debug|release) {
    TARGET = yarad
} else {
    TARGET = yara
}

#DEFINES += "OPENSSL_NO_DEPRECATED_3_0"
#DEFINES += "HAVE_LIBCRYPTO"

#DEFINES += "CUCKOO_MODULE"
#DEFINES += "HASH_MODULE"
DEFINES += "DOTNET_MODULE"
DEFINES += "YR_BUILDING_STATIC_LIB"
DEFINES += "BUCKETS_256"

win32 {
    DEFINES += "USE_WINDOWS_PROC"
}
linux {
    DEFINES += "USE_LINUX_PROC"
}
macx {
    DEFINES += "USE_MACH_PROC"
}


TARGETLIB_PATH = $$PWD

win32-g++ {
    contains(QT_ARCH, i386) {
        DESTDIR=$${TARGETLIB_PATH}/libs/win32-g++
    } else {
        DESTDIR=$${TARGETLIB_PATH}/libs/win64-g++
    }
}
win32-msvc* {
    contains(QMAKE_TARGET.arch, x86_64) {
        DESTDIR=$${TARGETLIB_PATH}/libs/win64-msvc
    } else {
        DESTDIR=$${TARGETLIB_PATH}/libs/win32-msvc
    }
}
unix:!macx {
    BITSIZE = $$system(getconf LONG_BIT)
    if (contains(BITSIZE, 64)) {
        DESTDIR=$${TARGETLIB_PATH}/libs/lin64
    }
    if (contains(BITSIZE, 32)) {
        DESTDIR=$${TARGETLIB_PATH}/libs/lin32
    }
}
unix:macx {
    DESTDIR=$${TARGETLIB_PATH}/libs/mac
}
# hash -> _hash
SOURCES += \
    src/ahocorasick.c \
    src/arena.c \
    src/atoms.c \
    src/base64.c \
    src/bitmask.c \
    src/compiler.c \
    src/exec.c \
    src/exefiles.c \
    src/filemap.c \
    src/grammar.c \
    src/_hash.c \
    src/hex_grammar.c \
    src/hex_lexer.c \
    src/lexer.c \
    src/libyara.c \
    src/mem.c \
    src/modules.c \
    src/modules/console/console.c \
    src/modules/dex/dex.c \
    src/modules/dotnet/dotnet.c \
    src/modules/elf/elf.c \
    src/modules/math/math.c \
    src/modules/macho/macho.c \
    src/modules/pe/pe.c \
    src/modules/pe/pe_utils.c \
    src/modules/string/string.c \
    src/modules/tests/tests.c \
    src/modules/time/time.c \
    src/notebook.c \
    src/object.c \
    src/parser.c \
    src/proc.c \
    src/re.c \
    src/re_grammar.c \
    src/re_lexer.c \
    src/rules.c \
    src/scan.c \
    src/scanner.c \
    src/sizedstr.c \
    src/stack.c \
    src/stopwatch.c \
    src/stream.c \
    src/strutils.c \
    src/threading.c \
    src/simple_str.c \
    src/tlshc/tlsh.c \
    src/tlshc/tlsh_impl.c \
    src/tlshc/tlsh_util.c

win32 {
    SOURCES += src/proc/windows.c
}

unix:!macx {
    SOURCES += src/proc/linux.c
    SOURCES += src/proc/openbsd.c
    SOURCES += src/proc/freebsd.c
}

unix:macx {
    SOURCES += src/proc/mach.c
}
