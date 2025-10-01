#pragma once
#include "dispatcher.hpp"

namespace redite::commands {
    void register_all_commands(Dispatcher& d);
}
