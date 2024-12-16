include_directories(${CMAKE_CURRENT_LIST_DIR})
include_directories(${CMAKE_CURRENT_LIST_DIR}/3rdparty/yara/src/include/)

set(XYARA_SOURCES
    ${XYARA_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/xyara.cpp
    ${CMAKE_CURRENT_LIST_DIR}/xyara.h
)
