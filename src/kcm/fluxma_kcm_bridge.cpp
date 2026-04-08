#include "fluxma_kcm_bridge.h"

namespace fluxma {

KcmSettingsSnapshot KcmSettingsSnapshot::from_config(const ModuleConfig& config) noexcept {
    return KcmSettingsSnapshot {
        .enabled = config.enabled,
        .show_hud = config.show_hud,
        .subtitle_protection = config.subtitle_protection,
        .cursor_protection = config.cursor_protection,
        .log_interval_frames = config.log_interval_frames,
        .max_log_messages = config.max_log_messages,
    };
}

std::string KcmRuntimeSnapshot::summary() const {
    std::string output;
    output += "state=";
    output += std::string(to_string(state));
    output += " bypass=";
    output += std::string(to_string(bypass_reason));
    output += " protected=";
    output += std::string(to_bool_string(protected_content));
    output += " passthrough=";
    output += std::string(to_bool_string(passthrough_only));
    output += " synthetic-armed=";
    output += std::string(to_bool_string(synthetic_armed));
    output += " synthetic-queued=";
    output += std::string(to_bool_string(synthetic_queued));
    output += " synthetic-suppressed-by-protection=";
    output += std::string(to_bool_string(synthetic_suppressed_by_protection));
    output += " cadence-millihz=";
    output += std::to_string(cadence_hz_millihz);
    return output;
}

KfiKcmBridge::KfiKcmBridge(const KfiPluginRoot& plugin_root) noexcept
    : plugin_root_(plugin_root) {}

KcmSettingsSnapshot KfiKcmBridge::settings() const noexcept {
    return KcmSettingsSnapshot::from_config(plugin_root_.config());
}

KcmRuntimeSnapshot KfiKcmBridge::runtime(std::uint64_t now_ns) const {
    const auto report = plugin_root_.observe_output_runtime(now_ns);
    return KcmRuntimeSnapshot {
        .state = report.snapshot.state,
        .bypass_reason = report.snapshot.bypass_reason,
        .protected_content = report.snapshot.protected_content,
        .passthrough_only = report.snapshot.passthrough_only,
        .synthetic_armed = report.synthetic_armed(),
        .synthetic_queued = report.synthetic_queued(),
        .synthetic_suppressed_by_protection = report.synthetic_suppressed_by_protection(),
        .cadence_hz_millihz = report.snapshot.cadence_hz_millihz,
        .hud_text = report.hud_text,
    };
}

}  // namespace fluxma
