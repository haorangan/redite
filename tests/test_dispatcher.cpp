#include <iostream>
#include <catch2/catch_test_macros.hpp>
#include "storage.hpp"
#include "dispatcher.hpp"
#include "registry.hpp"

using namespace redite;

static Dispatcher make_dispatcher() {
    Dispatcher d;
    commands::register_all_commands(d);
    return d;
}

TEST_CASE("Dispatcher: SET/GET round trip") {
    Storage s;
    auto d = make_dispatcher();

    auto r1 = d.dispatch(s, {"SET", {"a","b"}});
    REQUIRE(r1 == "+OK\r\n");

    auto r2 = d.dispatch(s, {"GET", {"a"}});
    REQUIRE(r2 == "$1\r\nb\r\n");
}

TEST_CASE("Dispatcher: GET miss yields Nil") {
    Storage s;
    auto d = make_dispatcher();

    auto r = d.dispatch(s, {"GET", {"nope"}});
    REQUIRE(r == "$-1\r\n"); // RESP Nil
}

TEST_CASE("Dispatcher: DEL returns number of removed keys") {
    Storage s;
    auto d = make_dispatcher();

    REQUIRE(d.dispatch(s, {"SET", {"k1","v"}}) == "+OK\r\n");
    REQUIRE(d.dispatch(s, {"SET", {"k2","v"}}) == "+OK\r\n");

    auto r = d.dispatch(s, {"DEL", {"k1","k2","k3"}});
    REQUIRE(r == ":2\r\n");
}

TEST_CASE("Dispatcher: unknown command returns error") {
    Storage s;
    auto d = make_dispatcher();

    auto r = d.dispatch(s, {"NOPE", {}});
    REQUIRE(r.rfind("-ERR", 0) == 0);                        // starts with -ERR
    REQUIRE(r.find("unknown command") != std::string::npos); // contains message
}

TEST_CASE("Dispatcher: argument errors surface as -ERR") {
    Storage s;
    auto d = make_dispatcher();

    auto r1 = d.dispatch(s, {"SET", {"only_one"}});
    REQUIRE(r1.rfind("-ERR", 0) == 0);
    REQUIRE(r1.find("wrong number of arguments") != std::string::npos);

    auto r2 = d.dispatch(s, {"GET", {"a","b"}});
    REQUIRE(r2.rfind("-ERR", 0) == 0);
    REQUIRE(r2.find("syntax error") != std::string::npos);
}