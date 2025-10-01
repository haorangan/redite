//
// Created by Haoran Gan on 9/29/25.
//

#include "registry.hpp"

#include "expire_cmds.hpp"
#include "meta_cmds.hpp"
#include "string_cmds.hpp"

namespace redite::commands {

    void register_all_commands(Dispatcher& d) {
        register_expire_commands(d);
        register_meta_commands(d);
        register_string_commands(d);
    }

}
