#include <dispatcher.hpp>
#include <resp_encoder.hpp>

namespace redite {
    std::string Dispatcher::dispatch(Storage &store, const Command &cmd) const {
        // need to normalize cmd.name, probably to uppercase (will do in parser)
        const auto it = table_.find(cmd.name);
        if (it == table_.end()) {
            return encode(resp::Err("unknown command '" + cmd.name + "'"));
        }

        try {
            return (it->second)(store, cmd);
        } catch (const std::exception& ex) {
            return encode(resp::Err(std::string("internal error: ") + ex.what()));
        } catch (...) {
            return encode(resp::Err("internal error"));
        }
    }

}
