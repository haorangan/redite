#include "resp_encoder.hpp"

#include <string>
#include <string_view>
#include <variant>

namespace redite::resp {

    static constexpr std::string_view EOL = "\r\n";

    std::string encode(const Reply& r) {
        if (const auto p = std::get_if<SimpleString>(&r)) {
            return encodeSimple(*p);
        }
        if (const auto p = std::get_if<Error>(&r)) {
            return encodeError(*p);
        }
        if (const auto p = std::get_if<Integer>(&r)) {
            return encodeInteger(*p);
        }
        if (const auto p = std::get_if<BulkString>(&r)) {
            return encodeBulk(*p);
        }
        if (const auto p = std::get_if<std::unique_ptr<Array>>(&r)) {
            return *p ? encodeArray(**p)
                      : std::string("*-1") + std::string(EOL);
        }
        return {};
    }


    std::string encodeSimple(const SimpleString& s) {
        return "+" + s.value + std::string(EOL);
    }

    std::string encodeError(const Error& e) {
        return "-ERR " + e.message + std::string(EOL);
    }

    std::string encodeInteger(const Integer& i) {
        return ":" + std::to_string(i.value) + std::string(EOL);
    }

    std::string encodeBulk(const BulkString& b) {
        if (!b.data) return std::string("$-1") + std::string(EOL); // nil bulk
        const std::string& s = *b.data;
        return "$" + std::to_string(s.size()) + std::string(EOL) + s + std::string(EOL);
    }
    std::string encodeArray(const Array& a) {
        std::string out = "*" + std::to_string(a.elements.size()) + std::string(EOL);
        for (const auto& elem : a.elements) {
            out += encode(elem); // each element already includes its EOL where needed
        }
        return out;
    }

}
