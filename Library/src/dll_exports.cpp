#include "rmq4mt/client.hpp"
#include "rmq4mt/error.hpp"

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>

namespace {

rmq4mt::Client* g_client = nullptr;
std::string g_last_error;
bool g_winsock_ready = false;

bool ensure_winsock() {
    if (g_winsock_ready) {
        return true;
    }
    WSADATA wsa{};
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        return false;
    }
    g_winsock_ready = true;
    return true;
}

void set_last_error(std::string message) {
    g_last_error = std::move(message);
}

void reset_client() {
    delete g_client;
    g_client = nullptr;
}

}  // namespace

extern "C" {

__declspec(dllexport) int __stdcall RmqConnect(const wchar_t* host, int port, const wchar_t* user, const wchar_t* pass) {
    reset_client();
    set_last_error({});

    if (!ensure_winsock()) {
        set_last_error("WSAStartup failed");
        return 0;
    }

    rmq4mt::Config config;
    config.host = rmq4mt::wide_to_utf8(host);
    if (config.host.empty()) {
        config.host = "127.0.0.1";
    }
    if (port > 0 && port <= 65535) {
        config.port = static_cast<std::uint16_t>(port);
    }
    const std::string user_str = rmq4mt::wide_to_utf8(user);
    if (!user_str.empty()) {
        config.user = user_str;
    }
    const std::string pass_str = rmq4mt::wide_to_utf8(pass);
    if (!pass_str.empty()) {
        config.pass = pass_str;
    }

    g_client = new rmq4mt::Client(config);
    if (!g_client->connect()) {
        set_last_error(g_client->last_error());
        reset_client();
        return 0;
    }
    return 1;
}

__declspec(dllexport) void __stdcall RmqDisconnect() {
    reset_client();
    set_last_error({});
}

__declspec(dllexport) int __stdcall RmqPublish(const wchar_t* body) {
    if (g_client == nullptr) {
        set_last_error("not connected");
        return 0;
    }
    const std::string payload = rmq4mt::wide_to_utf8(body);
    if (!g_client->publish(payload.c_str())) {
        set_last_error(g_client->last_error());
        return 0;
    }
    return 1;
}

__declspec(dllexport) int __stdcall RmqPoll(wchar_t* buffer, int buffer_size) {
    if (g_client == nullptr || buffer == nullptr || buffer_size <= 0) {
        return 0;
    }

    char narrow[4096];
    const int received = g_client->poll(narrow, sizeof(narrow));
    if (received <= 0) {
        return 0;
    }

    if (!rmq4mt::copy_utf8_to_wide_string(buffer, buffer_size, narrow)) {
        return 0;
    }
    return received;
}

__declspec(dllexport) int __stdcall RmqGetLastError(wchar_t* buffer, int buffer_size) {
    return rmq4mt::copy_utf8_to_wide_string(buffer, buffer_size, g_last_error) ? 1 : 0;
}

}
