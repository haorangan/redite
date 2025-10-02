#pragma once
#include <string>
#include <chrono>

namespace redite {

    enum class AofFsyncMode { Always, EverySec, No };

    struct AofWriter {
        explicit AofWriter(const std::string& path, AofFsyncMode mode = AofFsyncMode::EverySec);
        ~AofWriter();

        bool append(const std::string& bytes); // returns false on fatal write error
        void flush() const;

    private:
        int fd_ = -1;
        AofFsyncMode mode_;
        std::chrono::steady_clock::time_point last_sync_;

        bool write_all(const char* p, size_t n) const;
        void maybe_fsync();
    };

    // Exposed pointer you set in main *after* replay.
    extern AofWriter* g_aof;

} // namespace redi