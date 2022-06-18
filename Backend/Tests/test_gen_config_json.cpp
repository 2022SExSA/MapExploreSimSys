#include "pg/pgtest/pgtest.h"

#include <fstream>
#include <iostream>
#include <filesystem>

#include "config.h"

PGTEST_CASE(gen_config_json) {
    using namespace pg::messbase;
    namespace fs = std::filesystem;

    auto path = fs::path(__FILE__);
    path = path.parent_path();

    ComponentConfig comp_config;

    // For auth
    comp_config.auth_token = "T";

    // For Redis
    comp_config.redis_board_ip = "127.0.0.1";
    comp_config.redis_board_port = 6379;

    // For RabbitMQ
    comp_config.mq_host = "192.168.111.1";
    comp_config.mq_port = 5672;
    comp_config.mq_username = "pgzxb";
    comp_config.mq_password = "pgzxb";
    /* comp_config.mq_name = ""; */

    Config config;
    (ComponentConfig&)config = comp_config;

    config.fps = 1;

    config.map_config.size = {60, 60};
    config.map_config.map_code = std::string(60 * 60, '0');
    for (int r = 15; r < 40; ++r) {
        for (int c = 15; c < 45; ++c) {
            config.map_config.map_code[r * 60 + c] = '1';
        }
    }

    CarComponentConfig car_config;
    (ComponentConfig&)car_config = comp_config;

    car_config.mq_name = car_config.auth_token + "_Car001";
    car_config.init_pos_r = 0;
    car_config.init_pos_c = 0;
    car_config.light_r = 6;
    config.car_components_config.push_back(car_config);

    car_config.mq_name = car_config.auth_token + "_Car002";
    car_config.init_pos_r = 59;
    car_config.init_pos_c = 59;
    car_config.light_r = 6;
    config.car_components_config.push_back(car_config);

    NaviComponentConfig navi_config;
    (ComponentConfig&)navi_config = comp_config;
    navi_config.mq_name = car_config.auth_token + "_Navi001";
    navi_config.plugin_path = "/home/pgzxb/Documents/DevWorkspace/2022SACourseWorkspace/MapExploreSimSys/Backend/Build/Components/NaviPlugin/libmess_navi_plugin_a_star.so";
    config.navigator_components_config.push_back(navi_config);

    ViewComponentConfig view_config;
    (ComponentConfig&)view_config = comp_config;
    view_config.mq_name = car_config.auth_token + "_View";
    view_config.backrgound_img    = {0, "background.png"};
    view_config.covered_grid_img  = {1, "covered_grid.png"};
    view_config.nonblock_grid_img = {2, "nonblock_grid.jpg"};
    view_config.block_grid_img    = {3, "block_grid.gif"};
    view_config.future_route_img  = {5, "future_route_grid.png"};
    view_config.car_img           = {4, "car.gif"};
    view_config.ws_url = {"", 9876};
    config.view_config = view_config;

    std::ofstream of((path / "assets/simple-config.json").string());
    of << xpack::json::encode(config);

    return true;
}
