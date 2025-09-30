//
// Created by Haoran Gan on 9/29/25.
//

#include "storage.hpp"

namespace redite {
    void Storage::set(const std::string& key, std::string value, const std::optional<std::chrono::seconds> ttl) {
        std::optional<TimePoint> exp = std::nullopt;
        if (ttl.has_value()) {
            const TimePoint tp = Clock::now();
            exp = tp + ttl.value();
        }
        Value val(std::move(value), exp);
        purge_if_expired(key);
        kv_.insert_or_assign(key, std::move(val));
    }

    bool Storage::del(const std::string& key) {
        if (purge_if_expired(key)) {
            return false;
        }
        if (const auto it = kv_.find(key); it != kv_.end()) {
            kv_.erase(it);
            return true;
        }
        return false;
    }


    std::optional<long long> Storage::ttl(const std::string& key) {
        const auto it = kv_.find(key);
        if (it == kv_.end()) {
            return std::nullopt;
        }

        if (is_expired(it->second)) {
            kv_.erase(it);
            return std::nullopt;
        }

        if (!it->second.expireAt) {
            return -1;
        }

        const auto now = Clock::now();
        auto remaining = std::chrono::duration_cast<std::chrono::seconds>(*it->second.expireAt - now).count();
        if (remaining < 0) remaining = 0;
        return remaining;
    }

    bool Storage::purge_if_expired(const std::string& key) {
        const auto it = kv_.find(key);
        if (it == kv_.end()) return false;
        if (is_expired(it->second)) {
            kv_.erase(it);
            return true;
        }
        return false;
    }
}
