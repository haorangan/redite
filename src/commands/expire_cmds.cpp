//
// Created by Haoran Gan on 9/29/25.
//
#include "expire_cmds.hpp"

#include <resp_encoder.hpp>

namespace redite::commands {

    static std::string expire_cmd(Storage& s, const Command& c) {
        if (c.argv.size() != 2) {
            return encode(resp::Err("wrong number of arguments for EXPIRE"));
        }
        const std::string key = c.argv[0];
        long long secs = 0;
        try {
            secs = std::stoll(c.argv[1]);
            if (secs < 0) return encode(resp::Int(0));
        } catch (std::exception&) {
            return encode(resp::Err("value is not an integer or out of range"));
        }
        const int res = s.set_ttl(key, std::chrono::seconds(secs));
        return encode(resp::Int(res));
    }

    static std::string ttl_cmd(Storage& s, const Command& c) {
        if (c.argv.size() != 1)
            return encode(resp::Err("wrong number of arguments for 'TTL'"));

        const auto t = s.ttl(c.argv[0]);         // optional<long long>
        if (!t.has_value())  return encode(resp::Int(-2));  // no key
        if (*t == -1)        return encode(resp::Int(-1));  // exists, no TTL
        return encode(resp::Int(*t));
    }

    void register_expire_commands(Dispatcher& d) {
        d.add("EXPIRE", expire_cmd);
        d.add("TTL", ttl_cmd);
    }
}
