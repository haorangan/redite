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

#ifdef REDITE_ENABLE_LISTS

TEST_CASE("Lists: RPUSH/LLEN/LPOP") {
    Storage s; auto d = make_dispatcher();

    REQUIRE(d.dispatch(s, {"RPUSH", {"k","a","b","c"}}) == ":3\r\n");
    REQUIRE(d.dispatch(s, {"LLEN", {"k"}}) == ":3\r\n");

    auto p1 = d.dispatch(s, {"LPOP", {"k"}});
    REQUIRE(p1 == "$1\r\na\r\n");
    REQUIRE(d.dispatch(s, {"LLEN", {"k"}}) == ":2\r\n");

    auto p2 = d.dispatch(s, {"RPOP", {"k"}}); // should pop "c"
    REQUIRE(p2 == "$1\r\nc\r\n");
    REQUIRE(d.dispatch(s, {"LLEN", {"k"}}) == ":1\r\n");
}

TEST_CASE("Lists: WRONGTYPE guard") {
    Storage s; auto d = make_dispatcher();

    REQUIRE(d.dispatch(s, {"SET", {"x","v"}}) == "+OK\r\n");
    auto r = d.dispatch(s, {"RPUSH", {"x","z"}});
    REQUIRE(r.rfind("-ERR", 0) == 0);
    REQUIRE(r.find("WRONGTYPE") != std::string::npos);
}

#endif // REDITE_ENABLE_LISTS
