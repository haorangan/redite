#pragma once
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include "storage.hpp"

namespace redite {

    struct Command {
        std::string name;
        std::vector<std::string> argv;
    };

    using Handler = std::function<std::string(Storage&, const Command&)>;

    class Dispatcher {
    public:
        void add(std::string_view verb, Handler h) {
            table_[std::string(verb)] = std::move(h);
        }
        std::string dispatch(Storage& store, const Command& cmd) const;
    private:
        std::unordered_map<std::string, Handler> table_;
    };

} // namespace redite