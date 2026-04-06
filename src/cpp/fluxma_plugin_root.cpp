#include "fluxma_plugin_root.h"

namespace fluxma {

bool KwinNativeBridgeObservationReport::is_placeholder_state() const noexcept {
    return state == KwinNativeBridgeState::PlaceholderOnly;
}

std::string KwinNativeBridgeObservationReport::summary() const {
    std::string output;
    output += "state=";
    output += std::string(to_string(state));
    output += " bringup{";
    output += bringup.combined_summary();
    output += "} preflight{";
    output += preflight.summary();
    output += "} install{";
    output += install.summary();
    output += "}";
    return output;
}

bool KwinNativeBridgeInstallObservationReport::is_placeholder_state() const noexcept {
    return state == KwinNativeBridgeState::PlaceholderOnly;
}

bool KwinNativeBridgeInstallObservationReport::frame_gate_matches() const noexcept {
    return preflight.frame.gate.deferred_reason == install.frame.deferred_reason;
}

bool KwinNativeBridgeInstallObservationReport::present_gate_matches() const noexcept {
    return preflight.present.gate.deferred_reason == install.present.deferred_reason;
}

bool KwinNativeBridgeInstallObservationReport::all_gates_match() const noexcept {
    return frame_gate_matches() && present_gate_matches();
}

std::string KwinNativeBridgeInstallObservationReport::summary() const {
    std::string output;
    output += "state=";
    output += std::string(to_string(state));
    output += " preflight{";
    output += preflight.summary();
    output += "} install{";
    output += install.summary();
    output += "}";
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

}  // namespace fluxma
