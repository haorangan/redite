#pragma once
#include <string>
#include <vector>
#include <optional>
#include <variant>  

namespace redite::resp {

    struct SimpleString {
        std::string value; // +OK
    };
    struct Error {
        std::string message; // -ERR msg
    };
    struct Integer {
        long long value = 0; // :123
    };
    struct BulkString {
        std::optional<std::string> data; // $len ... / $-1
    };

    struct Array;

    using Reply = std::variant<
        SimpleString,
        Error,
        Integer,
        BulkString,
        std::unique_ptr<Array>
    >;

    struct Array {
        std::vector<Reply> elements; // elements can be any Reply, including nested arrays
    };

    inline Reply OK() {
        return SimpleString{"OK"};
    }

    inline Reply PONG() {
        return SimpleString{"PONG"};
    }
    inline Reply Simple(std::string s) {
        return SimpleString{std::move(s)};
    }
    inline Reply Err(std::string msg) {
        return Error{std::move(msg)};
    }
    inline Reply Int(const long long n) {
        return Integer{n};
    }
    inline Reply Bulk(std::string s) {
        return BulkString{std::move(s)};
    }
    inline Reply Nil() {
        return BulkString{std::nullopt};
    }
    inline Reply Arr(std::vector<Reply> elems) {
        auto a = std::make_unique<Array>();
        a->elements = std::move(elems);
        return Reply{std::move(a)};
    }

    // ---------- Encoder API ----------
    std::string encode(const Reply& r);

    std::string encodeSimple(const SimpleString& s);
    std::string encodeError(const Error& e);
    std::string encodeInteger(const Integer& i);
    std::string encodeBulk(const BulkString& b);
    std::string encodeArray(const Array& a);
}
