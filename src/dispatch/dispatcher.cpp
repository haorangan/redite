#include <dispatcher.hpp>
#include <resp_encoder.hpp>
#include "logger.hpp"
#include <chrono>

namespace redite {

    using Clock = std::chrono::steady_clock;

    std::string Dispatcher::dispatch(Storage &store, const Command &cmd) const {

        const auto it = table_.find(cmd.name);
        if (it == table_.end()) {
            RLOG_WARN("cmd_unknown", { Field{"cmd", cmd.name} });
            return encode(resp::Err("unknown command '" + cmd.name + "'"));
        }

        // --- timing & basic fields
        auto t0 = Clock::now();
        const size_t argc = cmd.argv.size();
        bool ok = true;

        try {
            auto out = (it->second)(store, cmd);

            // duration + slowlog policy
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - t0).count();
            if (ms >= 25) {
                RLOG_INFO("cmd_slow", { Field{"cmd", cmd.name},
                                        Field{"argc", std::to_string(argc)},
                                        Field{"ms",   std::to_string(ms)},
                                        Field{"status","ok"} });
            } else {
                RLOG_DEBUG("cmd", { Field{"cmd", cmd.name},
                                    Field{"argc", std::to_string(argc)},
                                    Field{"status","ok"} });
            }
            return out;

        } catch (const std::exception& ex) {
            ok = false;
            RLOG_ERROR("cmd_exception", { Field{"cmd", cmd.name},
                                          Field{"argc", std::to_string(argc)},
                                          Field{"what", ex.what()} });
            return encode(resp::Err(std::string("internal error: ") + ex.what()));

        } catch (...) {
            ok = false;
            RLOG_ERROR("cmd_exception", { Field{"cmd", cmd.name},
                                          Field{"argc", std::to_string(argc)},
                                          Field{"what", "unknown"} });
            return encode(resp::Err("internal error"));
        }
    }
}