#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <cstddef>
#include "dispatcher.hpp" // for Command

namespace redite::resp {

    class RespParser {
    public:
        void feed(std::string_view bytes);          // append incoming bytes
        bool next(Command& out);                    // parse next complete command (flat array of bulks)
        void reset();                               // clear all state/buffers
        [[nodiscard]] std::optional<std::string> error() const { return error_; }

    private:
        // parsing helpers
        bool parse_array_header();                  // expects "*<N>\r\n"
        bool parse_next_bulk();                     // expects "$<len>\r\n<payload>\r\n"

        // utils
        static bool to_uint(std::string_view s, std::size_t& out); // strict non-negative

        // internal state
        std::string buf_;        // accumulated bytes from socket
        std::size_t pos_ = 0;    // read cursor into buf_

        std::optional<std::size_t> expect_elems_;   // remaining array elements to read
        std::optional<std::size_t> next_bulk_len_;  // pending bulk length
        std::vector<std::string> tmp_args_;         // holds array items as we parse

        std::optional<std::string> error_;          // set on malformed input

        // maintenance
        void compact_if_needed();                   // drop consumed prefix of buf_
    };

}