#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <cstdarg>
#include <vector>

namespace logger {

    enum class Level {
        Info,
        Warn,
        Error,
        Debug
    };

    // ---------------- Color Utilities ----------------
    inline std::string rgb_fg(int r, int g, int b) {
        return "\x1b[38;2;" + std::to_string(r) + ";" +
            std::to_string(g) + ";" + std::to_string(b) + "m";
    }

    inline const std::string reset_color = "\x1b[0m";

    inline std::string label_for(Level level) {
        switch (level) {
        case Level::Info:  return "INFO";
        case Level::Warn:  return "WARN";
        case Level::Error: return "ERROR";
        case Level::Debug: return "DEBUG";
        default:           return "";
        }
    }

    inline std::string label_color(Level level) {
        switch (level) {
        case Level::Info:  return rgb_fg(128, 128, 128);
        case Level::Warn:  return rgb_fg(255, 255, 0);
        case Level::Error: return rgb_fg(255, 0, 0);
        case Level::Debug: return rgb_fg(255, 105, 180);
        default:           return rgb_fg(255, 255, 255);
        }
    }

    inline std::string message_color(Level level) {
        switch (level) {
        case Level::Info:  return rgb_fg(128, 128, 128);
        case Level::Warn:  return rgb_fg(255, 255, 0);
        case Level::Error: return rgb_fg(255, 0, 0);
        case Level::Debug: return rgb_fg(255, 105, 180);
        default:           return rgb_fg(255, 255, 255);
        }
    }

    // ---------------- Formatting Helper ----------------
    inline std::string format(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        int size = std::vsnprintf(nullptr, 0, fmt, args);
        va_end(args);

        std::vector<char> buf(size + 1);
        va_start(args, fmt);
        std::vsnprintf(buf.data(), buf.size(), fmt, args);
        va_end(args);

        return std::string(buf.data(), buf.size() - 1);
    }

    // ---------------- Logging Implementation ----------------
    template<typename... Args>
    inline void write(Level level, const char* fmt, Args&&... args) {
        std::string message = format(fmt, std::forward<Args>(args)...);

        std::cout << rgb_fg(255, 255, 255) << "[";
        std::cout << label_color(level) << label_for(level);
        std::cout << rgb_fg(255, 255, 255) << "] ";
        std::cout << message_color(level) << message << reset_color << std::endl;
    }

    // ---------------- Convenience Functions ----------------
    template<typename... Args>
    inline void info(const char* fmt, Args&&... args) { write(Level::Info, fmt, std::forward<Args>(args)...); }

    template<typename... Args>
    inline void warn(const char* fmt, Args&&... args) { write(Level::Warn, fmt, std::forward<Args>(args)...); }

    template<typename... Args>
    inline void error(const char* fmt, Args&&... args) { write(Level::Error, fmt, std::forward<Args>(args)...); }

    template<typename... Args>
    inline void debug(const char* fmt, Args&&... args) { write(Level::Debug, fmt, std::forward<Args>(args)...); }

}
