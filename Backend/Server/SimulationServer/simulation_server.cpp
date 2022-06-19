#include <map>
#include <thread>
#include <exception>

#include "Xpack/json.h"
#include "fwd.h"
#include "jwt-cpp/jwt.h"
#include <hv/WebSocketServer.h>

#include "db.h"
#include "config.h"
#include "utils.h"
#include "Board.h"
#include "frontend_config.h"

MESS_LOG_MODULE("Simulation+Statistics Server");

constexpr char CONTROLLER_EXE_PATH[] = "/home/pgzxb/Documents/DevWorkspace/2022SACourseWorkspace/MapExploreSimSys/Backend/Build/Components/Controller";
constexpr char SQLITE_PATH[] = "/home/pgzxb/Documents/DevWorkspace/2022SACourseWorkspace/MapExploreSimSys/Backend/Server/SimulationServer/Experiments.sqlite3";
constexpr char EX_CONFIG_PATH_PREFIX[] = "/home/pgzxb/Documents/DevWorkspace/2022SACourseWorkspace/MapExploreSimSys/Backend/Assets/";

using namespace pg::messbase;

struct StatisticsResponseData {
    std::string auth_token;
    double time{0.f};
    int frame_cnt{0};
    int light_grid{0};

    XPACK(O(auth_token, time, frame_cnt, light_grid));
};

struct StatisticsContext {
    std::vector<frontend_def::ExperimentConfig> config_list;
    std::shared_ptr<std::thread> sender_th;
};

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

static void free_statistics_runtime(MESSContext &mess_ctx, const std::unique_ptr<DAO> &db, const WebSocketChannelPtr& channel) {
    if (!channel->context()) {
        MESS_LOG("Null context (when free_statistics_runtime)", 1);
        return;
    }
    auto &ctx = *channel->getContext<StatisticsContext>();

    for (const auto &e : ctx.config_list) {
        const auto &token = e.auth_token;
        auto iter  = mess_ctx.experiments.find(token);
        if (iter != mess_ctx.experiments.end()) {
            iter->second.board->set_experiment_state(ES::EXIT);

            //Save to DB
            auto &exper = iter->second;
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
            iter->second.board->set_experiment_state(ES::EXIT);
            mess_ctx.experiments.erase(iter);
        }
    }
    if (ctx.sender_th && ctx.sender_th->joinable()) ctx.sender_th->join();
    channel->deleteContext<StatisticsContext>();
    channel->setContext(nullptr);
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

    // WebSocket Service
    hv::WebSocketService ws;
    
    ws.onopen = [](const WebSocketChannelPtr& channel, const std::string& url) {
        MESS_LOG("New WebSocket connection(from=\"{0}\", with url=\"{1}\")",
            channel->peeraddr(), url);
        channel->setContext(nullptr);
    };

    ws.onmessage = [&mess_ctx, &db](const WebSocketChannelPtr& channel, const std::string& msg) {
        if (msg.empty()) {
            MESS_LOG("Empty request", 1);
            return;
        }

        auto op = msg[0];
        if (op == 'C') { // Create
            free_statistics_runtime(mess_ctx, db, channel);
            std::vector<frontend_def::ExperimentConfig> configs;
            try {
                xpack::json::decode(msg.substr(1), configs);            
            } catch (const std::exception &e) {
                MESS_LOG("Parse configs(msg[1:]) failed, err={0} msg={1}", e.what(), msg);
                return;
            }
            auto &ctx = *channel->newContext<StatisticsContext>();
            auto &config_list = ctx.config_list;
            config_list = std::move(configs);
            // Launch experiments
            for (std::size_t i = 0; i < config_list.size(); ++i) {
                auto &e = config_list[i];
                // e.auth_token = e.auth_token + "_" + std::to_string(i); // [Mvto Frontend]Unique auth_token
                Config config = frontend_def::default_trans_frontend_config_to_backend(e);
                launch_component(CONTROLLER_EXE_PATH, xpack::json::encode(config));
                auto &exper = mess_ctx.experiments[config.auth_token];
                exper.board = Board::new_instance(config.auth_token, config.redis_board_ip, config.redis_board_port);
                exper.init_config = std::move(config);
                exper.started_at = std::time(nullptr);
            }
        } else if (op == 'S' || op == 'P') { // Start
            if (!channel->context()) {
                MESS_LOG("Null context. Not experiments", 1);
                return;
            }
            auto &context = *channel->getContext<StatisticsContext>();
            auto next_state = op == 'S' ? ES::RUNNING : ES::PAUSE;
            for (const auto &e : context.config_list) {
                const auto &token = e.auth_token;
                auto iter  = mess_ctx.experiments.find(token);
                if (iter != mess_ctx.experiments.end()) {
                    iter->second.board->set_experiment_state(next_state);
                } else {
                    MESS_LOG("{0} not found", token);
                }
            }
            if (!context.sender_th.get()) {
                context.sender_th.reset(new std::thread([channel, context, &mess_ctx]() {
                    while(true) {
                        if (channel->isClosed()) return;
                        for (const auto &e : context.config_list) {
                            const auto &token = e.auth_token;
                            auto iter  = mess_ctx.experiments.find(token);
                            if (iter != mess_ctx.experiments.end()) {
                                auto fr_cnt = iter->second.board->get_frame_cnt();
                                StatisticsResponseData data;
                                data.time = (double)fr_cnt * (1.0 / e.fps);
                                data.frame_cnt = fr_cnt;
                                data.auth_token = e.auth_token;
                                data.light_grid = iter->second.board->count_raw_grid_of_map();
                                channel->send(xpack::json::encode(data));
                            } else {
                                MESS_LOG("{0} not found", token);
                            }
                        }
                        using namespace std::chrono_literals;
                        std::this_thread::sleep_for(500ms);
                    }
                }));
            }
        } else if (op == 'E') {
            free_statistics_runtime(mess_ctx, db, channel);
        } else {
            MESS_LOG("Invalid request: {0}", msg);
        }
    };

    ws.onclose = [&mess_ctx, &db](const WebSocketChannelPtr& channel) {
        MESS_LOG("Close WebSocket connection(from=\"{0}\")",
            channel->peeraddr());
       free_statistics_runtime(mess_ctx, db, channel);
    };

    websocket_server_t server;
    server.service = &router;
    server.ws = &ws;
    server.port = 8080;
    // server.worker_threads = 8;
    websocket_server_run(&server); // Block
    return 0;
}