cmake_minimum_required(VERSION 3.16)
set(DCMAKE_SH="CMAKE_SH-NOTFOUND")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

project(MapExploreSimSys)

include_directories(Base)

add_subdirectory(External/PGUtils)
add_subdirectory(External/SimpleAmqpClient)
include_directories(External/PGUtils)
include_directories(External/nlohmann/include)
include_directories(External/SimpleAmqpClient/src)

SET(CMAKE_BUILD_TYPE "Debug")
add_definitions(-DPGZXB_DEBUG)
add_definitions(-D__STDC_FORMAT_MACROS)
add_definitions(-Wall)
add_definitions(-Wno-unused-parameter)

add_subdirectory(Tests)
add_subdirectory(Components)

file(GLOB MESSBASE_SRC_FILES Base/*.cpp)
add_library(MESSBase STATIC ${MESSBASE_SRC_FILES})
