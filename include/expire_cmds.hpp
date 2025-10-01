#pragma once
#include "dispatcher.hpp"

namespace redite::commands {
    void register_expire_commands(Dispatcher& d); // EXPIRE, TTL
}