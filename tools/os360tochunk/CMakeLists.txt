include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(os360tochunk ${HEADERS} ${SOURCES})

install(TARGETS os360tochunk)