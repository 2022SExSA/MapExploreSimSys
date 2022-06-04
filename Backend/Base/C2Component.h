#ifndef PGC2CALCU_C2COMPONENT_H
#define PGC2CALCU_C2COMPONENT_H

#include "fwd.h"
#include "SimpleAmqpClient/BasicMessage.h"
#include "SimpleAmqpClient/Channel.h"
#include "SimpleAmqpClient/SimpleAmqpClient.h"

MESSBASE_NAMESPACE_START
class C2Component {
public:
    C2Component(
      const std::string &host,
      short port,
      const std::string &username,
      const std::string &password,
      const std::string &name);

    void run();
protected:
    virtual std::string process_input(const std::string &msg) = 0;
private:
    std::string get_msg();
    void send_msg(const std::string &msg);

    AmqpClient::Channel::ptr_t channel_;
    std::string name_;
    std::string consumer_tag;
};

MESSBASE_NAMESPACE_END
#endif