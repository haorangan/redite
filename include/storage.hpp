#pragma once
#include <unordered_map>

#include "metrics.hpp"
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

        Metrics& metrics() { return metrics_; }
        [[nodiscard]] const Metrics& metrics() const { return metrics_; }

        size_t size();

    private:
        std::unordered_map<std::string, Value> kv_;
        Metrics metrics_;

        bool purge_if_expired(const std::string& key);
    };
}
