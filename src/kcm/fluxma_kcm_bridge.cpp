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
    output += " frame-taps=";
    output += std::to_string(frame_tap_count);
    output += " present-feedback=";
    output += std::to_string(present_feedback_count);
    output += " deadline-miss=";
    output += std::to_string(deadline_miss_count);
    output += " synthetic-dropped=";
    output += std::to_string(dropped_synthetic_count);
    output += " cadence-millihz=";
    output += std::to_string(cadence_hz_millihz);
    return output;
}

std::string KcmNativeBridgeSnapshot::summary() const {
    std::string output;
    output += "state=";
    output += std::string(to_string(state));
    output += " frame-deferred=";
    output += std::string(to_string(frame_deferred_reason));
    output += " present-deferred=";
    output += std::string(to_string(present_deferred_reason));
    output += " frame-version-blocked=";
    output += std::string(to_bool_string(frame_version_blocked));
    output += " present-version-blocked=";
    output += std::string(to_bool_string(present_version_blocked));
    output += " frame-backend-blocked=";
    output += std::string(to_bool_string(frame_backend_blocked));
    output += " present-backend-blocked=";
    output += std::string(to_bool_string(present_backend_blocked));
    output += " install{";
    output += install_summary;
    output += "}";
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
        .frame_tap_count = report.snapshot.frame_tap_count,
        .present_feedback_count = report.snapshot.present_feedback_count,
        .deadline_miss_count = report.snapshot.deadline_miss_count,
        .dropped_synthetic_count = report.snapshot.dropped_synthetic_count,
        .cadence_hz_millihz = report.snapshot.cadence_hz_millihz,
        .hud_text = report.hud_text,
    };
}

KcmNativeBridgeSnapshot KfiKcmBridge::native_bridge_install(
    const KwinNativeInstallContext& install_context
) const {
    const auto report = plugin_root_.observe_native_bridge_install(install_context);
    return KcmNativeBridgeSnapshot {
        .state = report.state,
        .frame_deferred_reason = report.frame_install_deferred_reason(),
        .present_deferred_reason = report.present_install_deferred_reason(),
        .frame_version_blocked = report.frame_preflight_version_blocked(),
        .present_version_blocked = report.present_preflight_version_blocked(),
        .frame_backend_blocked = report.frame_preflight_backend_blocked(),
        .present_backend_blocked = report.present_preflight_backend_blocked(),
        .install_summary = report.summary(),
    };
}

}  // namespace fluxma
