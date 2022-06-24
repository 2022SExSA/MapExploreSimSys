#ifndef MESS_BACKENDCONFIG_H
#define MESS_BACKENDCONFIG_H

#include "fwd.h"
#include "Xpack/json.h"
#include <cstdlib>
#include <exception>
MESSBASE_NAMESPACE_START

struct ProjectConfig {
    // To find navigator plugin
    std::string navi_plugin_so_root_path;
    // Component path (FIXME: Only localhost now)
    std::string component_exe_root_path;
    // Rendering-orders save root path
    std::string rendering_orders_save_path;
    // Assets root path
    std::string assets_root_path;
    // Board(Redis)
    std::string redis_board_ip;
    unsigned short redis_board_port;
    // RabbitMQ
    std::string rabbitmq_ip;
    unsigned short rabbitmq_port;
    std::string rabbitmq_username;
    std::string rabbitmq_password;
    // View component
    std::string view_component_ip;
    // Database
    std::string experiment_db_url;
    std::string user_db_url;

    XPACK(O(
        navi_plugin_so_root_path,
        component_exe_root_path,
        rendering_orders_save_path,
        assets_root_path,
        redis_board_ip,
        redis_board_port,
        rabbitmq_ip,
        rabbitmq_port,
        rabbitmq_username,
        rabbitmq_password,
        view_component_ip,
        experiment_db_url,
        user_db_url
    ));

    static ProjectConfig &get_instance() {
        static ProjectConfig ins;
        [[maybe_unused]] volatile static bool flag = load_config(ins);
        return ins;
    }
private:
    static bool load_config(ProjectConfig &result) {
        const char *path = std::getenv("PG_MESS_CONFIG_PATH");
        if (!path) {
            MESS_ERR("load config failed, not found `PG_MESS_CONFIG_PATH`", 0);
            return false;
        }
        try {
            xpack::json::decode_file(path, result);
        } catch (const std::exception &e) {
            MESS_ERR("load config failed, parse failed", 0);
            return false;
        }

        return true;
    }

    ProjectConfig() = default;
};

MESSBASE_NAMESPACE_END
#endif
