//
// Created by Haoran Gan on 9/29/25.
//

#include "string_cmds.hpp"

#include <resp_encoder.hpp>
#include "algorithm"



namespace redite::commands {

    using namespace std::chrono;
    using ll = long long;

    // --- helpers ---
    static bool ieq(const std::string_view a, const std::string_view b) {
        if (a.size()!=b.size()) return false;
        for (size_t i=0;i<a.size();++i) if (std::toupper(static_cast<unsigned char>(a[i])) != std::toupper(static_cast<unsigned char>(b[i]))) return false;
        return true;
    }

    static std::string ping(Storage&, const Command& cmd) {
        if (!cmd.argv.empty()) {
            return encode(resp::Err("wrong number of arguments for 'PING'"));
        }
        return encode(resp::PONG());
    }

    static std::string set(Storage& storage, const Command& cmd) {
        // syntax is SET key value [EX seconds]
        if (cmd.argv.size() < 2) {
            return encode(resp::Err("wrong number of arguments for 'SET'"));
        }

        std::optional<seconds> ttl = std::nullopt;
        if (cmd.argv.size() == 4 && ieq(cmd.argv[2], "EX")) {
            try {
                const ll sec = std::stoll(cmd.argv[3]);
                if (sec < 0) return encode(resp::Err("invalid expire time in set"));
                ttl = seconds(sec);
            } catch (const std::exception&) {
                return encode(resp::Err("invalid expire time in set"));
            }
        }
        else if (cmd.argv.size() != 2) {
            return encode(resp::Err("syntax error"));
        }
        storage.set(cmd.argv[0], cmd.argv[1], ttl);
        return encode(resp::OK());
    }

    void register_string_commands(Dispatcher& d) {
        d.add("PING", ping);
        d.add("SET", set);
    }// PING, SET, GET, DEL, INCR, DECR
}
