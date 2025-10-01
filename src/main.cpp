//
// Created by Haoran Gan on 9/29/25.
//
#include <cstdint>
#include <iostream>
#include <string>
#include "storage.hpp"
#include "dispatcher.hpp"
#include "registry.hpp"
#include "server.hpp"

int main(int argc, char** argv) {
    uint16_t port = 6380;
    if (argc >= 2) {
        try { port = static_cast<uint16_t>(std::stoul(argv[1])); }
        catch (...) { std::cerr << "Usage: " << argv[0] << " [port]\n"; return 1; }
    }

    redite::Storage store;
    redite::Dispatcher dispatcher;
    redite::commands::register_all_commands(dispatcher);

    try {
        redite::Server srv(port);
        std::cout << "redite listening on 0.0.0.0:" << port << " …\n";
        srv.run(store, dispatcher);
    } catch (const std::exception& ex) {
        std::cerr << "fatal: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}
