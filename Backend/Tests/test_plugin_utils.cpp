#include <iostream>
#include "pg/pgtest/pgtest.h"
#include "ProjectConfig.h"
#include "navigator_plugin_utils.h"

using namespace pg::messbase;

PGTEST_CASE(test_for_plugin_utils) {
    Board::get_instance()->init(
        "MESSAuthToken_1655364796",
        ProjectConfig::get_instance().redis_board_ip,
        ProjectConfig::get_instance().redis_board_port);

    MESSNP_Position src, dest;
    MESSNP_PositionArray dest_pos_array;
    MESSNP_MapInfoGetter *map_info_getter = naviplugin::create_default_map_info_getter();

    Dll plugin("/home/pgzxb/Documents/DevWorkspace/2022SACourseWorkspace/MapExploreSimSys/Backend/Build/Components/NaviPlugin/libmess_navi_plugin_bfs.so");

    src.r = 0;
    src.c = 0;
    auto pos_selector_func = plugin.find_function<MESSNP_NaviSelectDestPosFuncV1>("pg_mess_navi_select_dest_pos_v1");
    auto routing_func = plugin.find_function<MESSNP_NaviRoutingV1>("pg_mess_navi_routing_v1");
    PGZXB_DEBUG_ASSERT(pos_selector_func);
    PGZXB_DEBUG_ASSERT(routing_func);

    [[maybe_unused]] int err = pos_selector_func(&dest, &src, map_info_getter);
    MESS_ERR_IF(err != 0, "", 1);
    MESS_LOG("select dest: (r, c)=({0}, {1})", dest.r, dest.c);
    err = routing_func(&dest_pos_array, &src, &dest, map_info_getter);
    MESS_ERR_IF(err != 0, "", 1);
    MESS_LOG("routing: {0}",
        std::vector<std::pair<int, int>>( // !!!Unsafe!!!
            (std::pair<int, int>*)dest_pos_array.array,
            (std::pair<int, int>*)dest_pos_array.array + dest_pos_array.size
        ));

    return true;
}
