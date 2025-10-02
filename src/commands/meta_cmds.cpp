//
// Created by Haoran Gan on 9/29/25.
//

#include "meta_cmds.hpp"

#include <resp_encoder.hpp>

namespace redite::commands {
    using namespace resp;
    static std::string echo_cmd(Storage& s, const Command& cmd) {
        s.metrics().cmd_echo++;
        s.metrics().ops_total++;
        if (cmd.argv.size() != 1) {
            return encode(Err("wrong number of arguments for ECHO"));
        }
        return encode(Bulk(cmd.argv[0]));
    }

    namespace {
        // Helper to format seconds since start
        long long uptime_seconds(const Metrics& m) {
            auto now = Metrics::Clock::now();
            return std::chrono::duration_cast<std::chrono::seconds>(now - m.start).count();
        }

        std::string info_cmd(Storage& s, const Command& c) {
            auto& m = s.metrics();
            m.ops_total++; // counting INFO itself is optional

            if (!c.argv.empty()) {
                return encode(Err("wrong number of arguments for 'INFO'"));
            }

            // Build a simple, Redis-like INFO text block
            std::string out;
            out += "# Server\r\n";
            out += "uptime_in_seconds:" + std::to_string(uptime_seconds(m)) + "\r\n";

            out += "\r\n# Stats\r\n";
            out += "ops_total:"        + std::to_string(m.ops_total)        + "\r\n";
            out += "cmd_get:"          + std::to_string(m.cmd_get)          + "\r\n";
            out += "cmd_set:"          + std::to_string(m.cmd_set)          + "\r\n";
            out += "cmd_del:"          + std::to_string(m.cmd_del)          + "\r\n";
            out += "cmd_expire:"       + std::to_string(m.cmd_expire)       + "\r\n";
            out += "cmd_ttl:"          + std::to_string(m.cmd_ttl)          + "\r\n";
            out += "cmd_ping:"         + std::to_string(m.cmd_ping)         + "\r\n";
            out += "cmd_echo:"         + std::to_string(m.cmd_echo)         + "\r\n";

            out += "\r\n# Keyspace\r\n";
            out += "keys:"             + std::to_string(/* current size */ s.size()) + "\r\n";
            out += "hits:"             + std::to_string(m.hits)             + "\r\n";
            out += "misses:"           + std::to_string(m.misses)           + "\r\n";
            out += "expired_purges:"   + std::to_string(m.expired_purges)   + "\r\n";

            return encode(Bulk(std::move(out))); // single bulk, multi-line
        }
    }
    // later add info about runtime, etc
    void register_meta_commands(Dispatcher& d) {
        d.add("ECHO", echo_cmd);
        d.add("INFO", info_cmd);
    }
}
