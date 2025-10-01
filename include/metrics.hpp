#pragma once
#include <cstdint>
#include <chrono>

namespace redite {

    struct Metrics {
        using Clock = std::chrono::steady_clock;

        // lifecycle
        Clock::time_point start = Clock::now();

        // high-level
        uint64_t ops_total = 0;   // all commands processed

        // command counters
        uint64_t cmd_get = 0;
        uint64_t cmd_set = 0;
        uint64_t cmd_del = 0;
        uint64_t cmd_ttl = 0;
        uint64_t cmd_expire = 0;
        uint64_t cmd_ping = 0;
        uint64_t cmd_echo = 0;

        // keyspace stats
        uint64_t hits = 0;        // GET hit
        uint64_t misses = 0;      // GET miss
        uint64_t expired_purges = 0; // increment where you purge
    };

}