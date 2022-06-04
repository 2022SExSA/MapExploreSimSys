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
        
        if (op.value().auth_token != Board::get_instance().get_auth_token()) {
            MESS_LOG("Auth Failed", 1);
            return "";
        }

        if (op.value().op == "go-next") {
            auto pos = Board::get_instance().get_next_routing_position(car->config_.name);
            if (!pos.has_value()) MESS_LOG("Empty {0}", pgfmt::format(ROUTLIST_NAME_FROMAT, car->config_.name));
            else {
                MESS_LOG("Update Car@{0} to ({1}, {2})", car->config_.name, pos.value().x, pos.value().y);
                // Update pos of this car
                // Light grid on map
            }
        }
        return "";
    }

    CarComponentConfig config_;
};

int main(int argc, char **argv) {
    Board::get_instance().init("T", "127.0.0.1", 6379);

    CarComponentConfig config;
    
    config.host = "192.168.111.1";
    config.port = 5672;
    config.username = "pgzxb";
    config.password = "pgzxb";
    config.name = "Car001";

    config.light_r = 0;

    CarComponent(config).run();

    return 0;
}