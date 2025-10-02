#include "logger.hpp"
#include <ctime>
#include <iomanip>
#include <sstream>

namespace redite {

    static std::string now_iso8601() {
        using namespace std::chrono;
        const auto tp = system_clock::now();
        auto tt = system_clock::to_time_t(tp);
        const auto ms = duration_cast<milliseconds>(tp.time_since_epoch()) % 1000;
        std::tm tm{};
    #if defined(_WIN32)
        localtime_s(&tm, &tt);
    #else
        localtime_r(&tt, &tm);
    #endif
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S") << '.' << std::setw(3) << std::setfill('0') << ms.count();
        return oss.str();
    }

    const char* to_cstr(LogLevel lvl) {
        switch (lvl) {
            case LogLevel::TRACE: return "TRACE";
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO:  return "INFO";
            case LogLevel::WARN:  return "WARN";
            case LogLevel::ERROR: return "ERROR";
        }
        return "INFO";
    }

    Logger::Logger(const std::string& file_path, const LogLevel level, const bool also_stderr)
    : to_stderr_(also_stderr) {
        level_.store(level, std::memory_order_relaxed);
        if (!file_path.empty()) {
            fp_ = std::fopen(file_path.c_str(), "a");
            if (!fp_) {
                to_stderr_ = true; // fallback
            }
        }
    }

    Logger::~Logger() {
        flush();
        if (fp_) std::fclose(fp_);
    }

    void Logger::init(const std::string& file_path, const LogLevel level, const bool also_stderr) {
        // Meyers singleton gets constructed on first use
        (void) new (&instance()) Logger(file_path, level, also_stderr);
    }

    Logger& Logger::instance() {
        static Logger inst("", LogLevel::INFO, true);
        return inst;
    }

    void Logger::set_level(const LogLevel lvl) { instance().level_.store(lvl, std::memory_order_relaxed); }
    LogLevel Logger::level() { return instance().level_.load(std::memory_order_relaxed); }

    void Logger::log(const LogLevel lvl, const std::string_view msg, const std::initializer_list<Field> fields) {
        if (lvl < level()) return;
        instance().write_line(lvl, msg, fields);
    }

    void Logger::write_line(const LogLevel lvl, const std::string_view msg, const std::initializer_list<Field> fields) {
        std::lock_guard lock(mtx_);
        std::ostringstream line;
        line << now_iso8601() << " [" << to_cstr(lvl) << "] " << msg;
        for (auto& f : fields) {
            line << " " << f.key << "=";
            // basic escaping for spaces
            bool needs_quote = f.val.find_first_of(" \t\n") != std::string::npos;
            if (needs_quote) line << '"';
            line << f.val;
            if (needs_quote) line << '"';
        }
        line << "\n";
        auto s = line.str();
        if (fp_) std::fwrite(s.data(), 1, s.size(), fp_);
        if (to_stderr_) std::fwrite(s.data(), 1, s.size(), stderr);
    }

    void Logger::flush() {
        Logger& inst = instance();  // get the singleton
        std::lock_guard lock(inst.mtx_);
        if (inst.fp_) std::fflush(inst.fp_);
        std::fflush(stderr);
    }

}