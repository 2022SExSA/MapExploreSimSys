#include <fstream>
#include <iostream>
#include <filesystem>

#include "ProjectConfig.h"
#include "utils.h"

// Linux headers
#include <sys/types.h>
#include <sys/wait.h>

namespace fs = std::filesystem;
using namespace pg::messbase;

int main() {
    auto path = fs::path(__FILE__);
    path = path.parent_path();
    path /= "assets/simple-config.json";
    std::ifstream ifs(path.string());
    std::string config_json;
    ifs >> config_json;

    // FIXME: Bad smell, refused hardcode
    launch_component(ProjectConfig::get_instance().component_exe_root_path + "Controller", config_json);

    // Wait all children
    wait(nullptr);
    return 0;
}
