#include "utils.h"
#include "config.h"
#include "Board.h"
#include "run_component.h"

MESS_LOG_MODULE("Component/Car");

using namespace pg::messbase;

class CarComponent {
public:
    CarComponent(const CarComponentConfig &config) : config_(config) {
        Board::get_instance()->set_position_of_car(config.mq_name, {config.init_pos_c, config.init_pos_r});
    }

    void run() {
        run_component(config_, std::bind(msg_proc, this, std::placeholders::_1));
    }
private:
    static std::string msg_proc(CarComponent *car, const std::string &msg) {
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

        if (op.value().op == "go-next") {
            auto *board = Board::get_instance();
            auto pos = board->get_next_routing_position(car->config_.mq_name);
            if (pos.has_value()) {
                if (board->try_lock_grid(car->config_.mq_name, pos->y, pos->x)) {
                    int x = pos.value().x;
                    int y = pos.value().y;
                    if (x < 0 || y < 0) return "";
                    // Update pos of this car
                    [[maybe_unused]] auto ok = board->set_position_of_car(car->config_.mq_name, {x, y});
                    board->unlock_grid(car->config_.mq_name, car->last_r, car->last_c);
                    car->last_r = y;
                    car->last_c = x;
                    MESS_ERR_IF(!ok, "Car {0} move to failed", car->config_.mq_name);
                    // Light grid on map
                    int start_x = std::max(x - car->config_.light_r, 0);
                    int start_y = std::max(y - car->config_.light_r, 0);
                    int end_x = x + car->config_.light_r;
                    int end_y = y + car->config_.light_r;
                    for (int c = start_x; c <= end_x; ++c) {
                        for (int r = start_y; r <= end_y; ++r) {
                            board->set_grid_of_map(r, c, MAP_GRID_RAW_FLAG);
                        }
                    }
                    car->last_op_ = op.value();
                } else {
                    board->clear_routlist(car->config_.mq_name); // Clear routlist
                }
            }
        }
        return "";
    }

    CarComponentConfig config_;
    Op last_op_;
    int last_r{-1}, last_c{-1};
};

int main(int argc, char **argv) {
    if (argc != 2) return -1;

    CarComponentConfig config;

    try {
        xpack::json::decode(argv[1], config);    
    } catch (const std::exception&e) {
        MESS_ERR("config={0}\nxpack.err={1}", argv[1], e.what());
    }

    Board::get_instance()->init(config.auth_token, config.redis_board_ip, config.redis_board_port);    

    CarComponent(config).run();

    return 0;
}