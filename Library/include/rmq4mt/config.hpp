#pragma once

#include <cstdint>
#include <string>

namespace rmq4mt {

struct Config {
    std::string host = "127.0.0.1";
    std::uint16_t port = 5672;
    std::string user = "rmq4mt";
    std::string pass = "rmq4mt";
    std::string vhost = "/";
    std::string exchange = "mt.events";
    std::string queue = "mt.events.queue";
};

}  // namespace rmq4mt
