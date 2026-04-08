#pragma once

#include <cstdint>

namespace fluxma {

struct ModuleConfig {
    bool enabled = true;
    bool show_hud = true;
    bool subtitle_protection = true;
    bool cursor_protection = true;
    std::uint64_t log_interval_frames = 120;
    std::size_t max_log_messages = 64;
};

}  // namespace fluxma
