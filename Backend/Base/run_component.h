#ifndef MESSBASE_RUNCOMPONENT_H
#define MESSBASE_RUNCOMPONENT_H

#include "fwd.h"
#include "SimpleAmqpClient/BasicMessage.h"
#include "SimpleAmqpClient/Channel.h"
#include "SimpleAmqpClient/SimpleAmqpClient.h"

MESSBASE_NAMESPACE_START

using MsgProcessingCallback = std::function<std::string(const std::string&)>;

struct ComponentConfig {
    std::string host{"localhost"};
    short port{0};
    std::string username;
    std::string password;
    std::string name;
};

void run_component(const ComponentConfig &config, const MsgProcessingCallback & msg_proc_func);

MESSBASE_NAMESPACE_END
#endif // !MESSBASE_RUNCOMPONENT_H