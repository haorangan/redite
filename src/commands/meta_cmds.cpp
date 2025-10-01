//
// Created by Haoran Gan on 9/29/25.
//

#include "meta_cmds.hpp"

#include <resp_encoder.hpp>

namespace redite::commands {
    using namespace resp;
    static std::string echo_cmd(Storage& s, const Command& cmd) {
        if (cmd.argv.size() != 1) {
            return encode(Err("wrong number of arguments for ECHO"));
        }
        return encode(Bulk(cmd.argv[0]));
    }
    // later add info about runtime, etc
    void register_meta_commands(Dispatcher& d) {
        d.add("ECHO", echo_cmd);
    }
}
