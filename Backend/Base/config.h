#ifndef MESSBASE_CONFIG_H
#define MESSBASE_CONFIG_H

#include "fwd.h"
#include "Xpack/json.h"
MESSBASE_NAMESPACE_START

struct MQConfig {
    std::string mq_host{"localhost"};
    short mq_port{0};
    std::string mq_username;
    std::string mq_password;
    std::string mq_name;

    XPACK(O(mq_host, mq_port, mq_username, mq_password, mq_name));
};

struct ComponentConfig : public MQConfig {
    std::string auth_token{"T"};
    std::string redis_board_ip;
    short redis_board_port;
    XPACK(I(MQConfig), O(auth_token, redis_board_ip, redis_board_port));
};

struct CarComponentConfig : public ComponentConfig {
    int init_pos_r{0};
    int init_pos_c{0};
    int light_r{0};

    XPACK(I(ComponentConfig), O(init_pos_r, init_pos_c, light_r));
};

struct NaviComponentConfig : public ComponentConfig {
    std::string plugin_path;

    XPACK(I(ComponentConfig), O(plugin_path));
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

struct Config : public ComponentConfig {
    // MQConfig mq_config;
    int fps{60};
    MapInitConfig map_config;
    std::vector<CarComponentConfig> car_components_config;
    std::vector<NaviComponentConfig> navigator_components_config;

    XPACK(I(ComponentConfig), O(/*mq_config, */fps, map_config, car_components_config, navigator_components_config));
};

MESSBASE_NAMESPACE_END
#endif