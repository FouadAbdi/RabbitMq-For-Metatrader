#pragma once

#include <string>

namespace rmq4mt {

std::string wide_to_utf8(const wchar_t* wide);
std::string ansi_to_utf8(const char* text);
std::string format_wsa_error(const std::string& context, int code);
bool copy_utf8_to_mql_string(char* buffer, int buffer_size, const std::string& text);
bool copy_utf8_to_wide_string(wchar_t* buffer, int buffer_size, const std::string& text);

}  // namespace rmq4mt
