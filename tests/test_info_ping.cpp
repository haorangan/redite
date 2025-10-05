//
// Created by Haoran Gan on 10/5/25.
//
#include <catch2/catch_test_macros.hpp>
#include "storage.hpp"
#include "dispatcher.hpp"
#include "registry.hpp"

using namespace redite;

static Dispatcher make_dispatcher() {
    Dispatcher d; commands::register_all_commands(d); return d;
}

TEST_CASE("PING returns PONG") {
    Storage s;
    const auto d = make_dispatcher();
    auto r = d.dispatch(s, {"PING", {}});
    REQUIRE(r == "+PONG\r\n");
}

TEST_CASE("INFO returns bulk with sections") {
    Storage s; auto d = make_dispatcher();
    auto r = d.dispatch(s, {"INFO", {}});
    // Expect a RESP bulk string that contains these section headers
    REQUIRE(!r.empty());
    REQUIRE(r.rfind('$', 0) == 0);              // Bulk start
    REQUIRE(r.find("# Server") != std::string::npos);
    REQUIRE(r.find("# Stats")  != std::string::npos);
    REQUIRE(r.find("# Keyspace") != std::string::npos);
    REQUIRE(r.find("uptime_in_seconds:") != std::string::npos);
}