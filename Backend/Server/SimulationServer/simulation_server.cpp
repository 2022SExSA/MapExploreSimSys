#include <exception>
#include <hv/httpdef.h>
#include <map>

#include "Xpack/json.h"
#include <hv/HttpServer.h>

#include "config.h"
#include "utils.h"
#include "Board.h"
#include "frontend_config.h"

constexpr char CONTROLLER_EXE_PATH[] = "/home/pgzxb/Documents/DevWorkspace/2022SACourseWorkspace/MapExploreSimSys/Backend/Build/Components/Controller";

using namespace pg::messbase;

struct MESSContext {
    struct ExperimentRuntimeData {
        Config init_config;
        std::unique_ptr<Board> board;
    };

    std::map<std::string, ExperimentRuntimeData> experiments; // key = auth_token
};

static auto new_set_experiment_state_http_service(MESSContext &mess_ctx, ES new_state) {
    return [&mess_ctx, new_state](const HttpContextPtr& ctx) {
        const auto &body = ctx->body();

        // { "auth_token": "..." }
        frontend_def::StartSimulationRequest requset;
        try {
            xpack::json::decode(body, requset);
        } catch (const std::exception &e) {
            ctx->sendJson(make_response_json_data(-1, "请求非法", Json::object()));
            return HTTP_STATUS_OK;
        }

        if (!mess_ctx.experiments.count(requset.auth_token)) {
            ctx->sendJson(make_response_json_data(-1, "验证失败", Json::object()));
            return HTTP_STATUS_OK;
        }

        if (mess_ctx.experiments[requset.auth_token].board->set_experiment_state(new_state)) {
            ctx->sendJson(make_response_json_data(0, "开始成功", nullptr));
        } else {
            ctx->sendJson(make_response_json_data(-1, "开始失败, 检查后端redis", Json::object()));
        }
        return HTTP_STATUS_OK;
    };
}

int main() {
    hv::HttpService router;
    MESSContext mess_ctx;

    router.POST("/simulation", [&mess_ctx](const HttpContextPtr& ctx) {
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
        auto &exper = mess_ctx.experiments[config.auth_token];
        exper.board = Board::new_instance(config.auth_token, config.redis_board_ip, config.redis_board_port);
        exper.init_config = std::move(config);
        return HTTP_STATUS_OK;
    });

    router.POST("/start_simulation", new_set_experiment_state_http_service(mess_ctx, ES::RUNNING));
    router.POST("/pause_simulation", new_set_experiment_state_http_service(mess_ctx, ES::PAUSE));
    router.POST("/stop_simulation", new_set_experiment_state_http_service(mess_ctx, ES::EXIT));

    hv::HttpServer server;
    server.registerHttpService(&router);
    server.setPort(8080);
    server.run();
    return 0;
}