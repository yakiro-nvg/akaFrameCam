# Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
# Unauthorized copying of this file, via any medium is strictly prohibited.
file(GLOB_RECURSE INC_HEADERS ${CMAKE_SOURCE_DIR}/include/*.h)
file(GLOB_RECURSE INC_INLINES ${CMAKE_SOURCE_DIR}/include/*.inl)
file(GLOB_RECURSE SRC_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE SRC_INLINES ${CMAKE_CURRENT_SOURCE_DIR}/*.inl)
file(GLOB_RECURSE SRC_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_definitions(-DCAM_EXPORTS)

add_library(akaFrameCam SHARED
        ${INC_HEADERS} ${INC_INLINES}
        ${SRC_HEADERS} ${SRC_INLINES} ${SRC_SOURCES})

target_include_directories(akaFrameCam PUBLIC ${CMAKE_SOURCE_DIR}/include)

install(TARGETS akaFrameCam)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h")
