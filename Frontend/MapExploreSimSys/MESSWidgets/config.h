#ifndef MESSBASE_CONFIG_H
#define MESSBASE_CONFIG_H

#include "xpack/json.h"

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

#endif
