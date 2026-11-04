include_directories(${CMAKE_CURRENT_LIST_DIR})
include_directories(${CMAKE_CURRENT_LIST_DIR}/3rdparty/yara/src/include/)

set(XYARA_SOURCES
    ${XYARA_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/xyara.cpp
    ${CMAKE_CURRENT_LIST_DIR}/xyara.h
)

if (DEFINED X_RESOURCES)
    if (NOT DEFINED WITH_PEID OR WITH_PEID)
        install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/yara_rules DESTINATION ${X_RESOURCES} OPTIONAL)
    else()
        install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/yara_rules DESTINATION ${X_RESOURCES} OPTIONAL
            PATTERN "peid.yar" EXCLUDE)
    endif()
endif()
