#pragma once
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace redite {

enum class LogLevel : uint8_t { TRACE=0, DEBUG, INFO, WARN, ERROR };

struct Field {
    std::string key, val;
};

class Logger {
public:
    static void init(const std::string& file_path = "", LogLevel level = LogLevel::INFO, bool also_stderr = true);
    static void set_level(LogLevel lvl);
    static LogLevel level();

    static void log(LogLevel lvl, std::string_view msg, std::initializer_list<Field> fields = {});

    static void flush();

private:
    Logger(const std::string& file_path, LogLevel level, bool also_stderr);
    ~Logger();
    void write_line(LogLevel lvl, std::string_view msg, std::initializer_list<Field> fields);

    static Logger& instance();

    std::mutex mtx_;
    FILE* fp_{nullptr};
    bool to_stderr_{true};
    std::atomic<LogLevel> level_{LogLevel::INFO};

};

const char* to_cstr(LogLevel lvl);

#define RLOG_TRACE(msg, ...) do { if (redite::Logger::level() <= redite::LogLevel::TRACE) redite::Logger::log(redite::LogLevel::TRACE, (msg), ##__VA_ARGS__); } while(0)
#define RLOG_DEBUG(msg, ...) do { if (redite::Logger::level() <= redite::LogLevel::DEBUG) redite::Logger::log(redite::LogLevel::DEBUG, (msg), ##__VA_ARGS__); } while(0)
#define RLOG_INFO(msg, ...)  do { if (redite::Logger::level() <= redite::LogLevel::INFO ) redite::Logger::log(redite::LogLevel::INFO,  (msg), ##__VA_ARGS__); } while(0)
#define RLOG_WARN(msg, ...)  do { if (redite::Logger::level() <= redite::LogLevel::WARN ) redite::Logger::log(redite::LogLevel::WARN,  (msg), ##__VA_ARGS__); } while(0)
#define RLOG_ERROR(msg, ...) do { if (redite::Logger::level() <= redite::LogLevel::ERROR) redite::Logger::log(redite::LogLevel::ERROR, (msg), ##__VA_ARGS__); } while(0)

}