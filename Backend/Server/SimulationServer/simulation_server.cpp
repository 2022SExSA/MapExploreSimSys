#include <exception>

#include "Xpack/json.h"
#include <hv/HttpServer.h>

#include "config.h"
#include "utils.h"
#include "frontend_config.h"

constexpr char CONTROLLER_EXE_PATH[] = "/home/pgzxb/Documents/DevWorkspace/2022SACourseWorkspace/MapExploreSimSys/Backend/Build/Components/Controller";

using namespace pg::messbase;

int main() {
    hv::HttpService router;

    router.POST("/simulation", [](const HttpContextPtr& ctx) {
        const auto &body = ctx->body();
        
        // Get frontend-config
        frontend_def::ExperimentConfig config_from_frontend;
        try {
            xpack::json::decode(body, config_from_frontend);
        } catch (const std::exception &e) {
            ctx->sendJson(make_response_json_data(-1, "配置数据非法", Json::object()));
            return HTTP_STATUS_OK;
        }
        // Trans to frontend-config
        Config config = frontend_def::default_trans_frontend_config_to_backend(config_from_frontend);

        launch_component(CONTROLLER_EXE_PATH, xpack::json::encode(config));

        Json resp = Json::object();
        resp["view_http_url"] = "http://192.168.111.148:" + std::to_string(config.view_config.ws_url.port);
        resp["view_ws_url"]   = "ws://192.168.111.148:"   + std::to_string(config.view_config.ws_url.port);
        ctx->sendJson(make_response_json_data(0, resp));
        return HTTP_STATUS_OK;
    });

    hv::HttpServer server;
    server.registerHttpService(&router);
    server.setPort(8080);
    server.run();
    return 0;
}