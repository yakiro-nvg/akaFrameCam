# Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
# Unauthorized copying of this file, via any medium is strictly prohibited.
cmake_minimum_required(VERSION 3.1)

project(cam)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(WIN32 AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set(CMAKE_INSTALL_PREFIX "C:/akaFrame/cam" CACHE PATH "install prefix" FORCE)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

enable_testing()

set(CMAKE_CXX_STANDARD 11)

find_package(Threads)

add_subdirectory(
        ${CMAKE_SOURCE_DIR}/vendor/rapidcheck EXCLUDE_FROM_ALL)

add_definitions(-DCAM_Z390)
add_definitions(-DCAM_ASSERTION)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

add_subdirectory(source)
add_subdirectory(test)
add_subdirectory(tools)
