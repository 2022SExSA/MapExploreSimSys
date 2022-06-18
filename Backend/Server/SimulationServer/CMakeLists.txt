add_executable(simulation_server simulation_server.cpp)
target_link_libraries(simulation_server hv MESSBase SQLiteCpp sqlite3 pthread dl)
