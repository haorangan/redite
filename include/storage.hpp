#pragma once
#include <unordered_map>
#include "value.hpp"

namespace redite {
    class Storage {
    public:
        Storage() = default;
        ~Storage() = default;

        void set(const std::string &key, std::string value,
                 std::optional<std::chrono::seconds> ttl = std::nullopt);

        int set_ttl(const std::string &key, std::chrono::seconds ttl);

        std::optional<Value> get(const std::string &key);

        bool del(const std::string& key);

        std::optional<long long> ttl(const std::string& key);

    private:
        std::unordered_map<std::string, Value> kv_;

        bool purge_if_expired(const std::string& key);
    };
}
