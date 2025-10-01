#include "resp_encoder.hpp"

#include <assert.h>
#include <variant>

#include "value.hpp"

namespace redite::resp {

    static constexpr std::string_view EOL = "\r\n";
    std::string encode(const Reply& r) {
        if (std::holds_alternative<SimpleString>(r)) {
            return encodeSimple(std::get<SimpleString>(r));
        }
        if (std::holds_alternative<Array>(r)) {
            return encodeArray(std::get<Array>(r));
        }
        if (std::holds_alternative<Error>(r)) {
            return encodeError(std::get<Error>(r));
        }
        if (std::holds_alternative<BulkString>(r)) {
            return encodeBulk(std::get<BulkString>(r));
        }
        if (std::holds_alternative<Integer>(r)) {
            return encodeInteger(std::get<Integer>(r));
        }
        assert(false); // UNREACHABLE
        return {};
    }


    std::string encodeSimple(const SimpleString& s) {
        return "+" + s.value + std::string(EOL);
    }

    std::string encodeError(const Error& e) {
        return "-ERR " + e.message + std::string(EOL);
    }

    std::string encodeInteger(const Integer& i) {
        return ":" + i.value + std::string(EOL);
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
