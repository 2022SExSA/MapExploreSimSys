#include "C2Component.h"

MESSBASE_NAMESPACE_START

C2Component::C2Component(
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

void C2Component::run() {
    while (true) {
        auto input = get_msg();
        auto output = process_input(input);
        send_msg(output);
    }
}

std::string C2Component::get_msg() {
    return channel_
        ->BasicConsumeMessage(consumer_tag)
        ->Message()->Body();
}

void C2Component::send_msg(const std::string &msg) {
    channel_->BasicPublish(name_ + ".output", "", AmqpClient::BasicMessage::Create(msg));
}

MESSBASE_NAMESPACE_END
