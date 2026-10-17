#include "logging.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

namespace {
// Formatting codes
const std::string_view reset = "\033[m";
const std::string_view bold = "\033[1m";
const std::string_view dark = "\033[2m";
const std::string_view underline = "\033[4m";
const std::string_view blink = "\033[5m";
const std::string_view reverse = "\033[7m";
const std::string_view concealed = "\033[8m";
const std::string_view clear_line = "\033[K";

// Foreground colors
const std::string_view black = "\033[30m";
const std::string_view red = "\033[31m";
const std::string_view green = "\033[32m";
const std::string_view yellow = "\033[33m";
const std::string_view blue = "\033[34m";
const std::string_view magenta = "\033[35m";
const std::string_view cyan = "\033[36m";
const std::string_view white = "\033[37m";

/// Background colors
const std::string_view on_black = "\033[40m";
const std::string_view on_red = "\033[41m";
const std::string_view on_green = "\033[42m";
const std::string_view on_yellow = "\033[43m";
const std::string_view on_blue = "\033[44m";
const std::string_view on_magenta = "\033[45m";
const std::string_view on_cyan = "\033[46m";
const std::string_view on_white = "\033[47m";

/// Bold colors
const std::string_view yellow_bold = "\033[33m\033[1m";
const std::string_view red_bold = "\033[31m\033[1m";
const std::string_view bold_on_red = "\033[1m\033[41m";
}  // namespace

static std::string GetCurrentTimeAndDate() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&in_time_t));
    return buffer;
}

static std::string GetCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    char buffer[9];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", localtime(&in_time_t));
    return buffer;
}
