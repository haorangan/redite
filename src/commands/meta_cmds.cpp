//
// Created by Haoran Gan on 9/29/25.
//
#pragma once
#include "dispatcher.hpp"

namespace redite::commands {
    void register_meta_commands(Dispatcher& d); // ECHO (and INFO later)
}
