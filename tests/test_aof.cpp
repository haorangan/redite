//
// Created by Haoran Gan on 10/5/25.
//
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <iostream>

#include "storage.hpp"
#include "dispatcher.hpp"
#include "registry.hpp"
#include "aof_writer.hpp"
#include "aof_replayer.hpp"
#include "resp_encoder.hpp"

using namespace redite;

static Dispatcher make_dispatcher() {
    Dispatcher d; commands::register_all_commands(d); return d;
}

TEST_CASE("AOF: replay restores final state") {
    namespace fs = std::filesystem;
    auto path = fs::temp_directory_path() / "redite_test.aof";
    fs::remove(path);

    {
        Storage s;
        auto d = make_dispatcher();
        AofWriter w(path.string(), AofFsyncMode::No);
        g_aof = &w;

        REQUIRE(d.dispatch(s, {"SET", {"x","1"}}) == "+OK\r\n");
        REQUIRE(d.dispatch(s, {"SET", {"y","2"}}) == "+OK\r\n");
        REQUIRE(d.dispatch(s, {"DEL", {"x"}})     == ":1\r\n");
        REQUIRE(d.dispatch(s, {"SET", {"z","3"}}) == "+OK\r\n");

        g_aof = nullptr;  // stop any further appends
    }

    Storage s2;
    auto d2 = make_dispatcher();
    (void) replay_aof(path.string(), s2, d2);

    REQUIRE(d2.dispatch(s2, {"GET", {"x"}}) == "$-1\r\n");
    REQUIRE(d2.dispatch(s2, {"GET", {"y"}}) == "$1\r\n2\r\n");
    REQUIRE(d2.dispatch(s2, {"GET", {"z"}}) == "$1\r\n3\r\n");

    fs::remove(path);
}


TEST_CASE("AOF: EXPIRE commands persist (value presence)") {
    namespace fs = std::filesystem;
    auto path = fs::temp_directory_path() / "redite_ttl.aof";
    fs::remove(path);
    {
        Storage s;
        auto d = make_dispatcher();
        AofWriter w(path.string(), AofFsyncMode::No);
        g_aof = &w;

        REQUIRE(d.dispatch(s, {"SET", {"k","v"}}) == "+OK\r\n");
        REQUIRE(d.dispatch(s, {"EXPIRE", {"k","10"}}) == ":1\r\n");
    }

    Storage s2;
    auto d2 = make_dispatcher();
    (void) replay_aof(path.string(), s2, d2);
    auto g = d2.dispatch(s2, {"GET", {"k"}});
    REQUIRE(g == "$1\r\nv\r\n");

    g_aof = nullptr;
    fs::remove(path);
}