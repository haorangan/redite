#include "resp_parser.hpp"
#include <algorithm>
#include <cctype>

namespace redite::resp {

void RespParser::feed(std::string_view bytes) {
    buf_.append(bytes.data(), bytes.size());
}

void RespParser::reset() {
    buf_.clear();
    pos_ = 0;
    expect_elems_.reset();
    next_bulk_len_.reset();
    tmp_args_.clear();
    error_.reset();
}

bool RespParser::next(Command& out) {
    if (error_) return false;

    // If we’re not in the middle of an array, start one.
    if (!expect_elems_) {
        if (!parse_array_header()) return false; // need more bytes or error set
        tmp_args_.clear();
    }

    // Parse elements until we have a full array.
    while (*expect_elems_ > 0) {
        if (!parse_next_bulk()) return false; // need more bytes or error
        // When a bulk completes, it was pushed to tmp_args_ by parse_next_bulk
        --(*expect_elems_);
    }

    // We have a complete array → convert to Command (flat bulks)
    if (tmp_args_.empty()) {
        error_ = "ERR protocol: empty array";
        return false;
    }

    // First element is command name → uppercase it
    Command cmd;
    cmd.name = tmp_args_.front();
    std::transform(cmd.name.begin(), cmd.name.end(), cmd.name.begin(),
                   [](unsigned char c){ return static_cast<char>(std::toupper(c)); });
    cmd.argv.assign(tmp_args_.begin() + 1, tmp_args_.end());

    // reset array state for the next frame
    expect_elems_.reset();
    next_bulk_len_.reset();
    tmp_args_.clear();

    // compact buffer if we’ve advanced a lot
    compact_if_needed();

    out = std::move(cmd);
    return true;
}

bool RespParser::parse_array_header() {
    // Need at least "*<...>\r\n"
    if (pos_ >= buf_.size()) return false;
    if (buf_[pos_] != '*') { error_ = "ERR protocol: expected array (*)"; return false; }

    // find CRLF
    std::size_t cr = buf_.find('\r', pos_ + 1);
    if (cr == std::string::npos || cr + 1 >= buf_.size()) return false; // need more bytes
    if (buf_[cr + 1] != '\n') { error_ = "ERR protocol: expected CRLF after array length"; return false; }

    // parse length
    std::size_t n = 0;
    if (!to_uint(std::string_view{buf_.data() + pos_ + 1, cr - (pos_ + 1)}, n)) {
        error_ = "ERR protocol: bad array length";
        return false;
    }

    pos_ = cr + 2;            // consume "*<len>\r\n"
    expect_elems_ = n;
    return true;
}

bool RespParser::parse_next_bulk() {
    if (error_) return false;

    // If we don't yet know the next bulk length, read its header: $<len>\r\n
    if (!next_bulk_len_) {
        if (pos_ >= buf_.size()) return false;
        if (buf_[pos_] != '$') { error_ = "ERR protocol: expected bulk ($)"; return false; }

        std::size_t cr = buf_.find('\r', pos_ + 1);
        if (cr == std::string::npos || cr + 1 >= buf_.size()) return false; // need more bytes
        if (buf_[cr + 1] != '\n') { error_ = "ERR protocol: expected CRLF after bulk length"; return false; }

        std::size_t len = 0;
        if (!to_uint(std::string_view{buf_.data() + pos_ + 1, cr - (pos_ + 1)}, len)) {
            // RESP allows -1 for Null Bulk in replies, but requests (commands) are bulk strings with non-negative len
            if (cr > pos_ + 1 && buf_[pos_ + 1] == '-' &&
                std::string_view{buf_.data() + pos_ + 2, cr - (pos_ + 2)} == "1") {
                error_ = "ERR protocol: null bulk not allowed in command";
            } else {
                error_ = "ERR protocol: bad bulk length";
            }
            return false;
        }

        next_bulk_len_ = len;
        pos_ = cr + 2; // consume "$<len>\r\n"
    }

    // Now we know how many bytes the payload should have + trailing CRLF
    const std::size_t need = *next_bulk_len_;
    if (buf_.size() - pos_ < need + 2) return false; // wait for whole payload + CRLF
    if (buf_[pos_ + need] != '\r' || buf_[pos_ + need + 1] != '\n') {
        error_ = "ERR protocol: bulk not terminated with CRLF";
        return false;
    }

    // Capture payload verbatim (binary-safe)
    tmp_args_.emplace_back(buf_.data() + pos_, need);

    pos_ += need + 2;   // consume "<payload>\r\n"
    next_bulk_len_.reset();
    return true;
}

// Strict non-negative integer parse (no leading +/-, no spaces)
bool RespParser::to_uint(std::string_view s, std::size_t& out) {
    if (s.empty()) return false;
    std::size_t val = 0;
    for (unsigned char ch : s) {
        if (ch < '0' || ch > '9') return false;
        std::size_t digit = ch - '0';
        // naive overflow check for size_t
        if (val > (static_cast<std::size_t>(-1) / 10)) return false;
        val = val * 10 + digit;
    }
    out = val;
    return true;
}

void RespParser::compact_if_needed() {
    // If we consumed a lot, drop the front to keep buf_ small.
    if (pos_ > 65536 && pos_ > buf_.size() / 2) {
        buf_.erase(0, pos_);
        pos_ = 0;
    }
}

}