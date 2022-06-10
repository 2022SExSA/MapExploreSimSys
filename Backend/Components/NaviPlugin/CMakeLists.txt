include_directories(../)

file(GLOB MESS_NAVI_PLUGIN *.cpp)
foreach(file ${MESS_NAVI_PLUGIN})
    set(file_name ${file})
    string(REGEX REPLACE ".+/(.+)\\..*" "\\1" file_name ${file})
    project(${file_name})
    add_library(mess_navi_plugin_${file_name} SHARED ${file})
endforeach(file)