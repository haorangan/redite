//
// Created by Haoran Gan on 9/29/25.
//
#pragma once
#include "dispatcher.hpp"

namespace redite::commands {
    void register_string_commands(Dispatcher& d); // PING, SET, GET, DEL, INCR, DECR
}
