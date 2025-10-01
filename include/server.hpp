#pragma once
#include <cstdint>
#include "storage.hpp"
#include "dispatcher.hpp"
#include "resp_parser.hpp"

namespace redite {

    class Server {
    public:
        explicit Server(const uint16_t port) : port_(port) {}
        // Blocks; serves one client at a time (MVP).
        void run(Storage& store, const Dispatcher& dispatcher) const;

    private:
        uint16_t port_;
    };

} // namespace redite