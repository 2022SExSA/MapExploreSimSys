include_directories(../external/PGUtils)

file(GLOB MESSBASE_TESTS_FILES test_*.cpp)
add_executable(MESSBASE_test ${MESSBASE_TESTS_FILES})
target_link_libraries(MESSBASE_test MESSBase PGUtils PGTestMain dl)

file(GLOB MESSBASE_DEMOS_FILES demo_*.cpp)
foreach(file ${MESSBASE_DEMOS_FILES})
    set(file_name ${file})
    string(REGEX REPLACE ".+/(.+)\\..*" "\\1" file_name ${file})
    project(${file_name})
    add_executable(${file_name} ${file})
    target_link_libraries(${file_name} MESSBase SimpleAmqpClient dl)
endforeach(file)