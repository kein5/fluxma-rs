#pragma once

#include "fluxma_config.h"
#include "fluxma_kwin_hook_adapter.h"
#include "fluxma_kwin_native_bridge.h"
#include "fluxma_output_controller.h"

namespace fluxma {

struct KwinNativeBridgeObservationReport {
    KwinNativeBridgeState state = KwinNativeBridgeState::PlaceholderOnly;
    KwinNativeBringupReport bringup {};
    KwinNativeCombinedPreflightReport preflight {};
    KwinNativeCombinedInstallReport install {};

    // Use structured fields/helpers for code paths; keep summary() for logs and human diagnostics.
    [[nodiscard]] bool is_placeholder_state() const noexcept;
    // Shorthand for "bring-up has all required fields populated for both frame and present".
    [[nodiscard]] bool bringup_complete() const noexcept;
    // These are passthrough helpers over bringup.*_has_unresolved() so callers can stay on
    // the observation report without unpacking nested bring-up state.
    [[nodiscard]] bool frame_bringup_has_unresolved() const noexcept;
    [[nodiscard]] bool present_bringup_has_unresolved() const noexcept;
    [[nodiscard]] bool bringup_has_unresolved_candidates() const noexcept;
    [[nodiscard]] bool frame_gate_matches() const noexcept;
    [[nodiscard]] bool present_gate_matches() const noexcept;
    [[nodiscard]] KwinNativeDeferredReason frame_preflight_deferred_reason() const noexcept;
    [[nodiscard]] KwinNativeDeferredReason present_preflight_deferred_reason() const noexcept;
    [[nodiscard]] KwinNativeDeferredReason frame_install_deferred_reason() const noexcept;
    [[nodiscard]] KwinNativeDeferredReason present_install_deferred_reason() const noexcept;
    [[nodiscard]] bool frame_preflight_version_blocked() const noexcept;
    [[nodiscard]] bool present_preflight_version_blocked() const noexcept;
    [[nodiscard]] bool frame_preflight_backend_blocked() const noexcept;
    [[nodiscard]] bool present_preflight_backend_blocked() const noexcept;
    [[nodiscard]] bool frame_preflight_has_any_blocker() const noexcept;
    [[nodiscard]] bool present_preflight_has_any_blocker() const noexcept;
    // This checks preflight/install consistency for both frame and present independently.
    [[nodiscard]] bool all_gates_match() const noexcept;
    // These reflect the current skeleton contract where native bridge install remains deferred.
    [[nodiscard]] bool frame_install_deferred() const noexcept;
    [[nodiscard]] bool present_install_deferred() const noexcept;
    [[nodiscard]] bool all_installs_deferred() const noexcept;
    // These are passthrough helpers over the nested reports for test/log callers that do not
    // want to unpack the structured subreports first.
    [[nodiscard]] std::string bringup_summary() const;
    [[nodiscard]] std::string preflight_summary() const;
    [[nodiscard]] std::string install_summary() const;
    [[nodiscard]] std::string summary() const;
};

struct KwinNativeBridgeInstallObservationReport {
    KwinNativeBridgeState state = KwinNativeBridgeState::PlaceholderOnly;
    KwinNativeCombinedPreflightReport preflight {};
    KwinNativeCombinedInstallReport install {};

    // Use structured fields/helpers for code paths; keep summary() for logs and human diagnostics.
    [[nodiscard]] bool is_placeholder_state() const noexcept;
    [[nodiscard]] bool frame_gate_matches() const noexcept;
    [[nodiscard]] bool present_gate_matches() const noexcept;
    [[nodiscard]] KwinNativeDeferredReason frame_preflight_deferred_reason() const noexcept;
    [[nodiscard]] KwinNativeDeferredReason present_preflight_deferred_reason() const noexcept;
    [[nodiscard]] KwinNativeDeferredReason frame_install_deferred_reason() const noexcept;
    [[nodiscard]] KwinNativeDeferredReason present_install_deferred_reason() const noexcept;
    [[nodiscard]] bool frame_preflight_version_blocked() const noexcept;
    [[nodiscard]] bool present_preflight_version_blocked() const noexcept;
    [[nodiscard]] bool frame_preflight_backend_blocked() const noexcept;
    [[nodiscard]] bool present_preflight_backend_blocked() const noexcept;
    [[nodiscard]] bool frame_preflight_has_any_blocker() const noexcept;
    [[nodiscard]] bool present_preflight_has_any_blocker() const noexcept;
    // This checks preflight/install consistency for both frame and present independently.
    [[nodiscard]] bool all_gates_match() const noexcept;
    // These reflect the current skeleton contract where native bridge install remains deferred.
    [[nodiscard]] bool frame_install_deferred() const noexcept;
    [[nodiscard]] bool present_install_deferred() const noexcept;
    [[nodiscard]] bool all_installs_deferred() const noexcept;
    // These are passthrough helpers over the nested reports for test/log callers that do not
    // want to unpack the structured subreports first.
    [[nodiscard]] std::string preflight_summary() const;
    [[nodiscard]] std::string install_summary() const;
    [[nodiscard]] std::string summary() const;
};

struct OutputRuntimeObservationReport {
    MetricsSnapshot snapshot {};
    SyntheticFramePlan synthetic_plan {};
    SyntheticFrameArtifact synthetic_artifact {};
    std::string hud_text {};

    [[nodiscard]] bool synthetic_armed() const noexcept {
        return synthetic_plan.armed;
    }

    [[nodiscard]] bool synthetic_should_drop() const noexcept {
        return synthetic_plan.should_drop;
    }

    [[nodiscard]] bool synthetic_generated() const noexcept {
        return synthetic_artifact.generated;
    }

    [[nodiscard]] std::string summary() const;
};

class KfiPluginRoot {
  public:
    explicit KfiPluginRoot(ModuleConfig config = {});
    KfiPluginRoot(ModuleConfig config, bool force_rust_core_unavailable_for_tests);

    [[nodiscard]] KfiOutputController& primary_output() noexcept;
    [[nodiscard]] const KfiOutputController& primary_output() const noexcept;
    [[nodiscard]] KfiKwinHookAdapter& primary_output_hook_adapter() noexcept;
    [[nodiscard]] const KfiKwinHookAdapter& primary_output_hook_adapter() const noexcept;
    [[nodiscard]] KfiKwinNativeBridge& native_bridge() noexcept;
    [[nodiscard]] const KfiKwinNativeBridge& native_bridge() const noexcept;
    // Use this when only hook input completeness/readiness matters and install gate state is irrelevant.
    [[nodiscard]] KwinNativeBringupReport observe_native_bridge_bringup(
        const KwinCompositorFrameInputs& frame_inputs,
        const KwinPresentFeedbackInputs& present_inputs
    ) const;
    [[nodiscard]] KwinNativeBridgeObservationReport observe_native_bridge(
        const KwinCompositorFrameInputs& frame_inputs,
        const KwinPresentFeedbackInputs& present_inputs,
        const KwinNativeInstallContext& install_context
    ) const;
    [[nodiscard]] KwinNativeBridgeInstallObservationReport observe_native_bridge_install(
        const KwinNativeInstallContext& install_context
    ) const;
    [[nodiscard]] OutputRuntimeObservationReport observe_output_runtime(
        std::uint64_t now_ns
    ) const;

  private:
    ModuleConfig config_ {};
    KfiOutputController primary_output_;
    KfiKwinHookAdapter primary_output_hook_adapter_;
    KfiKwinNativeBridge native_bridge_;
};

}  // namespace fluxma
