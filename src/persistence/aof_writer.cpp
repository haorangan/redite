#include "aof_writer.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cerrno>

namespace redite {

    AofWriter* g_aof = nullptr;

    AofWriter::AofWriter(const std::string& path, const AofFsyncMode mode)
    : mode_(mode), last_sync_(std::chrono::steady_clock::now()) {
        fd_ = ::open(path.c_str(), O_CREAT | O_APPEND | O_WRONLY, 0644);
        // You might want to throw on failure; for MVP, fd_ < 0 means disabled.
    }

    AofWriter::~AofWriter() {
        flush();
        if (fd_ >= 0) ::close(fd_);
    }

    bool AofWriter::write_all(const char* p, size_t n) const {
        while (n > 0) {
            ssize_t w = ::write(fd_, p, n);
            if (w < 0) {
                if (errno == EINTR) continue;
                return false;
            }
            p += static_cast<size_t>(w);
            n -= static_cast<size_t>(w);
        }
        return true;
    }

    void AofWriter::maybe_fsync() {
        switch (mode_) {
            case AofFsyncMode::Always:
                ::fsync(fd_);
            break;
            case AofFsyncMode::EverySec: {
                auto now = std::chrono::steady_clock::now();
                if (now - last_sync_ >= std::chrono::seconds(1)) {
                    ::fsync(fd_);
                    last_sync_ = now;
                }
                break;
            }
            case AofFsyncMode::No:
                break;
        }
    }

    bool AofWriter::append(const std::string& bytes) {
        if (fd_ < 0) return false;
        if (!write_all(bytes.data(), bytes.size())) return false;
        maybe_fsync();
        return true;
    }

    void AofWriter::flush() const {
        if (fd_ >= 0 && mode_ != AofFsyncMode::No) ::fsync(fd_);
    }

} // namespace redite