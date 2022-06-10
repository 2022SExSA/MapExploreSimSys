#ifndef MESSBASE_RUNCOMPONENT_H
#define MESSBASE_RUNCOMPONENT_H

#include "fwd.h"
#include "config.h"
#include "SimpleAmqpClient/BasicMessage.h"
#include "SimpleAmqpClient/Channel.h"
#include "SimpleAmqpClient/SimpleAmqpClient.h"

MESSBASE_NAMESPACE_START

using MsgProcessingCallback = std::function<std::string(const std::string&)>;

void run_component(const ComponentConfig &config, const MsgProcessingCallback & msg_proc_func);

MESSBASE_NAMESPACE_END
#endif // !MESSBASE_RUNCOMPONENT_H