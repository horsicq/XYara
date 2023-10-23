QT       -= core gui

TARGET = yara
TEMPLATE = lib
CONFIG += staticlib

linux {
    QMAKE_CFLAGS += -std=c11
    DEFINES += "_GNU_SOURCE"
}

freebsd {
    QMAKE_CFLAGS += -std=c11
    DEFINES += "_GNU_SOURCE"
}
openbsd {
    QMAKE_CFLAGS += -std=c11
    DEFINES += "_GNU_SOURCE"
}

CONFIG += c++11

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD/src
DEPENDPATH += $$PWD/src
INCLUDEPATH += $$PWD/src/include
DEPENDPATH += $$PWD/src/include

include(../../build.pri)

#DEFINES += "OPENSSL_NO_DEPRECATED_3_0"
#DEFINES += "HAVE_LIBCRYPTO"

#DEFINES += "CUCKOO_MODULE"
#DEFINES += "HASH_MODULE"
#DEFINES += "DOTNET_MODULE"
#DEFINES += "YR_BUILDING_STATIC_LIB"
#DEFINES += "BUCKETS_256"
#DEFINES += "BUCKETS_128"
# pthreads linux
# DEFINES += "_THREAD_SAFE" # macOS check
# DEFINES += "HASH_MODULE"  # macOS check
# TODO Check DEFS  in MakeFile
# TODO Check bazel/yara.bzl

win32 {
	DEFINES += "_CRT_SECURE_NO_WARNINGS"
	DEFINES += "USE_WINDOWS_PROC"
	DEFINES += "YR_BUILDING_STATIC_LIB"
	#DEFINES += "NDEBUG=1"
	DEFINES += "BUCKETS_128=1"
	DEFINES += "CHECKSUM_1B=1"
}
linux {
	DEFINES += "HAVE_CLOCK_GETTIME=1"
	DEFINES += "HAVE_STDBOOL_H=1"
	DEFINES += "HAVE_TIMEGM=1"
	DEFINES += "BUCKETS_128=1"
	DEFINES += "CHECKSUM_1B=1"
	DEFINES += "HAVE_MEMMEM=1"
	DEFINES += "USE_LINUX_PROC"
	DEFINES += "HAVE_SCAN_PROC_IMPL=1"
}
macx {
	DEFINES += "HAVE_CLOCK_GETTIME=1"
	DEFINES += "HAVE_STDBOOL_H=1"
	DEFINES += "HAVE_TIMEGM=1"
	DEFINES += "BUCKETS_128=1"
	DEFINES += "CHECKSUM_1B=1"
	DEFINES += "HAVE_MEMMEM=1"
	DEFINES += "USE_MACH_PROC"
	DEFINES += "HAVE_SCAN_PROC_IMPL=1"
	DEFINES += "HAVE_STRLCAT=1"
	DEFINES += "HAVE_STRLCPY=1"
}
freebsd {
	DEFINES += "HAVE_CLOCK_GETTIME=1"
	DEFINES += "HAVE_STDBOOL_H=1"
	DEFINES += "HAVE_TIMEGM=1"
	DEFINES += "BUCKETS_128=1"
	DEFINES += "CHECKSUM_1B=1"
    DEFINES += "USE_FREEBSD_PROC"
    DEFINES += "HAVE_SCAN_PROC_IMPL=1"
    DEFINES += "HAVE_STRLCAT=1"
    DEFINES += "HAVE_STRLCPY=1"
}
openbsd {
	DEFINES += "HAVE_CLOCK_GETTIME=1"
	DEFINES += "HAVE_STDBOOL_H=1"
	DEFINES += "HAVE_TIMEGM=1"
	DEFINES += "BUCKETS_128=1"
	DEFINES += "CHECKSUM_1B=1"
    DEFINES += "USE_OPENBSD_PROC"
    DEFINES += "HAVE_SCAN_PROC_IMPL=1"
    DEFINES += "HAVE_STRLCAT=1"
    DEFINES += "HAVE_STRLCPY=1"
}

TARGETLIB_PATH = $$PWD
DESTDIR=$${TARGETLIB_PATH}/libs

win32{
    TARGET = yara-win-$${QT_ARCH}
}
unix:!macx {
    TARGET = yara-unix-$${QT_ARCH}
}
unix:macx {
    TARGET = yara-macos-$${QT_ARCH}
}

# hash -> _hash
SOURCES += \
    src/ahocorasick.c \
    src/arena.c \
    src/atoms.c \
    src/base64.c \
    src/bitmask.c \
    src/compiler.c \
    src/endian.c \
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
    src/proc/none.c \
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
linux {
    SOURCES += src/proc/linux.c
}
openbsd {
    SOURCES += src/proc/openbsd.c
}
freebsd {
    SOURCES += src/proc/freebsd.c
}
unix:macx {
    SOURCES += src/proc/mach.c
}
