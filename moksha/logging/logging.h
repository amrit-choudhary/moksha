/**
 * Logging.
 */

#pragma once

#include <cstdint>
#include <iostream>
#include <sstream>
#include <string_view>

namespace ME {

enum class LogLevel : uint8_t {
    Info = 0x0,
    Warning = 0x1,
    Error = 0x2,
    Debug = 0x3,
    Success = 0x4
};

// Added padding spaces at the end of the log level string for alignment in the console output.
inline std::string_view LogLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Info:
            return "[INFO]   ";
        case LogLevel::Warning:
            return "[WARNING]";
        case LogLevel::Error:
            return "[ERROR]  ";
        case LogLevel::Debug:
            return "[DEBUG]  ";
        case LogLevel::Success:
            return "[SUCCESS]";
        default:
            return "[DEFAULT]";
    }
}

// Color is used for terminal output and uses ANSI escape codes.
inline std::string_view LogLevelToColor(LogLevel level) {
    switch (level) {
        case LogLevel::Info:
            return "\033[37m";  // White
        case LogLevel::Warning:
            return "\033[34m";  // Blue
        case LogLevel::Error:
            return "\033[31m";  // Red
        case LogLevel::Debug:
            return "\033[33m";  // Yellow
        case LogLevel::Success:
            return "\033[32m";  // Green
        default:
            return "\033[37m";  // White
    }
}

// Reset to the default terminal color = white.
inline std::string_view DefaultLogColor() {
    return "\033[37m";  // White
}

template <typename... Args>
void LogImpl(LogLevel level, const Args&... args) {
    std::ostringstream oss;
    oss << LogLevelToColor(level);
    oss << LogLevelToString(level) << ' ';
    (oss << ... << args);
    oss << DefaultLogColor();
    oss << '\n';
    std::cout << oss.str();
}

template <typename... Args>
void LogInfo(const Args&... args) {
    LogImpl(LogLevel::Info, args...);
}

template <typename... Args>
void LogWarning(const Args&... args) {
    LogImpl(LogLevel::Warning, args...);
}

template <typename... Args>
void LogError(const Args&... args) {
    LogImpl(LogLevel::Error, args...);
}

template <typename... Args>
void LogDebug(const Args&... args) {
    LogImpl(LogLevel::Debug, args...);
}

template <typename... Args>
void LogSuccess(const Args&... args) {
    LogImpl(LogLevel::Success, args...);
}

template <typename... Args>
void Log(const Args&... args) {
    LogImpl(LogLevel::Info, args...);
}

}  // namespace ME
