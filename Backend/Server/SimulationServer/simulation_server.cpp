#include <exception>
#include <hv/httpdef.h>
#include <map>

#include "Xpack/json.h"
#include "jwt-cpp/jwt.h"
#include <hv/HttpServer.h>

#include "db.h"
#include "config.h"
#include "utils.h"
#include "Board.h"
#include "frontend_config.h"

constexpr char CONTROLLER_EXE_PATH[] = "/home/pgzxb/Documents/DevWorkspace/2022SACourseWorkspace/MapExploreSimSys/Backend/Build/Components/Controller";
constexpr char SQLITE_PATH[] = "/home/pgzxb/Documents/DevWorkspace/2022SACourseWorkspace/MapExploreSimSys/Backend/Server/SimulationServer/Experiments.sqlite3";
constexpr char EX_CONFIG_PATH_PREFIX[] = "/home/pgzxb/Documents/DevWorkspace/2022SACourseWorkspace/MapExploreSimSys/Backend/Assets/";

using namespace pg::messbase;

struct MESSContext {
    struct ExperimentRuntimeData {
        ES state = ES::INVALID;
        std::time_t started_at{0};
        std::time_t stoped_at{0};
        Config init_config;
        std::unique_ptr<Board> board;

        XPACK(O(state, init_config));
    };

    std::map<std::string, ExperimentRuntimeData> experiments; // key = auth_token

    XPACK(O(experiments));
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
            ctx->sendJson(make_response_json_data(0, "成功", nullptr));
            mess_ctx.experiments[requset.auth_token].state = new_state;
            if (new_state == ES::EXIT)
                mess_ctx.experiments[requset.auth_token].stoped_at = std::time(nullptr);
        } else {
            ctx->sendJson(make_response_json_data(-1, "失败, 检查后端redis", Json::object()));
        }
        return HTTP_STATUS_OK;
    };
}

static bool jwt_auth(const HttpContextPtr& ctx, const std::string &type) {
    return true; // Tmp
    auto token = ctx->header("token");
    auto decoded = jwt::decode(token);
    auto verifier = jwt::verify()
        .allow_algorithm(jwt::algorithm::hs256{ "PGZXB_MESS_USER2022SExSA" });
        std::error_code err;
    verifier.verify(decoded, err);
    if (err) {
        ctx->sendJson(make_response_json_data(-1, "验证失败", Json::object()));
        return false;
    }
    try {
        auto type = decoded.get_payload_claim("type").as_string();
        if (type != type) {
            ctx->sendJson(make_response_json_data(-1, pgfmt::format("验证失败", Json::object())));
            return false;
        }
    } catch (const std::exception &e) {
        ctx->sendJson(make_response_json_data(-1, pgfmt::format("验证失败, {0}", e.what()), Json::object()));
        return false;
    }

    return true;
}


// For dumping data to sqlite
static struct { void *db, *mess_ctx; } ctx;
static void exit() {

};

int main() {
    auto db = DAO::create_sqlite_ins(SQLITE_PATH);
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
        exper.started_at = std::time(nullptr);
        return HTTP_STATUS_OK;
    });

    router.POST("/start_simulation", new_set_experiment_state_http_service(mess_ctx, ES::RUNNING));
    router.POST("/pause_simulation", new_set_experiment_state_http_service(mess_ctx, ES::PAUSE));
    router.POST("/stop_simulation", new_set_experiment_state_http_service(mess_ctx, ES::EXIT));

    router.POST("/save_simulation", [&mess_ctx, &db](const HttpContextPtr& ctx) {
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

        auto &exper = mess_ctx.experiments[requset.auth_token];
        if (exper.state != ES::EXIT) {
            ctx->sendJson(make_response_json_data(-1, "实验还在运行", Json::object()));
            return HTTP_STATUS_OK;
        }

        // Save to databases
        db_model::Exeriment e;
        e.user_id = exper.init_config.auth_token; // Tmp
        e.started_at = exper.started_at;
        e.stoped_at = exper.stoped_at;
        e.config_path = exper.init_config.auth_token + ".mess_config";
        e.rendering_orders_path = exper.init_config.auth_token + ".mess_show";
 
        // Dump config
        auto json = xpack::json::encode(exper);
        std::ofstream of(EX_CONFIG_PATH_PREFIX + e.config_path);
        PGZXB_DEBUG_ASSERT(of);
        of << json;

        // Save data
        db->add_experiment(e);
        ctx->sendJson(make_response_json_data(0, "保存成功", Json::object()));
        // Save -> Remove from server
        mess_ctx.experiments.erase(requset.auth_token);
        return HTTP_STATUS_OK;
    });

    // Mgr level
    // Read from mess_ctx
    router.POST("/get_running_simulations", [&mess_ctx](const HttpContextPtr& ctx) {
        if (!jwt_auth(ctx, "m")) return HTTP_STATUS_OK;

        ctx->sendJson(make_response_json_data(0, Json::parse(xpack::json::encode(mess_ctx))));
        return HTTP_STATUS_OK;
    });

    // Try read from sqlite
    router.POST("/get_history_simulations", [&db](const HttpContextPtr& ctx) {
        if (!jwt_auth(ctx, "m")) return HTTP_STATUS_OK;

        std::vector<db_model::Exeriment> result = db->get_all_experiments();

        ctx->sendJson(make_response_json_data(0, Json::parse(xpack::json::encode(result))));
        return HTTP_STATUS_OK;
    });

    router.document_root = EX_CONFIG_PATH_PREFIX;

    hv::HttpServer server;
    server.registerHttpService(&router);
    server.setPort(8080);
    server.run();
    return 0;
}