#include "fluxma_kcm_bridge.h"

namespace fluxma {

KcmSettingsSnapshot KcmSettingsSnapshot::from_config(const ModuleConfig& config) noexcept {
    return KcmSettingsSnapshot {
        .enabled = config.enabled,
        .mode = config.mode,
        .show_hud = config.show_hud,
        .subtitle_protection = config.subtitle_protection,
        .cursor_protection = config.cursor_protection,
        .log_interval_frames = config.log_interval_frames,
        .max_log_messages = config.max_log_messages,
    };
}

std::string KcmSettingsSnapshot::summary() const {
    std::string output;
    output += "enabled=";
    output += std::string(to_bool_string(enabled));
    output += " mode=";
    output += std::string(to_string(mode));
    output += " show-hud=";
    output += std::string(to_bool_string(show_hud));
    output += " subtitle-protection=";
    output += std::string(to_bool_string(subtitle_protection));
    output += " cursor-protection=";
    output += std::string(to_bool_string(cursor_protection));
    output += " log-interval=";
    output += std::to_string(log_interval_frames);
    output += " max-log-messages=";
    output += std::to_string(max_log_messages);
    return output;
}

std::string KcmRuntimeSnapshot::summary() const {
    std::string output;
    output += "state=";
    output += std::string(to_string(state));
    output += " bypass=";
    output += std::string(to_string(bypass_reason));
    output += " governor=";
    output += std::string(to_string(governor_mode));
    output += " scheduler=";
    output += std::string(to_string(scheduler_mode));
    output += " classifier=";
    output += std::string(to_bool_string(classifier_allows_interpolation));
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
    output += " cursor-passthrough=";
    output += std::string(to_bool_string(cursor_passthrough));
    output += " cursor-recomposite=";
    output += std::string(to_bool_string(cursor_recomposite));
    output += " subtitle-band=";
    output += std::string(to_bool_string(subtitle_band_active));
    output += " overlay-passthrough=";
    output += std::string(to_bool_string(overlay_passthrough));
    output += " protection-placeholder=";
    output += std::string(to_bool_string(protection_placeholder_only));
    output += " state-transitions=";
    output += std::to_string(state_transition_count);
    output += " frame-taps=";
    output += std::to_string(frame_tap_count);
    output += " present-feedback=";
    output += std::to_string(present_feedback_count);
    output += " deadline-miss=";
    output += std::to_string(deadline_miss_count);
    output += " synthetic-dropped=";
    output += std::to_string(dropped_synthetic_count);
    output += " last-presented-frame=";
    output += std::to_string(last_presented_frame_id);
    output += " last-presented-ts=";
    output += std::to_string(last_presented_timestamp_ns);
    output += " refresh-ns=";
    output += std::to_string(refresh_interval_ns);
    output += " target-present-ns=";
    output += std::to_string(last_target_presentation_timestamp_ns);
    output += " predicted-render-ns=";
    output += std::to_string(last_predicted_render_time_ns);
    output += " present-mode=";
    output += std::string(to_string(last_presentation_mode));
    output += " content-type=";
    output += std::string(to_string(last_content_type));
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

std::string KcmNativeBringupSnapshot::summary() const {
    std::string output;
    output += "state=";
    output += std::string(to_string(state));
    output += " frame-complete=";
    output += std::string(to_bool_string(frame_complete));
    output += " present-complete=";
    output += std::string(to_bool_string(present_complete));
    output += " frame-unresolved=";
    output += std::string(to_bool_string(frame_has_unresolved));
    output += " present-unresolved=";
    output += std::string(to_bool_string(present_has_unresolved));
    output += " bringup{";
    output += bringup_summary;
    output += "}";
    return output;
}

std::string KcmNativeDiagnosticsSnapshot::summary() const {
    std::string output;
    output += "state=";
    output += std::string(to_string(state));
    output += " bringup-complete=";
    output += std::string(to_bool_string(bringup_complete));
    output += " unresolved-candidates=";
    output += std::string(to_bool_string(has_unresolved_candidates));
    output += " frame-gate-match=";
    output += std::string(to_bool_string(frame_gate_matches));
    output += " present-gate-match=";
    output += std::string(to_bool_string(present_gate_matches));
    output += " frame-blocked=";
    output += std::string(to_bool_string(frame_has_any_blocker));
    output += " present-blocked=";
    output += std::string(to_bool_string(present_has_any_blocker));
    output += " frame-install-deferred=";
    output += std::string(to_bool_string(frame_install_deferred));
    output += " present-install-deferred=";
    output += std::string(to_bool_string(present_install_deferred));
    output += " frame-version-blocked=";
    output += std::string(to_bool_string(frame_version_blocked));
    output += " present-version-blocked=";
    output += std::string(to_bool_string(present_version_blocked));
    output += " frame-backend-blocked=";
    output += std::string(to_bool_string(frame_backend_blocked));
    output += " present-backend-blocked=";
    output += std::string(to_bool_string(present_backend_blocked));
    output += " frame-deferred=";
    output += std::string(to_string(frame_deferred_reason));
    output += " present-deferred=";
    output += std::string(to_string(present_deferred_reason));
    output += " diagnostics{";
    output += diagnostics_summary;
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
        .governor_mode = report.snapshot.governor_mode,
        .scheduler_mode = report.snapshot.scheduler_mode,
        .classifier_allows_interpolation = report.snapshot.classifier_allows_interpolation,
        .protected_content = report.snapshot.protected_content,
        .passthrough_only = report.snapshot.passthrough_only,
        .synthetic_armed = report.synthetic_armed(),
        .synthetic_queued = report.synthetic_queued(),
        .synthetic_suppressed_by_protection = report.synthetic_suppressed_by_protection(),
        .cursor_passthrough = report.cursor_passthrough(),
        .cursor_recomposite = report.cursor_recomposite(),
        .subtitle_band_active = report.subtitle_band_active(),
        .overlay_passthrough = report.overlay_passthrough(),
        .protection_placeholder_only = report.protection_placeholder_only(),
        .state_transition_count = report.snapshot.state_transition_count,
        .frame_tap_count = report.snapshot.frame_tap_count,
        .present_feedback_count = report.snapshot.present_feedback_count,
        .deadline_miss_count = report.snapshot.deadline_miss_count,
        .dropped_synthetic_count = report.snapshot.dropped_synthetic_count,
        .last_presented_frame_id = report.snapshot.last_presented_frame_id,
        .last_presented_timestamp_ns = report.snapshot.last_presented_timestamp_ns,
        .refresh_interval_ns = report.snapshot.refresh_interval_ns,
        .last_target_presentation_timestamp_ns =
            report.snapshot.last_target_presentation_timestamp_ns,
        .last_predicted_render_time_ns = report.snapshot.last_predicted_render_time_ns,
        .last_presentation_mode = report.snapshot.last_presentation_mode,
        .last_content_type = report.snapshot.last_content_type,
        .cadence_hz_millihz = report.snapshot.cadence_hz_millihz,
        .hud_text = report.hud_text,
    };
}

KcmNativeBringupSnapshot KfiKcmBridge::native_bridge_bringup(
    const KwinCompositorFrameInputs& frame_inputs,
    const KwinPresentFeedbackInputs& present_inputs
) const {
    const auto report = plugin_root_.observe_native_bridge_bringup(frame_inputs, present_inputs);
    return KcmNativeBringupSnapshot {
        .state = report.state,
        .frame_complete = report.frame_complete(),
        .present_complete = report.present_complete(),
        .frame_has_unresolved = report.frame_has_unresolved(),
        .present_has_unresolved = report.present_has_unresolved(),
        .bringup_summary = report.combined_summary(),
    };
}

KcmNativeDiagnosticsSnapshot KfiKcmBridge::native_bridge_diagnostics(
    const KwinCompositorFrameInputs& frame_inputs,
    const KwinPresentFeedbackInputs& present_inputs,
    const KwinNativeInstallContext& install_context
) const {
    const auto report =
        plugin_root_.observe_native_bridge(frame_inputs, present_inputs, install_context);
    return KcmNativeDiagnosticsSnapshot {
        .state = report.state,
        .bringup_complete = report.bringup_complete(),
        .has_unresolved_candidates = report.bringup_has_unresolved_candidates(),
        .frame_gate_matches = report.frame_gate_matches(),
        .present_gate_matches = report.present_gate_matches(),
        .frame_has_any_blocker = report.frame_preflight_has_any_blocker(),
        .present_has_any_blocker = report.present_preflight_has_any_blocker(),
        .frame_install_deferred = report.frame_install_deferred(),
        .present_install_deferred = report.present_install_deferred(),
        .frame_version_blocked = report.frame_preflight_version_blocked(),
        .present_version_blocked = report.present_preflight_version_blocked(),
        .frame_backend_blocked = report.frame_preflight_backend_blocked(),
        .present_backend_blocked = report.present_preflight_backend_blocked(),
        .frame_deferred_reason = report.frame_install_deferred_reason(),
        .present_deferred_reason = report.present_install_deferred_reason(),
        .diagnostics_summary = report.summary(),
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
