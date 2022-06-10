#include <thread>

#include "config.h"
#include "utils.h"
#include "Board.h"
#include "Xpack/json.h"

using namespace pg::messbase;

class Controller {
public:
    Controller(const Config &config) : config_(config) {
        const auto &map = config_.map_config;
        auto &board = *Board::get_instance();
        board.set_map_size(config_.map_config.size.width, config.map_config.size.height);
        const auto [w, h] = map.size;
        for (int r = 0; r < h; ++r) {
            for (int c = 0; c < w; ++c) {
                board.set_grid_of_map(r, c, 0);
                board.set_grid_of_map_block(r, c, (bool)(map.map_code[r * w + c] - '0'));
            }
        }

        PGZXB_DEBUG_ASSERT(config_.navigator_components_config.size() > 0);

        for (const auto &car_config : config_.car_components_config) {
            launch_component("Car", xpack::json::encode(car_config));
        }
        for (const auto &navi_config : config_.navigator_components_config) {
            launch_component("Navigator", xpack::json::encode(navi_config));
        }
    }

    void run() {
        while (true) {
            const auto start = std::chrono::steady_clock::now();
            ///////////////////////////////////////////////////////
            //|________________Update Frame_____________________|//

            auto &board = *Board::get_instance();

            // Scan all cars rout-list & Collect car-ids whose rout-list is empty
            std::vector<std::string> car_ids;
            for (const auto &car : config_.car_components_config) {
                auto cnt = board.get_routlist_size(car.mq_name);
                if (cnt == 0) {
                    car_ids.push_back(car.mq_name);
                }
            }

            // Call navi ({auth_token} navi car1-id car2-id ...)
            const int car_count = (int)car_ids.size();
            const int navi_count = (int)navi_components.size();
            std::vector<Op> command_for_navigators(navi_count);
            for (
                int car_idx = 0, navi_idx = 0;
                car_idx < car_count; 
                ++car_idx, navi_idx = (navi_idx + 1) % navi_count
            ) {
                command_for_navigators[navi_idx].args.push_back(car_ids[car_idx]);
            }
            for (int i = 0; i < navi_count; ++i) {
                if (command_for_navigators[i].args.empty()) break;
                command_for_navigators[i].op = "navi";
                command_for_navigators[i].auth_token = config_.auth_token;
                navi_components[i].input(command_for_navigators[i].to_string());
            }            

            // For each car: update car ({auth_token} go-next)
            Op car_go_next_commmand;
            car_go_next_commmand.auth_token = config_.auth_token;
            car_go_next_commmand.op = "go-next";
            for (auto &car : car_components) {
                car.input(car_go_next_commmand.to_string());
            }
            
            //|________________Update Frame_____________________|//
            ///////////////////////////////////////////////////////
            using namespace std::chrono_literals;
            const auto need_delay = 1000ms / config_.fps;
            const auto true_delay = std::chrono::steady_clock::now() - start;
            if (true_delay < need_delay) {
                std::this_thread::sleep_for(need_delay - true_delay);
            }
        }
    }
private:
    Config config_;
    std::vector<C2Call> car_components;
    std::vector<C2Call> navi_components;
};

int main(int argc, char **argv) {
    std::setbuf(stdout, NULL);
    if (argc != 2) return -1;

    Config config;
    const char *config_json = argv[1];
    xpack::json::decode(config_json, config);
    
    // Init Board
    Board::get_instance()->init(config.auth_token, config.redis_board_ip, config.redis_board_port);

    // Create Controller & Run
    Controller(config).run();

    return 0;
}
