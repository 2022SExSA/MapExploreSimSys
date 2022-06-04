include_directories(../external/PGUtils)

file(GLOB MESSBASE_TESTS_FILES *.cpp)
add_executable(MESSBASE_test ${MESSBASE_TESTS_FILES})
target_link_libraries(MESSBASE_test PGUtils PGTestMain)
