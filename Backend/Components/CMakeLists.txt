add_subdirectory(NaviPlugin)

include_directories(../src)

file(GLOB MESSS_COMPONENTS *.cpp)
foreach(file ${MESSS_COMPONENTS})
    set(file_name ${file})
    string(REGEX REPLACE ".+/(.+)\\..*" "\\1" file_name ${file})
    project(${file_name})
    add_executable(${file_name} ${file})
    target_link_libraries(${file_name} MESSBase SimpleAmqpClient dl hv)
endforeach(file)
