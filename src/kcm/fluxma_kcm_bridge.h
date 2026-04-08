#pragma once

#include <cstdint>
#include <string>

#include "fluxma_plugin_root.h"

namespace fluxma {

struct KcmSettingsSnapshot {
    bool enabled = true;
    bool show_hud = true;
    bool subtitle_protection = true;
    bool cursor_protection = true;
    std::uint64_t log_interval_frames = 0;
    std::size_t max_log_messages = 0;

    [[nodiscard]] static KcmSettingsSnapshot from_config(const ModuleConfig& config) noexcept;
};

struct KcmRuntimeSnapshot {
    OutputState state = OutputState::Bypass;
    BypassReason bypass_reason = BypassReason::None;
    bool protected_content = false;
    bool passthrough_only = true;
    bool synthetic_armed = false;
    bool synthetic_queued = false;
    bool synthetic_suppressed_by_protection = false;
    std::uint64_t frame_tap_count = 0;
    std::uint64_t present_feedback_count = 0;
    std::uint64_t deadline_miss_count = 0;
    std::uint64_t dropped_synthetic_count = 0;
    std::uint32_t cadence_hz_millihz = 0;
    std::string hud_text {};

    [[nodiscard]] std::string summary() const;
};

class KfiKcmBridge {
  public:
    explicit KfiKcmBridge(const KfiPluginRoot& plugin_root) noexcept;

    [[nodiscard]] KcmSettingsSnapshot settings() const noexcept;
    [[nodiscard]] KcmRuntimeSnapshot runtime(std::uint64_t now_ns) const;

  private:
    const KfiPluginRoot& plugin_root_;
};

}  // namespace fluxma
