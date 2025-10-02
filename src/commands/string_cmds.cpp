//
// Created by Haoran Gan on 9/29/25.
//

#include "string_cmds.hpp"

#include <resp_encoder.hpp>
#include "aof_writer.hpp"
#include "helpers.hpp"


namespace redite::commands {

    using namespace std::chrono;
    using ll = long long;

    // --- helpers ---
    static bool ieq(const std::string_view a, const std::string_view b) {
        if (a.size()!=b.size()) return false;
        for (size_t i=0;i<a.size();++i) if (std::toupper(static_cast<unsigned char>(a[i])) != std::toupper(static_cast<unsigned char>(b[i]))) return false;
        return true;
    }

    static std::string ping(Storage& s, const Command& cmd) {
        s.metrics().cmd_ping++;
        s.metrics().ops_total++;
        if (!cmd.argv.empty()) {
            return encode(resp::Err("wrong number of arguments for 'PING'"));
        }
        return encode(resp::PONG());
    }

    static std::string set(Storage& s, const Command& cmd) {
        s.metrics().cmd_set++;
        s.metrics().ops_total++;
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
        s.set(cmd.argv[0], cmd.argv[1], ttl);
        if (g_aof) {
            g_aof->append(resp::encode_command(cmd));
        }
        return encode(resp::OK());
    }

    static std::string get(Storage& storage, const Command& cmd) {
        storage.metrics().cmd_get++;
        storage.metrics().ops_total++;
        if (cmd.argv.size() != 1) {
            return encode(resp::Err("syntax error"));
        }
        const auto v = storage.get(cmd.argv[0]);
        if (!v) {
            storage.metrics().misses++;
            return encode(resp::Nil());
        }
        storage.metrics().hits++;
        return v ? encode(resp::BulkString(v.value().data)) : "";
    }

    static std::string del_cmd(Storage& s, const Command& c) {
        s.metrics().cmd_del++;
        s.metrics().ops_total++;
        if (c.argv.empty())
            return encode(resp::Err("wrong number of arguments for 'DEL'"));
        long long n = 0;
        for (const auto& k : c.argv) if (s.del(k)) ++n;
        if (n > 0 && g_aof) {
            g_aof->append(resp::encode_command(c));
        }
        return encode(resp::Int(n));
    }

    static std::string incr_cmd(Storage&, const Command&) {
        return encode(resp::Err("INCR not implemented"));
    }
    static std::string decr_cmd(Storage&, const Command&) {
        return encode(resp::Err("DECR not implemented"));
    }

    void register_string_commands(Dispatcher& d) {
        d.add("PING", ping);
        d.add("SET", set);
        d.add("GET", get);
        d.add("DEL", del_cmd);
        d.add("INCR", incr_cmd);
        d.add("DECR", decr_cmd);
    }// PING, SET, GET, DEL, INCR, DECR
}
