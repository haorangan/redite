#pragma once
#include <string>
#include "dispatcher.hpp"

namespace redite::resp {

    // Array of bulk strings: ["CMD", arg1, ...] -> RESP bytes
    inline std::string encode_command(const Command& cmd) {
        const size_t n = 1 + cmd.argv.size();
        std::string out;
        out.reserve(16 * n); // cheap prealloc

        auto bulk = [&](std::string_view s) {
            out += "$";
            out += std::to_string(s.size());
            out += "\r\n";
            out.append(s.data(), s.size());
            out += "\r\n";
        };

        out += "*";
        out += std::to_string(n);
        out += "\r\n";
        bulk(cmd.name);                 // already uppercased by parser
        for (auto& a : cmd.argv) bulk(a);
        return out;
    }

} // namespace redite::resp