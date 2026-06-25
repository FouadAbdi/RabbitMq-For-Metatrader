#include "rmq4mt/error.hpp"

#include <cstring>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>

#include <rabbitmq-c/amqp.h>

namespace rmq4mt {

std::string wide_to_utf8(const wchar_t* wide) {
    if (wide == nullptr || wide[0] == L'\0') {
        return {};
    }
    const int size = WideCharToMultiByte(CP_UTF8, 0, wide, -1, nullptr, 0, nullptr, nullptr);
    if (size <= 1) {
        return {};
    }
    std::string out(static_cast<std::size_t>(size - 1), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide, -1, out.data(), size, nullptr, nullptr);
    return out;
}

std::string ansi_to_utf8(const char* text) {
    if (text == nullptr || text[0] == '\0') {
        return {};
    }
    return std::string(text);
}

bool copy_utf8_to_wide_string(wchar_t* buffer, int buffer_size, const std::string& text) {
    if (buffer == nullptr || buffer_size <= 0) {
        return false;
    }
    const int written = MultiByteToWideChar(
        CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), buffer, buffer_size);
    if (written <= 0 || written >= buffer_size) {
        buffer[0] = L'\0';
        return false;
    }
    buffer[written] = L'\0';
    return true;
}

bool copy_utf8_to_mql_string(char* buffer, int buffer_size, const std::string& text) {
    if (buffer == nullptr || buffer_size <= 0) {
        return false;
    }
    if (static_cast<int>(text.size()) + 1 > buffer_size) {
        buffer[0] = '\0';
        return false;
    }
    std::memcpy(buffer, text.c_str(), text.size() + 1);
    return true;
}

std::string format_wsa_error(const std::string& context, int code) {
    if (code == AMQP_STATUS_CONNECTION_CLOSED) {
        return context + ": connection closed";
    }
    if (code == AMQP_STATUS_SOCKET_ERROR) {
        return context + ": socket error";
    }
    if (code == AMQP_STATUS_TIMEOUT) {
        return context + ": timeout";
    }
    if (code == AMQP_STATUS_TCP_ERROR) {
        return context + ": tcp error";
    }
    return context + " (code " + std::to_string(code) + ")";
}

}  // namespace rmq4mt
