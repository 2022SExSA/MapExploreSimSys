#include "Board.h"
#include "utils.h"
#include "config.h"
#include "run_component.h"
#include "navigator_plugin_utils.h"

MESS_LOG_MODULE("Component/Navigator");

using namespace pg::messbase;

// 2 algorithms(Combined as a dll):
//      1. Dest-Position-Selecting | int pg_mess_navi_select_dest_pos_v1(MESSNP_Position *dest_pos, const MESSNP_Position *src_pos, MESSNP_MapInfoGetter *map_info_getter);
//      2. Routing                 | int pg_mess_navi_routing_v1(MESSNP_PositionArray *rout_pos_array, const MESSNP_Position *src_pos, const MESSNP_Position *dest_pos);

class NaviComponent {
public:
    NaviComponent(const NaviComponentConfig &config) : config_(config), plugin_(config.plugin_path) {
    }

    void run() {
        run_component(config_, std::bind(msg_proc, this, std::placeholders::_1));
    }
private:
    static std::string msg_proc(NaviComponent *navi, const std::string &msg) {
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
            exit(0);
        }

        if (op->op == "navi") { // navi <car1-id> <car2-id> ...
            auto *board = Board::get_instance();
            MESS_LOG("navi for {0} cars", op->args.size());
            for (const auto &car_id : op->args) {
                MESS_LOG("Navi for {0}", car_id);
                auto pos = board->get_current_position_of_car(car_id);
                MESS_ERR_IF(!pos.has_value(), "Get position of {0} failed", car_id);
    
                MESSNP_Position src, dest;
                MESSNP_PositionArray dest_pos_array;
                MESSNP_MapInfoGetter *map_info_getter = naviplugin::create_default_map_info_getter();

                std::tie(src.c, src.r) = std::tie(pos.value().x, pos.value().y);
                auto pos_selector_func = navi->plugin_.find_function<MESSNP_NaviSelectDestPosFuncV1>("pg_mess_navi_select_dest_pos_v1");
                auto routing_func = navi->plugin_.find_function<MESSNP_NaviRoutingV1>("pg_mess_navi_routing_v1");
                PGZXB_DEBUG_ASSERT(pos_selector_func);
                PGZXB_DEBUG_ASSERT(routing_func);

                [[maybe_unused]] int err = pos_selector_func(&dest, &src, map_info_getter);
                MESS_ERR_IF(err != 0, "", 1);
                MESS_LOG("Navi for {0}, select dest: ({1}, {2})", car_id, dest.r, dest.c);
                err = routing_func(&dest_pos_array, &src, &dest, map_info_getter);
                MESS_ERR_IF(err != 0, "", 1);
                MESS_LOG("Navi for {0}, routing: {1}",
                    car_id,
                    std::vector<std::pair<int, int>>( // !!!Unsafe!!!
                        (std::pair<int, int>*)dest_pos_array.array,
                        (std::pair<int, int>*)dest_pos_array.array + dest_pos_array.size
                    ));
                if (board->get_routlist_size(car_id) == 0) {
                    for (int i = 0; i < dest_pos_array.size; ++i) {
                        auto &pos = dest_pos_array.array[i];
                        [[maybe_unused]] auto ret = board->add_position_to_routlist(car_id, {pos.c, pos.r});
                        MESS_ERR_IF(ret == -1, "", 1);
                    }
                }
                dest_pos_array.destroy_array(dest_pos_array.ctx);
            }
        }

        return "";
    }

    NaviComponentConfig config_;
    NaviPlugin plugin_;
};

int main(int argc, char **argv) {
    if (argc != 2) return -1;

    NaviComponentConfig config;

    try {
        xpack::json::decode(argv[1], config);    
    } catch (const std::exception&e) {
        MESS_ERR("config={0}\nxpack.err={1}", argv[1], e.what());
    }

    Board::get_instance()->init(config.auth_token, config.redis_board_ip, config.redis_board_port);    

    NaviComponent(config).run();

    return 0;
}
