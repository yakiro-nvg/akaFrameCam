# Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
# Unauthorized copying of this file, via any medium is strictly prohibited.
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/vendor/Catch2/contrib)
include_directories(${CMAKE_SOURCE_DIR}/vendor/Catch2/single_include/catch2)
include(Catch)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(akaFrameCamTest ${HEADERS} ${SOURCES})

target_link_libraries(akaFrameCamTest
        akaFrameCam ${CMAKE_THREAD_LIBS_INIT} rapidcheck)

catch_discover_tests(akaFrameCamTest)
