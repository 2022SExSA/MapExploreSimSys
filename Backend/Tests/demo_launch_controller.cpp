#include <fstream>
#include <iostream>
#include <filesystem>

#include "utils.h"

namespace fs = std::filesystem;
using namespace pg::messbase;

int main() {
    
    auto path = fs::path(__FILE__);
    path = path.parent_path();
    path /= "assets/simple-config.json";
    std::ifstream ifs(path.string());
    std::string config_json;
    ifs >> config_json;

    launch_component("/home/pgzxb/Documents/DevWorkspace/2022SACourseWorkspace/MapExploreSimSys/Backend/Build/Components/Controller", config_json);

    return 0;
}
