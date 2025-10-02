#pragma once
#include <string>

namespace redite {
    class Storage;
    class Dispatcher;

    bool replay_aof(const std::string& path, Storage& store, const Dispatcher& dispatcher);
}