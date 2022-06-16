#ifndef MESS_FRONEND_CONFIG_H
#define MESS_FRONEND_CONFIG_H

#include "fwd.h"
#include "utils.h"
#include "config.h"
#include "Xpack/json.h"
MESSBASE_NAMESPACE_START
namespace frontend_def {

struct ComponentConfig {
    std::string auth_token{"T"};
    XPACK(O(auth_token));
};

struct CarComponentConfig : public ComponentConfig {
    int init_pos_r{0};
    int init_pos_c{0};
    int light_r{0};

    XPACK(I(ComponentConfig), O(init_pos_r, init_pos_c, light_r));
};

struct NaviComponentConfig : public ComponentConfig {
    std::string plugin_id;

    XPACK(I(ComponentConfig), O(plugin_id));
};

struct MapInitConfig {
    struct Size {
        int width{0};
        int height{0};
        XPACK(O(width, height));
    } size;
    std::string map_code; // bitmap

    XPACK(O(size, map_code));
};

struct ExperimentConfig : public ComponentConfig {
    int fps{60};
    MapInitConfig map_config;
    std::vector<CarComponentConfig> car_components_config;
    std::vector<NaviComponentConfig> navigator_components_config;

    XPACK(
        I(ComponentConfig),
        O(/*mq_config, */fps, map_config, car_components_config, navigator_components_config)
    );
};

inline messbase::Config default_trans_frontend_config_to_backend(const ExperimentConfig &frontend_config) {
    messbase::ComponentConfig comp_config;

    // For auth
    comp_config.auth_token = frontend_config.auth_token;

    // For Redis
    comp_config.redis_board_ip = "127.0.0.1";
    comp_config.redis_board_port = 6379;

    // For RabbitMQ
    comp_config.mq_host = "192.168.111.1";
    comp_config.mq_port = 5672;
    comp_config.mq_username = "pgzxb";
    comp_config.mq_password = "pgzxb";
    /* comp_config.mq_name = ""; */

    messbase::Config result;
    (messbase::ComponentConfig&)result = comp_config;

    result.fps = frontend_config.fps;

    result.map_config.size = {
        frontend_config.map_config.size.width,
        frontend_config.map_config.size.height
    };
    result.map_config.map_code = frontend_config.map_config.map_code;

    int cnt = 0;
    for (const auto &e : frontend_config.car_components_config) {
        messbase::CarComponentConfig car_config;
        (messbase::ComponentConfig&)car_config = comp_config;
        car_config.mq_name = car_config.auth_token + "_Car" + std::to_string(++cnt);
        car_config.init_pos_r = e.init_pos_r;
        car_config.init_pos_c = e.init_pos_c;
        car_config.light_r = e.light_r;
        result.car_components_config.push_back(car_config);
    }

    cnt = 0;
    for (const auto &e : frontend_config.navigator_components_config) {
        messbase::NaviComponentConfig navi_config;
        (messbase::ComponentConfig&)navi_config = comp_config;
        navi_config.mq_name = navi_config.auth_token + "_Navi" + std::to_string(++cnt);
        navi_config.plugin_path = get_navi_plugin_by_id(e.plugin_id);
        result.navigator_components_config.push_back(navi_config);
    }

    messbase::ViewComponentConfig view_config;
    (messbase::ComponentConfig&)view_config = comp_config;
    view_config.mq_name = result.auth_token + "_View";
    view_config.backrgound_img    = {0, "background.png"};
    view_config.covered_grid_img  = {1, "covered_grid.png"};
    view_config.nonblock_grid_img = {2, "nonblock_grid.jpg"};
    view_config.block_grid_img    = {3, "block_grid.gif"};
    view_config.car_img           = {4, "car.gif"};
    view_config.ws_url = {"", get_available_port()};
    result.view_config = view_config;

    return result;
}

} // namespace frontend_def
MESSBASE_NAMESPACE_END
#endif // !MESS_FRONEND_CONFIG_H
