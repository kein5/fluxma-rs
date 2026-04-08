#include "fluxma_plugin_root.h"

namespace fluxma {

bool KwinNativeBridgeObservationReport::is_placeholder_state() const noexcept {
    return state == KwinNativeBridgeState::PlaceholderOnly;
}

bool KwinNativeBridgeObservationReport::bringup_complete() const noexcept {
    return bringup.fully_populated();
}

bool KwinNativeBridgeObservationReport::frame_bringup_has_unresolved() const noexcept {
    return bringup.frame_has_unresolved();
}

bool KwinNativeBridgeObservationReport::present_bringup_has_unresolved() const noexcept {
    return bringup.present_has_unresolved();
}

bool KwinNativeBridgeObservationReport::bringup_has_unresolved_candidates() const noexcept {
    return bringup.has_unresolved_candidates();
}

bool KwinNativeBridgeObservationReport::frame_gate_matches() const noexcept {
    return frame_preflight_deferred_reason() == frame_install_deferred_reason();
}

bool KwinNativeBridgeObservationReport::present_gate_matches() const noexcept {
    return present_preflight_deferred_reason() == present_install_deferred_reason();
}

KwinNativeDeferredReason KwinNativeBridgeObservationReport::frame_preflight_deferred_reason(
) const noexcept {
    return preflight.frame.gate.deferred_reason;
}

KwinNativeDeferredReason KwinNativeBridgeObservationReport::present_preflight_deferred_reason(
) const noexcept {
    return preflight.present.gate.deferred_reason;
}

KwinNativeDeferredReason KwinNativeBridgeObservationReport::frame_install_deferred_reason(
) const noexcept {
    return install.frame.deferred_reason;
}

KwinNativeDeferredReason KwinNativeBridgeObservationReport::present_install_deferred_reason(
) const noexcept {
    return install.present.deferred_reason;
}

bool KwinNativeBridgeObservationReport::frame_preflight_version_blocked() const noexcept {
    return preflight.frame.gate.version_blocked;
}

bool KwinNativeBridgeObservationReport::present_preflight_version_blocked() const noexcept {
    return preflight.present.gate.version_blocked;
}

bool KwinNativeBridgeObservationReport::frame_preflight_backend_blocked() const noexcept {
    return preflight.frame.gate.backend_blocked;
}

bool KwinNativeBridgeObservationReport::present_preflight_backend_blocked() const noexcept {
    return preflight.present.gate.backend_blocked;
}

bool KwinNativeBridgeObservationReport::frame_preflight_has_any_blocker() const noexcept {
    return preflight.frame.gate.has_any_blocker();
}

bool KwinNativeBridgeObservationReport::present_preflight_has_any_blocker() const noexcept {
    return preflight.present.gate.has_any_blocker();
}

bool KwinNativeBridgeObservationReport::all_gates_match() const noexcept {
    return frame_gate_matches() && present_gate_matches();
}

bool KwinNativeBridgeObservationReport::frame_install_deferred() const noexcept {
    return install.frame.result == KwinNativeInstallResult::Deferred;
}

bool KwinNativeBridgeObservationReport::present_install_deferred() const noexcept {
    return install.present.result == KwinNativeInstallResult::Deferred;
}

bool KwinNativeBridgeObservationReport::all_installs_deferred() const noexcept {
    return frame_install_deferred() && present_install_deferred();
}

std::string KwinNativeBridgeObservationReport::bringup_summary() const {
    return bringup.combined_summary();
}

std::string KwinNativeBridgeObservationReport::preflight_summary() const {
    return preflight.summary();
}

std::string KwinNativeBridgeObservationReport::install_summary() const {
    return install.summary();
}

std::string KwinNativeBridgeObservationReport::summary() const {
    std::string output;
    output += "state=";
    output += std::string(to_string(state));
    output += " bringup{";
    output += bringup_summary();
    output += "} preflight{";
    output += preflight_summary();
    output += "} install{";
    output += install_summary();
    output += "}";
    return output;
}

bool KwinNativeBridgeInstallObservationReport::is_placeholder_state() const noexcept {
    return state == KwinNativeBridgeState::PlaceholderOnly;
}

bool KwinNativeBridgeInstallObservationReport::frame_gate_matches() const noexcept {
    return frame_preflight_deferred_reason() == frame_install_deferred_reason();
}

bool KwinNativeBridgeInstallObservationReport::present_gate_matches() const noexcept {
    return present_preflight_deferred_reason() == present_install_deferred_reason();
}

KwinNativeDeferredReason
KwinNativeBridgeInstallObservationReport::frame_preflight_deferred_reason() const noexcept {
    return preflight.frame.gate.deferred_reason;
}

KwinNativeDeferredReason
KwinNativeBridgeInstallObservationReport::present_preflight_deferred_reason() const noexcept {
    return preflight.present.gate.deferred_reason;
}

KwinNativeDeferredReason
KwinNativeBridgeInstallObservationReport::frame_install_deferred_reason() const noexcept {
    return install.frame.deferred_reason;
}

KwinNativeDeferredReason
KwinNativeBridgeInstallObservationReport::present_install_deferred_reason() const noexcept {
    return install.present.deferred_reason;
}

bool KwinNativeBridgeInstallObservationReport::frame_preflight_version_blocked() const noexcept {
    return preflight.frame.gate.version_blocked;
}

bool KwinNativeBridgeInstallObservationReport::present_preflight_version_blocked() const noexcept {
    return preflight.present.gate.version_blocked;
}

bool KwinNativeBridgeInstallObservationReport::frame_preflight_backend_blocked() const noexcept {
    return preflight.frame.gate.backend_blocked;
}

bool KwinNativeBridgeInstallObservationReport::present_preflight_backend_blocked() const noexcept {
    return preflight.present.gate.backend_blocked;
}

bool KwinNativeBridgeInstallObservationReport::frame_preflight_has_any_blocker() const noexcept {
    return preflight.frame.gate.has_any_blocker();
}

bool KwinNativeBridgeInstallObservationReport::present_preflight_has_any_blocker() const noexcept {
    return preflight.present.gate.has_any_blocker();
}

bool KwinNativeBridgeInstallObservationReport::all_gates_match() const noexcept {
    return frame_gate_matches() && present_gate_matches();
}

bool KwinNativeBridgeInstallObservationReport::frame_install_deferred() const noexcept {
    return install.frame.result == KwinNativeInstallResult::Deferred;
}

bool KwinNativeBridgeInstallObservationReport::present_install_deferred() const noexcept {
    return install.present.result == KwinNativeInstallResult::Deferred;
}

bool KwinNativeBridgeInstallObservationReport::all_installs_deferred() const noexcept {
    return frame_install_deferred() && present_install_deferred();
}

std::string KwinNativeBridgeInstallObservationReport::preflight_summary() const {
    return preflight.summary();
}

std::string KwinNativeBridgeInstallObservationReport::install_summary() const {
    return install.summary();
}

std::string KwinNativeBridgeInstallObservationReport::summary() const {
    std::string output;
    output += "state=";
    output += std::string(to_string(state));
    output += " preflight{";
    output += preflight_summary();
    output += "} install{";
    output += install_summary();
    output += "}";
    return output;
}

std::string OutputRuntimeObservationReport::summary() const {
    std::string output;
    output += "state=";
    output += std::string(to_string(snapshot.state));
    output += " bypass=";
    output += std::string(to_string(snapshot.bypass_reason));
    output += " synthetic-armed=";
    output += std::string(to_bool_string(synthetic_plan.armed));
    output += " synthetic-drop=";
    output += std::string(to_bool_string(synthetic_plan.should_drop));
    output += " synthetic-generated=";
    output += std::string(to_bool_string(synthetic_artifact.generated));
    output += " synthetic-placeholder=";
    output += std::string(to_bool_string(synthetic_artifact.placeholder_only));
    output += " synthetic-queued=";
    output += std::string(to_bool_string(synthetic_submission.queued));
    output += " synthetic-submission-drop=";
    output += std::string(to_bool_string(synthetic_submission.dropped));
    output += " synthetic-subtitle-current=";
    output += std::string(to_bool_string(synthetic_submission.prefer_current_in_subtitle_band));
    output += " cursor-passthrough=";
    output += std::string(to_bool_string(protection_plan.cursor_passthrough));
    output += " subtitle-band=";
    output += std::string(to_bool_string(protection_plan.subtitle_band_active));
    return output;
}

KfiPluginRoot::KfiPluginRoot(ModuleConfig config)
    : config_(config),
      primary_output_(0, config_),
      primary_output_hook_adapter_(0, primary_output_),
      native_bridge_(primary_output_hook_adapter_) {
    // TODO: This is a native-module skeleton only. Do not add QML effect entry points here.
}

KfiPluginRoot::KfiPluginRoot(ModuleConfig config, bool force_rust_core_unavailable_for_tests)
    : config_(config),
      primary_output_(0, config_, force_rust_core_unavailable_for_tests),
      primary_output_hook_adapter_(0, primary_output_),
      native_bridge_(primary_output_hook_adapter_) {
    // TODO: This constructor is a local test seam for bypass fallback only.
}

KfiOutputController& KfiPluginRoot::primary_output() noexcept {
    return primary_output_;
}

const KfiOutputController& KfiPluginRoot::primary_output() const noexcept {
    return primary_output_;
}

KfiKwinHookAdapter& KfiPluginRoot::primary_output_hook_adapter() noexcept {
    return primary_output_hook_adapter_;
}

const KfiKwinHookAdapter& KfiPluginRoot::primary_output_hook_adapter() const noexcept {
    return primary_output_hook_adapter_;
}

KfiKwinNativeBridge& KfiPluginRoot::native_bridge() noexcept {
    return native_bridge_;
}

const KfiKwinNativeBridge& KfiPluginRoot::native_bridge() const noexcept {
    return native_bridge_;
}

KwinNativeBringupReport KfiPluginRoot::observe_native_bridge_bringup(
    const KwinCompositorFrameInputs& frame_inputs,
    const KwinPresentFeedbackInputs& present_inputs
) const {
    return native_bridge_.build_report(frame_inputs, present_inputs);
}

KwinNativeBridgeObservationReport KfiPluginRoot::observe_native_bridge(
    const KwinCompositorFrameInputs& frame_inputs,
    const KwinPresentFeedbackInputs& present_inputs,
    const KwinNativeInstallContext& install_context
) const {
    const auto bringup = native_bridge_.build_report(frame_inputs, present_inputs);
    const auto preflight = native_bridge_.preflight_install(install_context);
    const auto install = native_bridge_.install_stub(install_context);
    return KwinNativeBridgeObservationReport {
        .state = native_bridge_.state(),
        .bringup = bringup,
        .preflight = preflight,
        .install = install,
    };
}

KwinNativeBridgeInstallObservationReport KfiPluginRoot::observe_native_bridge_install(
    const KwinNativeInstallContext& install_context
) const {
    const auto preflight = native_bridge_.preflight_install(install_context);
    const auto install = native_bridge_.install_stub(install_context);
    return KwinNativeBridgeInstallObservationReport {
        .state = native_bridge_.state(),
        .preflight = preflight,
        .install = install,
    };
}

OutputRuntimeObservationReport KfiPluginRoot::observe_output_runtime(std::uint64_t now_ns) const {
    const auto sample = primary_output_.sample_runtime(now_ns);
    return OutputRuntimeObservationReport {
        .snapshot = sample.snapshot,
        .synthetic_plan = sample.synthetic_plan,
        .synthetic_artifact = sample.synthetic_artifact,
        .synthetic_submission = sample.synthetic_submission,
        .protection_plan = sample.protection_plan,
        .hud_text = primary_output_.render_hud_text(sample),
    };
}

}  // namespace fluxma
