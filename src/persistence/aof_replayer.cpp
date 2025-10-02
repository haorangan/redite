#include "aof_replayer.hpp"
#include "resp_parser.hpp"
#include "dispatcher.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <vector>

namespace redite {

    bool replay_aof(const std::string& path, Storage& store, const Dispatcher& dispatcher) {
        const int fd = ::open(path.c_str(), O_RDONLY);
        if (fd < 0) return true; // no file → nothing to replay

        resp::RespParser parser;
        std::vector<char> buf(64 * 1024);

        for (;;) {
            const ssize_t n = read(fd, buf.data(), buf.size());
            if (n == 0) break;
            if (n < 0) {
                if (errno == EINTR) continue;
                close(fd);
                return false;
            }
            parser.feed(std::string_view(buf.data(), static_cast<size_t>(n)));

            Command cmd;
            while (parser.next(cmd)) {
                (void)dispatcher.dispatch(store, cmd);
            }
            if (auto err = parser.error()) {
                close(fd);
                return false; // corrupted tail, will probably ignore
            }
        }
        close(fd);
        return true;
    }

}