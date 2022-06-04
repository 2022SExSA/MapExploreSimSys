#include "run_component.h"

MESSBASE_NAMESPACE_START

class Component {
public:
    Component(
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

Component::Component(
    const std::string &host,
    short port,
    const std::string &username,
    const std::string &password,
    const std::string &name) {
    AmqpClient::Channel::OpenOpts opts;
    opts.host = host;
    opts.port = port;
    opts.auth = AmqpClient::Channel::OpenOpts::BasicAuth{username, password};
    channel_ = AmqpClient::Channel::Open(opts);
    channel_->DeclareQueue(name + ".input", false, true, false, false); // input
    channel_->DeclareQueue(name + ".output", false, true, false, false); // output

    channel_->DeclareExchange(name + ".output");
    channel_->BindQueue(name + ".output", name + ".output");
    consumer_tag = channel_->BasicConsume(name + ".input");
    name_ = name;
}

void Component::run() {
    while (true) {
        auto input = get_msg();
        auto output = process_input(input);
        if (!output.empty()) send_msg(output);
    }
}

std::string Component::get_msg() {
    return channel_
        ->BasicConsumeMessage(consumer_tag)
        ->Message()->Body();
}

void Component::send_msg(const std::string &msg) {
    channel_->BasicPublish(name_ + ".output", "", AmqpClient::BasicMessage::Create(msg));
}

void run_component(const ComponentConfig &config, const MsgProcessingCallback & msg_proc_func) {
    class ComponentWithCallback : public Component {
    public:
        ComponentWithCallback(
            const std::string &host,
            short port,
            const std::string &username,
            const std::string &password,
            const std::string &name,
            const MsgProcessingCallback & msg_proc_func)
        : Component(host, port, username, password, name), msg_proc_func_(msg_proc_func) {
        }
    protected:
        std::string process_input(const std::string &msg) override {
            PGZXB_DEBUG_ASSERT(msg_proc_func_);
            return msg_proc_func_(msg);
        }
    private:
        const MsgProcessingCallback &msg_proc_func_{nullptr};
    };

    ComponentWithCallback(
        config.host,
        config.port,
        config.username,
        config.password,
        config.name,
        msg_proc_func)
    .run();
}

MESSBASE_NAMESPACE_END
