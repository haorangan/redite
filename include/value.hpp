//
// Created by Haoran Gan on 9/29/25.
//

#pragma once
#include <string>
#include <optional>
#include <chrono>
#include <utility>


namespace redite {
    using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;
    using Clock     = std::chrono::steady_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    enum class ValueType {
        String
    // TODO
    };

    struct Value {
        ValueType type;
        std::string data;
        std::optional<TimePoint> expireAt;
        Value() = default;
        explicit Value(std::string  data, const std::optional<TimePoint> expireAt) :
        type(ValueType::String),
        data(std::move(data)), expireAt(expireAt)
        {}
    };

    inline bool is_expired(const Value& v, const TimePoint now = Clock::now()) {
        return v.expireAt && now >= *v.expireAt;
    }

    inline void set_ttl(Value& v, const std::chrono::seconds ttl,
                        const TimePoint now = Clock::now()) {
        v.expireAt = now + ttl;
    }
}
