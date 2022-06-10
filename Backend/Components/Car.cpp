#include "utils.h"
#include "Board.h"
#include "run_component.h"

using namespace pg::messbase;

struct CarComponentConfig : public ComponentConfig {
    int light_r{0};
};

class CarComponent {
public:
    CarComponent(const CarComponentConfig &config) : config_(config) {

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

        if (op.value().op == "go-next") {
            auto *board = Board::get_instance();
            auto pos = board->get_next_routing_position(car->config_.name);
            if (!pos.has_value()) {
                MESS_LOG("Empty {0}", pgfmt::format(ROUTLIST_NAME_FROMAT, car->config_.name));
            } else {
                int x = pos.value().x;
                int y = pos.value().y;
                MESS_LOG("Update Car@{0} to ({1}, {2})", car->config_.name, x, y);

                // Update pos of this car
                auto ok = board->set_position_of_car(car->config_.name, {x, y});
                MESS_ERR_IF(!ok, "Car {0} move to failed", car->config_.name);
                
                // Light grid on map
                int start_x = std::max(x - car->config_.light_r, 0);
                int start_y = std::max(y - car->config_.light_r, 0);
                int end_x = start_x + car->config_.light_r;
                int end_y = start_y + car->config_.light_r;
                for (int c = start_x; c <= end_x; ++c) {
                    for (int r = start_y; r <= end_y; ++r) {
                        board->set_grid_of_map(r, c, 1);
                    }
                }
            }
        }
        return "";
    }

    CarComponentConfig config_;
};

int main(int argc, char **argv) {
    Board::get_instance()->init("T", "127.0.0.1", 6379);

    CarComponentConfig config;
    
    config.host = "192.168.111.1";
    config.port = 5672;
    config.username = "pgzxb";
    config.password = "pgzxb";
    config.name = Board::get_instance()->get_auth_token() + "_Car001";

    config.light_r = 1;

    CarComponent(config).run();

    return 0;
}