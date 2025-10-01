#pragma once
#include "dispatcher.hpp"

namespace redite::commands {
    void register_meta_commands(Dispatcher& d); // ECHO (and INFO later)
}