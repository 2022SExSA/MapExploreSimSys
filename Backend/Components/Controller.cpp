#include <thread>

#include "config.h"
#include "utils.h"
#include "Board.h"
#include "Xpack/json.h"

// Linux headers
#include <sys/types.h>
#include <sys/wait.h>

MESS_LOG_MODULE("Component/Controller");

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
                board.set_grid_of_map(r, c, MAP_GRID_COVERED_FLAG);
                board.set_grid_of_map_block(r, c, (bool)(map.map_code[r * w + c] - '0'));
            }
        }

        PGZXB_DEBUG_ASSERT(config_.navigator_components_config.size() > 0);

        launch_component("/home/pgzxb/Documents/DevWorkspace/2022SACourseWorkspace/MapExploreSimSys/Backend/Build/Components/View", xpack::json::encode(config_.view_config));
        view_component = C2Call{config_.view_config};

        for (const auto &car_config : config_.car_components_config) {
            // FIXME: Bad smell: hardcode
            launch_component("/home/pgzxb/Documents/DevWorkspace/2022SACourseWorkspace/MapExploreSimSys/Backend/Build/Components/Car", xpack::json::encode(car_config));
            car_components.push_back(C2Call{car_config});
        }
        for (const auto &navi_config : config_.navigator_components_config) {
            // FIXME: Bad smell: hardcode
            launch_component("/home/pgzxb/Documents/DevWorkspace/2022SACourseWorkspace/MapExploreSimSys/Backend/Build/Components/Navigator", xpack::json::encode(navi_config));
            navi_components.push_back(C2Call{navi_config});
        }
    }

    void run() {
        int i = 0;
        while (true) {
            std::cerr << "RUNNING" << i++ << "\n";
            const auto start = std::chrono::steady_clock::now();
            ///////////////////////////////////////////////////////
            //|________________Update Frame_____________________|//

            auto &board = *Board::get_instance();

            // Scan all cars rout-list & Collect car-ids whose rout-list is empty
            std::vector<std::string> car_ids;
            for (const auto &car : config_.car_components_config) {
                auto cnt = board.get_routlist_size(car.mq_name);
                if (cnt <= 0) {
                    car_ids.push_back(car.mq_name);
                }
            }

            // Call navi ({auth_token} navi car1-id car2-id ...)
            const int car_count = (int)car_ids.size();
            const int navi_count = (int)navi_components.size();
            std::vector<Op> command_for_navigators(navi_count);
            for (
                int car_idx = 0, navi_idx = 0;
                car_idx < car_count && navi_idx < navi_count;
                ++car_idx, navi_idx = (navi_idx + 1) % navi_count
            ) {
                PGZXB_DEBUG_ASSERT(navi_count != 0);
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
            
            // Update View
            Op update_view_op;
            update_view_op.auth_token = config_.auth_token;
            update_view_op.op = "update";
            view_component.input(update_view_op.to_string());

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
    C2Call view_component;
    std::vector<C2Call> car_components;
    std::vector<C2Call> navi_components;
};

int main(int argc, char **argv) {
    std::setbuf(stdout, NULL);
    if (argc != 2) return -1;

    Config config;

    try {
        xpack::json::decode(argv[1], config);    
    } catch (const std::exception&e) {
        MESS_ERR("config={0}\nxpack.err={1}", argv[1], e.what());
    }
    
    // Init Board
    Board::get_instance()->init(config.auth_token, config.redis_board_ip, config.redis_board_port);

    // Create Controller & Run
    Controller(config).run();

    // Wait all children
    wait(nullptr);
    return 0;
}
