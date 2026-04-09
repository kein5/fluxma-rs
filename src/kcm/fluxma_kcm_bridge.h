#pragma once

#include <cstdint>
#include <string>

#include "fluxma_plugin_root.h"

namespace fluxma {

struct KcmSettingsSnapshot {
    bool enabled = true;
    ModuleMode mode = ModuleMode::Synthetic2x;
    bool show_hud = true;
    bool subtitle_protection = true;
    bool cursor_protection = true;
    std::uint64_t log_interval_frames = 0;
    std::size_t max_log_messages = 0;

    [[nodiscard]] static KcmSettingsSnapshot from_config(const ModuleConfig& config) noexcept;
    [[nodiscard]] std::string summary() const;
};

struct KcmRuntimeSnapshot {
    OutputState state = OutputState::Bypass;
    BypassReason bypass_reason = BypassReason::None;
    GovernorMode governor_mode = GovernorMode::Bypass;
    SchedulerMode scheduler_mode = SchedulerMode::PassthroughOnly;
    bool classifier_allows_interpolation = false;
    bool protected_content = false;
    bool passthrough_only = true;
    bool synthetic_armed = false;
    bool synthetic_queued = false;
    bool synthetic_suppressed_by_protection = false;
    bool cursor_passthrough = false;
    bool cursor_recomposite = false;
    bool subtitle_band_active = false;
    bool overlay_passthrough = false;
    bool protection_placeholder_only = false;
    std::uint64_t state_transition_count = 0;
    std::uint64_t frame_tap_count = 0;
    std::uint64_t present_feedback_count = 0;
    std::uint64_t deadline_miss_count = 0;
    std::uint64_t dropped_synthetic_count = 0;
    std::uint64_t last_presented_frame_id = 0;
    std::uint64_t last_presented_timestamp_ns = 0;
    std::uint64_t refresh_interval_ns = 0;
    std::uint64_t last_target_presentation_timestamp_ns = 0;
    std::uint64_t last_predicted_render_time_ns = 0;
    PresentationMode last_presentation_mode = PresentationMode::VSync;
    ContentType last_content_type = ContentType::None;
    std::uint32_t cadence_hz_millihz = 0;
    std::string hud_text {};

    [[nodiscard]] static KcmRuntimeSnapshot from_observation(
        const OutputRuntimeObservationReport& report
    ) noexcept;
    [[nodiscard]] std::string summary() const;
};

struct KcmNativeBridgeSnapshot {
    KwinNativeBridgeState state = KwinNativeBridgeState::PlaceholderOnly;
    bool all_gates_match = false;
    bool all_installs_deferred = false;
    KwinNativeDeferredReason frame_deferred_reason = KwinNativeDeferredReason::PlaceholderOnly;
    KwinNativeDeferredReason present_deferred_reason = KwinNativeDeferredReason::PlaceholderOnly;
    bool frame_version_blocked = false;
    bool present_version_blocked = false;
    bool frame_backend_blocked = false;
    bool present_backend_blocked = false;
    std::string install_summary {};

    [[nodiscard]] static KcmNativeBridgeSnapshot from_observation(
        const KwinNativeBridgeInstallObservationReport& report
    ) noexcept;
    [[nodiscard]] std::string summary() const;
};

struct KcmNativeBringupSnapshot {
    KwinNativeBridgeState state = KwinNativeBridgeState::PlaceholderOnly;
    bool frame_complete = false;
    bool present_complete = false;
    bool frame_has_unresolved = false;
    bool present_has_unresolved = false;
    std::string bringup_summary {};

    [[nodiscard]] static KcmNativeBringupSnapshot from_report(
        const KwinNativeBringupReport& report
    ) noexcept;
    [[nodiscard]] std::string summary() const;
};

struct KcmNativeDiagnosticsSnapshot {
    KwinNativeBridgeState state = KwinNativeBridgeState::PlaceholderOnly;
    bool bringup_complete = false;
    bool has_unresolved_candidates = false;
    bool all_gates_match = false;
    bool all_installs_deferred = false;
    bool frame_gate_matches = false;
    bool present_gate_matches = false;
    bool frame_has_any_blocker = false;
    bool present_has_any_blocker = false;
    bool frame_install_deferred = false;
    bool present_install_deferred = false;
    bool frame_version_blocked = false;
    bool present_version_blocked = false;
    bool frame_backend_blocked = false;
    bool present_backend_blocked = false;
    KwinNativeDeferredReason frame_deferred_reason = KwinNativeDeferredReason::PlaceholderOnly;
    KwinNativeDeferredReason present_deferred_reason = KwinNativeDeferredReason::PlaceholderOnly;
    std::string diagnostics_summary {};

    [[nodiscard]] static KcmNativeDiagnosticsSnapshot from_observation(
        const KwinNativeBridgeObservationReport& report
    ) noexcept;
    [[nodiscard]] std::string summary() const;
};

class KfiKcmBridge {
  public:
    explicit KfiKcmBridge(const KfiPluginRoot& plugin_root) noexcept;

    [[nodiscard]] KcmSettingsSnapshot settings() const noexcept;
    [[nodiscard]] KcmRuntimeSnapshot runtime(std::uint64_t now_ns) const;
    [[nodiscard]] KcmNativeBringupSnapshot native_bridge_bringup(
        const KwinCompositorFrameInputs& frame_inputs,
        const KwinPresentFeedbackInputs& present_inputs
    ) const;
    [[nodiscard]] KcmNativeDiagnosticsSnapshot native_bridge_diagnostics(
        const KwinCompositorFrameInputs& frame_inputs,
        const KwinPresentFeedbackInputs& present_inputs,
        const KwinNativeInstallContext& install_context
    ) const;
    [[nodiscard]] KcmNativeBridgeSnapshot native_bridge_install(
        const KwinNativeInstallContext& install_context
    ) const;

  private:
    const KfiPluginRoot& plugin_root_;
};

}  // namespace fluxma
