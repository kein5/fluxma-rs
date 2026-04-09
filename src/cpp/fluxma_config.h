#pragma once

#include <cstdint>

namespace fluxma {

enum class ModuleMode : std::uint8_t {
    PassthroughOnly = 0,
    Synthetic2x = 1,
};

struct ModuleConfig {
    bool enabled = true;
    ModuleMode mode = ModuleMode::Synthetic2x;
    bool show_hud = true;
    bool subtitle_protection = true;
    bool cursor_protection = true;
    std::uint64_t log_interval_frames = 120;
    std::size_t max_log_messages = 64;
};

[[nodiscard]] inline const char* to_string(ModuleMode mode) noexcept {
    switch (mode) {
    case ModuleMode::PassthroughOnly:
        return "passthrough-only";
    case ModuleMode::Synthetic2x:
        return "synthetic-2x";
    }

    return "unknown";
}

}  // namespace fluxma
