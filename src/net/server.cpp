#include "server.hpp"

#include <array>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>         // close
#include <csignal>         // SIGPIPE

#include "resp_encoder.hpp"
#include "resp_parser.hpp"

namespace redite {

// Small helper: ignore SIGPIPE so send() returns EPIPE instead of killing the process.
static void ignore_sigpipe_once() {
    static bool done = false;
    if (!done) {
        ::signal(SIGPIPE, SIG_IGN);
        done = true;
    }
}

void Server::run(Storage& store, const Dispatcher& dispatcher) const {
    ignore_sigpipe_once();

    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) throw std::system_error(errno, std::generic_category(), "socket");

    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_);

    if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        int e = errno; ::close(fd);
        throw std::system_error(e, std::generic_category(), "bind");
    }

    if (listen(fd, 128) < 0) {
        int e = errno; close(fd);
        throw std::system_error(e, std::generic_category(), "listen");
    }

    for (;;) {
        sockaddr_in cli{};
        socklen_t clilen = sizeof(cli);
        int cfd = ::accept(fd, reinterpret_cast<sockaddr*>(&cli), &clilen);
        if (cfd < 0) {
            if (errno == EINTR) continue; // interrupted by signal, retry
            int e = errno; ::close(fd);
            throw std::system_error(e, std::generic_category(), "accept");
        }

        resp::RespParser parser;
        std::array<char, 4096> rx{};

        for (;;) {
            ssize_t n = ::recv(cfd, rx.data(), rx.size(), 0);
            if (n == 0) {
                break;
            }
            if (n < 0) {
                if (errno == EINTR) continue;
                break;
            }

            parser.feed(std::string_view(rx.data(), static_cast<size_t>(n)));

            Command cmd;
            while (parser.next(cmd)) {
                std::string reply = dispatcher.dispatch(store, cmd);
                // Note: send may send fewer bytes; loop until all sent.
                const char* p = reply.data();
                size_t left = reply.size();
                while (left > 0) {
                    ssize_t sent = ::send(cfd, p, left, 0);
                    if (sent < 0) {
                        if (errno == EINTR) continue;
                        // EPIPE if client vanished; bail out of this client.
                        left = 0;
                        break;
                    }
                    p += static_cast<size_t>(sent);
                    left -= static_cast<size_t>(sent);
                }
            }

            if (auto err = parser.error()) {
                // Protocol error: send an error and close.
                auto bytes = encode(resp::Err(*err));
                (void) send(cfd, bytes.data(), bytes.size(), 0);
                break;
            }
        }

        close(cfd);
    }
}

}