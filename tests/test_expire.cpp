//
// Created by Haoran Gan on 10/5/25.
//

#include <catch2/catch_test_macros.hpp>
#include <thread>
#include <chrono>
#include "storage.hpp"
#include "dispatcher.hpp"
#include "registry.hpp"
#include "resp_encoder.hpp"

using namespace redite;
using namespace std::chrono_literals;

static Dispatcher make_dispatcher() {
    Dispatcher d; commands::register_all_commands(d); return d;
}

TEST_CASE("Expire: TTL states and lazy expiration") {
    Storage s;
    auto d = make_dispatcher();

    // Non-existent key
    REQUIRE(d.dispatch(s, {"TTL", {"nx"}}) == ":-2\r\n");

    // Exists but no TTL
    REQUIRE(d.dispatch(s, {"SET", {"k","v"}}) == "+OK\r\n");
    REQUIRE(d.dispatch(s, {"TTL", {"k"}}) == ":-1\r\n");

    // Set TTL and validate
    REQUIRE(d.dispatch(s, {"EXPIRE", {"k","1"}}) == ":1\r\n"); // 1 = TTL set
    auto ttl = d.dispatch(s, {"TTL", {"k"}});
    REQUIRE(ttl.rfind(':', 0) == 0); // integer
    // Sleep past expiration and trigger lazy expire via GET
    std::this_thread::sleep_for(1200ms);
    auto g = d.dispatch(s, {"GET", {"k"}});
    REQUIRE(g == "$-1\r\n"); // Nil

    // TTL on missing key remains -2
    REQUIRE(d.dispatch(s, {"TTL", {"k"}}) == ":-2\r\n");
}

TEST_CASE("Expire: EXPIRE with negative returns 0") {
    Storage s;
    auto d = make_dispatcher();

    REQUIRE(d.dispatch(s, {"SET", {"k","v"}}) == "+OK\r\n");
    auto r = d.dispatch(s, {"EXPIRE", {"k","-5"}});
    REQUIRE(r == ":0\r\n");
}