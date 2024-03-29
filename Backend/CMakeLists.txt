cmake_minimum_required(VERSION 3.16)
set(DCMAKE_SH="CMAKE_SH-NOTFOUND")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

project(MapExploreSimSys)

include_directories(Base)

add_subdirectory(External/PGUtils)
add_subdirectory(External/SimpleAmqpClient)
add_subdirectory(External/SQLiteCpp)
include_directories(External/PGUtils)
include_directories(External/nlohmann/include)
include_directories(External/SimpleAmqpClient/src)
include_directories(External/xpack)
include_directories(External/jwt-cpp/include)
include_directories(External/SQLiteCpp/include)

SET(CMAKE_BUILD_TYPE "Debug")
add_definitions(-DPGZXB_DEBUG)
add_definitions(-DMESS_WITH_LOG)
add_definitions(-DMESSBASE_LIB)
add_definitions(-D__STDC_FORMAT_MACROS)
add_definitions(-Wall)
add_definitions(-Wno-unused-parameter)

add_subdirectory(Tests)
add_subdirectory(Components)
add_subdirectory(Server/SimulationServer)

file(GLOB MESSBASE_SRC_FILES Base/*.cpp)
add_library(MESSBase STATIC ${MESSBASE_SRC_FILES})
target_link_libraries(MESSBase hiredis)

# Generate default config file
set(MESS_BACKEND_SOURCE_ROOT_PATH ${CMAKE_SOURCE_DIR})
configure_file(Template/config.template.json ${MESS_BACKEND_SOURCE_ROOT_PATH}/default_config.json)
