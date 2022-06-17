#include "Xpack/json.h"
#include "config.h"
#include "fwd.h"
#include "run_component.h"
#include "Board.h"

#include <hv/WebSocketChannel.h>
#include <hv/WebSocketServer.h>
#include <hv/HttpService.h>

#include <map>
#include <list>

#define MESS_WITH_LOG
MESS_LOG_MODULE("View");

using namespace pg::messbase;

enum class GRID_TYPE {
    NONE = 0,
    COVERED = 1,
    NONBLOCK = NONE,
    BLOCK = 2,
    CAR = 3,
    FUTURE_ROUTE = 4,
};

struct RenderOrder {
    enum class Code : int {
        NOP, CLEAR, DRAW, 
    };

    static constexpr const char *CODE2STRING[] = {
        "N", "C", "D",
    };
    
    Code code{Code::NOP};
    std::vector<int> args;

    std::string stringify() const {
        std::string res(CODE2STRING[(int)code]);
        for (auto arg : args) {
            res.append(" ").append(std::to_string(arg));
        }
        return res;
    }

    bool from_json(const Json &json) {
        if (!json.contains("op")) return false;
        auto c = json["op"].get<std::string>();
        if (c == "N") code = Code::NOP;
        else if (c == "C") code = Code::CLEAR;
        else if (c == "D") code = Code::DRAW;
        else return false;;

        if (!json.contains("args")) return false;
        auto arr = json["args"];
        args = arr.get<std::vector<int>>();
    }

    Json to_json() const {
        Json res;
        res["op"] = CODE2STRING[(int)code];
        res["args"] = args;
        return res;
    }

    bool operator== (const RenderOrder &o) const {
        return this->code == o.code && this->args == o.args;
    }
};

class ViewComponent {
public:
    using SubscriberList = std::list<WebSocketChannelPtr>;
    using SubscriberIter = SubscriberList::const_iterator;

    ViewComponent(const ViewComponentConfig &config) : config_(config) {
    }

    void run() {
        of_.open(config_.orders_saved_path);
        PGZXB_DEBUG_ASSERT(of_.is_open());
        run_component(config_, std::bind(msg_proc, this, std::placeholders::_1));
    }

    SubscriberIter add_subscriber_channel(WebSocketChannelPtr chann) {
        subscriber_channels_.push_front(chann);
        return subscriber_channels_.cbegin();
    }

    void remove_subscriber_channel(SubscriberIter iter) {
        subscriber_channels_.erase(iter);
    }
private:
    void display(const std::vector<RenderOrder> &orders) {
        Json resp_data;
        auto &orders_json = (resp_data["orders"] = Json::array());
        for (const auto &e : orders) {
            orders_json.push_back(e.to_json());
        }
        auto resp_json = make_response_json_data(0, resp_data);
        std::string json_str = resp_json.dump();
        for (const auto &chann : subscriber_channels_) {
            if (chann) {
                chann->send(json_str);
            } else {
                MESS_LOG("Nullptr chann", 1);
            }
        }
    }

    std::vector<RenderOrder> gen_rendering_orders(std::size_t feame_cnt, const std::vector<std::vector<GRID_TYPE>> &grids, bool background = true) {
        const int h = grids.size();
        PGZXB_DEBUG_ASSERT(h > 0);
        const int w = grids.back().size();
        int virtual_grid_width = 50;
        int virtual_grid_height = 50;

        std::vector<RenderOrder> orders;

        // 0 level: background
        if (background) {
            int virtual_map_width = virtual_grid_width * w;
            int virtual_map_height = virtual_grid_height * h;
            RenderOrder order;
            order.code = RenderOrder::Code::DRAW;
            order.args = { // id, x, y, w, h, theta, freame_cnt
                config_.backrgound_img.id,
                0, 0,
                virtual_map_width,
                virtual_map_height,
                270 * D2I_FACTOR,
                (int)feame_cnt
            };
            orders.push_back(order);
        }
        
        for (int r = 0; r < h; ++r) {
            for (int c = 0; c < w; ++c) {
                RenderOrder order;
                order.code = RenderOrder::Code::DRAW;
                order.args = { // id, x, y, w, h, theta, freame_cnt
                    -1,
                    c * virtual_grid_width + 1,
                    r * virtual_grid_height + 1,
                    virtual_grid_width - 2,
                    virtual_grid_height - 2,
                    270 * D2I_FACTOR,
                    (int)feame_cnt
                };

                switch (grids[r][c]) {
                case GRID_TYPE::NONE :
                /* case GRID_TYPE::NONBLOCK : */
                    break;
                case GRID_TYPE::COVERED :
                    order.args[0] = config_.covered_grid_img.id;
                    break;
                case GRID_TYPE::BLOCK :
                    order.args[0] = config_.block_grid_img.id;
                    break;
                case GRID_TYPE::CAR :
                    order.args[0] = config_.car_img.id;
                    break;
                case GRID_TYPE::FUTURE_ROUTE : 
                    order.args[0] = config_.future_route_img.id;
                    break;
                default: break;
                }

                if (order.args[0] != -1) orders.push_back(order);
            }
        }
        return orders;
    }

    static std::string msg_proc(ViewComponent *view, const std::string &msg) {
        auto op = Op::from_string(msg);
        if (!op.has_value()) {
            MESS_LOG("recv invalid msg: msg=\"{0}\"", msg);
            return "";
        }
        
        if (op.value().auth_token != Board::get_instance()->get_auth_token()) {
            MESS_LOG("Auth Failed", 1);
            return "";
        }

        if (op->op == "exit") {
            view->of_.flush();
            exit(0);
        }

        if (op.value().op == "update") {
            auto &board = *Board::get_instance();
            const auto [w, h] = board.get_map_size();
            std::vector<std::vector<GRID_TYPE>> grids(h, std::vector<GRID_TYPE>(w, GRID_TYPE::NONE));
    
            // 0 level: background
            // ... Construct in gen_rendering_orders

            // 1 level: block(s)
            for (int r = 0; r < h; ++r) {
                for (int c = 0; c < w; ++c) {
                    if (board.get_grid_of_map_block(r, c)) {
                        grids[r][c] = GRID_TYPE::BLOCK;
                    } else {
                        grids[r][c] = GRID_TYPE::NONE;
                    }
                }
            }

            // 2 level: (if-)covered
            for (int r = 0; r < h; ++r) {
                for (int c = 0; c < w; ++c) {
                    if (board.get_grid_of_map(r, c)) {
                        grids[r][c] = GRID_TYPE::COVERED;
                    } /* else {
                        // Do nothing
                    } */
                }
            }

            // 3 level: car(s)
            {
                auto car_ids = board.get_all_cars();
                for (const auto &id : car_ids) {
                    auto pos_opt = board.get_current_position_of_car(id);
                    if (pos_opt.has_value()) {
                        const auto &pos = pos_opt.value();
                        if (pos.x < 0 || pos.x >= w || pos.y < 0 || pos.y >= h) {
                            MESS_LOG("car={0} out of bound(w={1}, h={2})", id, w, h);
                            continue;
                        }
                        grids[pos.y][pos.x] = GRID_TYPE::CAR;
                    }
                }
            }

            std::vector<std::vector<GRID_TYPE>> future_route_grids(h, std::vector<GRID_TYPE>(w, GRID_TYPE::NONE));
            {
                auto car_ids = board.get_all_cars();
                for (const auto &id : car_ids) {
                    auto pos_list = board.get_all_pos_of_routlist(id);
                    for (const auto &e : pos_list) {
                        if (e.x < 0 || e.x >= w || e.y < 0 || e.y >= h) {
                            MESS_LOG("Routlist of car={0} out of bound(w={1}, h={2})", id, w, h);
                            continue;
                        }
                        future_route_grids[e.y][e.x] = GRID_TYPE::FUTURE_ROUTE;
                    }
                }
            }

            // Gen rendering-orders & Send orders to channels
            auto orders = view->gen_rendering_orders(op->frame_cnt, grids, true);
            auto future_route_orders = view->gen_rendering_orders(op->frame_cnt, future_route_grids, false);
            for (auto &e : future_route_orders) orders.push_back(std::move(e));
            view->display(orders);

            for (const auto &e : orders) {
                view->of_ << e.to_json().dump() << "\n";
            }

            // Debug
            std::map<int, int> counter;
            for (std::size_t i = 0; i < orders.size(); ++i) {
                auto &e = orders[i];
                counter[e.args[0]]++;
                // MESS_LOG("order#{0}: op={1}, args={2}", i, (int)e.code, e.args);
            }
            MESS_LOG("render-orders-summary={0}", counter);
        }

        return "";
    }

    ViewComponentConfig config_;
    std::list<WebSocketChannelPtr> subscriber_channels_;
    std::ofstream of_;
};

void *mess_view_ctx = nullptr;
void exit() {
    websocket_server_stop((hv::WebSocketServer*)mess_view_ctx);
}

int main(int argc, char **argv) {
    if (argc != 2) return -1;
    auto path = std::filesystem::path(__FILE__);
    path = path.parent_path();

    ViewComponentConfig config;

    try {
        xpack::json::decode(argv[1], config);    
    } catch (const std::exception&e) {
        MESS_ERR("config={0}\nxpack.err={1}", argv[1], e.what());
    }

    // Create ViewComponent
    ViewComponent view(config);

    // Register WebSocket/HTTP service
    hv::HttpService http;
    http.document_root = (path / "../Assets").string();

    WebSocketService ws;

    ws.onopen = [&view, config](const WebSocketChannelPtr& channel, const std::string& url) {
        MESS_LOG("New WebSocket connection(from=\"{0}\", with url=\"{1}\")",
            channel->peeraddr(), url);
        auto iter = view.add_subscriber_channel(channel);
        *channel->newContext<ViewComponent::SubscriberIter>() = iter;
        
        std::vector<ViewComponentConfig::Resource> ress = {
            config.backrgound_img,
            config.covered_grid_img,
            config.nonblock_grid_img,
            config.block_grid_img,
            config.car_img,
            config.future_route_img
        };

        // Slow
        Json resq_obj = Json::object();
        resq_obj["resources"] = Json::parse(xpack::json::encode(ress));
        channel->send(make_response_json_data(0, resq_obj).dump());
    };

    ws.onmessage = [](const WebSocketChannelPtr& channel, const std::string& msg) {
        // Pass
    };

    ws.onclose = [&view](const WebSocketChannelPtr& channel) {
        MESS_LOG("Close WebSocket connection(from=\"{0}\")",
            channel->peeraddr());
        auto iter = *channel->getContext<ViewComponent::SubscriberIter>();
        view.remove_subscriber_channel(iter);
        channel->deleteContext<ViewComponent::SubscriberIter>();
    };

    // WS Server
    websocket_server_t ws_server;
    ws_server.port = config.ws_url.port;
    MESS_LOG("port={0}", ws_server.port);
    ws_server.service = &http;
    ws_server.ws = &ws;
    
    // Init Board
    Board::get_instance()->init(config.auth_token, config.redis_board_ip, config.redis_board_port);    

    websocket_server_run(&ws_server, 0);
    mess_view_ctx = &ws_server;

    std::atexit(exit);

    // Run component
    view.run();

    return 0;
}
