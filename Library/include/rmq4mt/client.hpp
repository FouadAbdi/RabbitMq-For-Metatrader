#pragma once

#include "rmq4mt/config.hpp"

#include <cstddef>
#include <string>

struct amqp_connection_state_t_;

namespace rmq4mt {

class Client {
public:
    explicit Client(Config config);
    ~Client();

    bool connect();
    void disconnect();
    bool is_connected() const;
    const std::string& last_error() const;

    bool publish(const char* body);
    int poll(char* buffer, std::size_t buffer_size);

private:
    bool setup_topology();
    bool check_reply(const char* step);

    Config config_;
    amqp_connection_state_t_* conn_ = nullptr;
    bool connected_ = false;
    bool consuming_ = false;
    std::string last_error_;
};

}  // namespace rmq4mt
