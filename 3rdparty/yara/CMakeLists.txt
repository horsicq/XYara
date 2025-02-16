cmake_minimum_required(VERSION 3.14)

project(yara LANGUAGES C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(GNUInstallDirs)

include_directories(${PROJECT_SOURCE_DIR}/src/)
include_directories(${PROJECT_SOURCE_DIR}/src/include/)

if(WIN32)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-DUSE_WINDOWS_PROC)
  add_definitions(-DYR_BUILDING_STATIC_LIB)
  add_definitions(-DBUCKETS_128)
  add_definitions(-DCHECKSUM_1B)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  add_definitions(-DHAVE_CLOCK_GETTIME)
  add_definitions(-DHAVE_STDBOOL_H)
  add_definitions(-DHAVE_TIMEGM)
  add_definitions(-DBUCKETS_128)
  add_definitions(-DCHECKSUM_1B)
  add_definitions(-DHAVE_MEMMEM)
  add_definitions(-DUSE_LINUX_PROC)
  add_definitions(-DHAVE_SCAN_PROC_IMPL)
endif()

if(APPLE)
  add_definitions(-DUSE_MACH_PROC)
  add_definitions(-DHAVE_CLOCK_GETTIME)
  add_definitions(-DHAVE_STDBOOL_H)
  add_definitions(-DHAVE_TIMEGM)
  add_definitions(-DBUCKETS_128)
  add_definitions(-DCHECKSUM_1B)
  add_definitions(-DHAVE_MEMMEM)
  add_definitions(-DHAVE_SCAN_PROC_IMPL)
  add_definitions(-DHAVE_STRLCAT) 
  add_definitions(-DHAVE_STRLCPY)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
  add_definitions(-DUSE_FREEBSD_PROC)
  add_definitions(-DHAVE_CLOCK_GETTIME)
  add_definitions(-DHAVE_STDBOOL_H)
  add_definitions(-DHAVE_TIMEGM)
  add_definitions(-DBUCKETS_128)
  add_definitions(-DCHECKSUM_1B)
  add_definitions(-DHAVE_MEMMEM)
  add_definitions(-DHAVE_SCAN_PROC_IMPL)
  add_definitions(-DHAVE_STRLCAT) 
  add_definitions(-DHAVE_STRLCPY)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
  add_definitions(-DUSE_OPENBSD_PROC)
  add_definitions(-DHAVE_CLOCK_GETTIME)
  add_definitions(-DHAVE_STDBOOL_H)
  add_definitions(-DHAVE_TIMEGM)
  add_definitions(-DBUCKETS_128)
  add_definitions(-DCHECKSUM_1B)
  add_definitions(-DHAVE_MEMMEM)
  add_definitions(-DHAVE_SCAN_PROC_IMPL)
  add_definitions(-DHAVE_STRLCAT) 
  add_definitions(-DHAVE_STRLCPY)
endif()

add_library(yara STATIC
${PROJECT_SOURCE_DIR}/src/ahocorasick.c
${PROJECT_SOURCE_DIR}/src/arena.c
${PROJECT_SOURCE_DIR}/src/atoms.c
${PROJECT_SOURCE_DIR}/src/base64.c
${PROJECT_SOURCE_DIR}/src/bitmask.c
${PROJECT_SOURCE_DIR}/src/compiler.c
${PROJECT_SOURCE_DIR}/src/endian.c
${PROJECT_SOURCE_DIR}/src/exec.c
${PROJECT_SOURCE_DIR}/src/exefiles.c
${PROJECT_SOURCE_DIR}/src/filemap.c
${PROJECT_SOURCE_DIR}/src/grammar.c
${PROJECT_SOURCE_DIR}/src/_hash.c
${PROJECT_SOURCE_DIR}/src/hex_grammar.c
${PROJECT_SOURCE_DIR}/src/hex_lexer.c
${PROJECT_SOURCE_DIR}/src/lexer.c
${PROJECT_SOURCE_DIR}/src/libyara.c
${PROJECT_SOURCE_DIR}/src/mem.c
${PROJECT_SOURCE_DIR}/src/modules.c
${PROJECT_SOURCE_DIR}/src/modules/console/console.c
${PROJECT_SOURCE_DIR}/src/modules/dex/dex.c
${PROJECT_SOURCE_DIR}/src/modules/dotnet/dotnet.c
${PROJECT_SOURCE_DIR}/src/modules/elf/elf.c
${PROJECT_SOURCE_DIR}/src/modules/math/math.c
${PROJECT_SOURCE_DIR}/src/modules/macho/macho.c
${PROJECT_SOURCE_DIR}/src/modules/pe/pe.c
${PROJECT_SOURCE_DIR}/src/modules/pe/pe_utils.c
${PROJECT_SOURCE_DIR}/src/modules/string/string.c
${PROJECT_SOURCE_DIR}/src/modules/tests/tests.c
${PROJECT_SOURCE_DIR}/src/modules/time/time.c
${PROJECT_SOURCE_DIR}/src/notebook.c
${PROJECT_SOURCE_DIR}/src/object.c
${PROJECT_SOURCE_DIR}/src/parser.c
${PROJECT_SOURCE_DIR}/src/proc.c
${PROJECT_SOURCE_DIR}/src/proc/none.c
${PROJECT_SOURCE_DIR}/src/re.c
${PROJECT_SOURCE_DIR}/src/re_grammar.c
${PROJECT_SOURCE_DIR}/src/re_lexer.c
${PROJECT_SOURCE_DIR}/src/rules.c
${PROJECT_SOURCE_DIR}/src/scan.c
${PROJECT_SOURCE_DIR}/src/scanner.c
${PROJECT_SOURCE_DIR}/src/sizedstr.c
${PROJECT_SOURCE_DIR}/src/stack.c
${PROJECT_SOURCE_DIR}/src/stopwatch.c
${PROJECT_SOURCE_DIR}/src/stream.c
${PROJECT_SOURCE_DIR}/src/strutils.c
${PROJECT_SOURCE_DIR}/src/threading.c
${PROJECT_SOURCE_DIR}/src/simple_str.c
${PROJECT_SOURCE_DIR}/src/tlshc/tlsh.c
${PROJECT_SOURCE_DIR}/src/tlshc/tlsh_impl.c
${PROJECT_SOURCE_DIR}/src/tlshc/tlsh_util.c
)

if(WIN32)
  target_sources(yara PRIVATE
  ${PROJECT_SOURCE_DIR}/src/proc/windows.c
  )
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  target_sources(yara PRIVATE
  ${PROJECT_SOURCE_DIR}/src/proc/linux.c
  )
endif()

if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
  target_sources(yara PRIVATE
  ${PROJECT_SOURCE_DIR}/src/proc/freebsd.c
  )
endif()

if(CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
  target_sources(yara PRIVATE
  ${PROJECT_SOURCE_DIR}/src/proc/openbsd.c
  )
endif()

if(APPLE)
  target_sources(yara PRIVATE
  ${PROJECT_SOURCE_DIR}/src/proc/mach.c
  )
endif()

set_target_properties(yara PROPERTIES LINKER_LANGUAGE C)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
