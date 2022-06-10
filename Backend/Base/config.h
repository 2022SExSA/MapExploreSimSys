#ifndef MESSBASE_CONFIG_H
#define MESSBASE_CONFIG_H

#include "fwd.h"
#include "utils.h"
#include "Xpack/json.h"
MESSBASE_NAMESPACE_START

struct ComponentConfig {
    std::string host{"localhost"};
    short port{0};
    std::string username;
    std::string password;
    std::string name;

    XPACK(O(host, username, password, name));
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

struct InitConfig {
    MapInitConfig map_config;
    std::vector<CarComponentConfig> car_components_config;
    std::vector<NaviComponentConfig> navigator_components_config;

    XPACK(O(map_config, car_components_config, navigator_components_config));
};

MESSBASE_NAMESPACE_END
#endif
