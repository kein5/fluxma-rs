#include <cstdlib>
#include <iostream>

#include "fluxma_kwin_hook_builders.h"
#include "fluxma_plugin_root.h"

namespace {

fluxma::KwinCompositorFrameInputs complete_frame_inputs() {
    return fluxma::KwinCompositorFrameInputs {
        .output_id = 0,
        .frame_id = 11,
        .timestamp_ns = 1'000'000,
        .target_presentation_timestamp_ns = 16'666'667,
        .predicted_render_time_ns = 2'000'000,
        .content_type = fluxma::ContentType::Video,
        .width = 1920,
        .height = 1080,
        .gpu_handle = fluxma::GpuFrameHandle {.backend_kind = 0, .handle_id = 41},
    };
}

fluxma::KwinPresentFeedbackInputs complete_present_inputs() {
    return fluxma::KwinPresentFeedbackInputs {
        .output_id = 0,
        .frame_id = 11,
        .presented_timestamp_ns = 2'000'000,
        .refresh_interval_ns = 16'666'667,
        .presentation_mode = fluxma::PresentationMode::VSync,
        .present_success = true,
        .dropped_synthetic = false,
    };
}

}  // namespace

int main() {
    fluxma::KfiPluginRoot plugin_root(
        {.enabled = true, .show_hud = true, .log_interval_frames = 1, .max_log_messages = 4}
    );

    const auto bringup_only = plugin_root.observe_native_bridge_bringup(
        complete_frame_inputs(),
        complete_present_inputs()
    );
    if (bringup_only.state != fluxma::KwinNativeBridgeState::PlaceholderOnly ||
        !bringup_only.frame_complete() || !bringup_only.present_complete() ||
        !bringup_only.fully_populated() || !bringup_only.frame_has_unresolved() ||
        !bringup_only.present_has_unresolved() || !bringup_only.has_unresolved_candidates() ||
        bringup_only.frame.plan.hook_point !=
            fluxma::KwinFrameHookPoint::CompositorOutputFrameReady ||
        bringup_only.present.plan.hook_point !=
            fluxma::KwinPresentHookPoint::OutputFramePresented ||
        bringup_only.combined_summary().find("bringup") != std::string::npos ||
        bringup_only.combined_summary().find("frame{hook=compositor-output-frame-ready") ==
            std::string::npos) {
        std::cerr << "plugin root bringup-only observation must preserve native bridge readiness\n";
        return EXIT_FAILURE;
    }

    const auto version_gate_observation = plugin_root.observe_native_bridge(
        complete_frame_inputs(),
        complete_present_inputs(),
        fluxma::KwinNativeInstallContext {
            .kwin_version_supported = false,
            .backend_supported = true,
            .kwin_version = "6.3.81",
            .backend_name = "drm",
        }
    );
    const auto version_gate_summary = version_gate_observation.summary();
    const auto version_gate_frame_summary = version_gate_observation.bringup.frame_summary();
    if (!version_gate_observation.is_placeholder_state() ||
        !version_gate_observation.bringup.is_placeholder_state() ||
        !version_gate_observation.bringup.frame_complete() ||
        !version_gate_observation.bringup.present_complete() ||
        !version_gate_observation.bringup_complete() ||
        !version_gate_observation.frame_bringup_has_unresolved() ||
        !version_gate_observation.present_bringup_has_unresolved() ||
        !version_gate_observation.bringup_has_unresolved_candidates() ||
        version_gate_observation.frame_bringup_has_unresolved() !=
            version_gate_observation.bringup.frame_has_unresolved() ||
        version_gate_observation.present_bringup_has_unresolved() !=
            version_gate_observation.bringup.present_has_unresolved() ||
        version_gate_observation.bringup_has_unresolved_candidates() !=
            version_gate_observation.bringup.has_unresolved_candidates() ||
        !version_gate_observation.frame_gate_matches() ||
        !version_gate_observation.present_gate_matches() ||
        !version_gate_observation.all_gates_match() ||
        !version_gate_observation.all_installs_deferred() ||
        version_gate_observation.bringup.frame.plan.hook_point !=
            fluxma::KwinFrameHookPoint::CompositorOutputFrameReady ||
        version_gate_frame_summary.find(
            "hook=compositor-output-frame-ready"
        ) == std::string::npos ||
        version_gate_observation.preflight.frame.gate.deferred_reason !=
            fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        version_gate_observation.preflight.present.gate.deferred_reason !=
            fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        version_gate_observation.install.frame.deferred_reason !=
            fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        version_gate_observation.install.present.deferred_reason !=
            fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        version_gate_observation.preflight.present.source_file != "src/core/renderbackend.cpp" ||
        version_gate_observation.preflight.present.symbol != "OutputFrame::presented(...)" ||
        version_gate_observation.preflight.present.checklist_hint_secondary !=
            "confirm backend completion timestamp semantics across backends" ||
        version_gate_observation.install.present.source_file != "src/core/renderbackend.cpp" ||
        version_gate_observation.install.present.symbol != "OutputFrame::presented(...)" ||
        version_gate_observation.install.present.checklist_hint_secondary !=
            "confirm backend completion timestamp semantics across backends" ||
        version_gate_summary.find("state=") == std::string::npos ||
        version_gate_summary.find("bringup{") == std::string::npos ||
        version_gate_summary.find("preflight{") ==
            std::string::npos ||
        version_gate_summary.find("install{") == std::string::npos ||
        version_gate_summary.find("present{") ==
            std::string::npos) {
        std::cerr << "plugin root observation must surface version gate state\n";
        return EXIT_FAILURE;
    }

    const auto backend_gate_observation = plugin_root.observe_native_bridge(
        complete_frame_inputs(),
        complete_present_inputs(),
        fluxma::KwinNativeInstallContext {
            .kwin_version_supported = true,
            .backend_supported = false,
            .kwin_version = "6.3.82",
            .backend_name = "wayland",
        }
    );
    if (backend_gate_observation.preflight.frame.gate.deferred_reason !=
            fluxma::KwinNativeDeferredReason::BackendGate ||
        backend_gate_observation.preflight.present.gate.deferred_reason !=
            fluxma::KwinNativeDeferredReason::BackendGate ||
        backend_gate_observation.install.frame.deferred_reason !=
            fluxma::KwinNativeDeferredReason::BackendGate ||
        backend_gate_observation.install.present.deferred_reason !=
            fluxma::KwinNativeDeferredReason::BackendGate ||
        !backend_gate_observation.all_gates_match() ||
        backend_gate_observation.preflight.present.source_file != "src/core/renderbackend.cpp" ||
        backend_gate_observation.preflight.present.symbol != "OutputFrame::presented(...)" ||
        backend_gate_observation.install.present.source_file != "src/core/renderbackend.cpp" ||
        backend_gate_observation.install.present.symbol != "OutputFrame::presented(...)" ||
        backend_gate_observation.summary().find("preflight{") ==
            std::string::npos ||
        backend_gate_observation.summary().find(
            "reason=backend gate blocked install for wayland"
        ) == std::string::npos) {
        std::cerr << "plugin root observation must surface backend gate state\n";
        return EXIT_FAILURE;
    }

    const auto precedence_observation = plugin_root.observe_native_bridge(
        complete_frame_inputs(),
        complete_present_inputs(),
        fluxma::KwinNativeInstallContext {
            .kwin_version_supported = false,
            .backend_supported = false,
            .kwin_version = "6.3.82",
            .backend_name = "wayland",
        }
    );
    if (precedence_observation.preflight.frame.gate.deferred_reason !=
            fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        precedence_observation.preflight.present.gate.deferred_reason !=
            fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        precedence_observation.install.frame.deferred_reason !=
            fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        precedence_observation.install.present.deferred_reason !=
            fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        !precedence_observation.all_gates_match() ||
        precedence_observation.summary().find("reason=kwin version gate blocked install for 6.3.82")
            == std::string::npos) {
        std::cerr << "version gate must remain the first deferred reason\n";
        return EXIT_FAILURE;
    }

    const auto install_only_observation = plugin_root.observe_native_bridge_install(
        fluxma::KwinNativeInstallContext {
            .kwin_version_supported = true,
            .backend_supported = false,
            .kwin_version = "6.3.83",
            .backend_name = "wayland",
        }
    );
    const auto install_only_summary = install_only_observation.summary();
    if (!install_only_observation.is_placeholder_state() ||
        install_only_observation.preflight.frame.gate.deferred_reason !=
            fluxma::KwinNativeDeferredReason::BackendGate ||
        install_only_observation.preflight.present.gate.deferred_reason !=
            fluxma::KwinNativeDeferredReason::BackendGate ||
        install_only_observation.preflight.present.source_file !=
            "src/core/renderbackend.cpp" ||
        install_only_observation.install.frame.deferred_reason !=
            fluxma::KwinNativeDeferredReason::BackendGate ||
        install_only_observation.install.present.deferred_reason !=
            fluxma::KwinNativeDeferredReason::BackendGate ||
        install_only_observation.install.present.source_file !=
            "src/core/renderbackend.cpp" ||
        install_only_observation.install.present.checklist_hint_secondary !=
            "confirm backend completion timestamp semantics across backends" ||
        !install_only_observation.frame_gate_matches() ||
        !install_only_observation.present_gate_matches() ||
        !install_only_observation.all_gates_match() ||
        !install_only_observation.all_installs_deferred() ||
        install_only_summary.find("state=") == std::string::npos ||
        install_only_summary.find("preflight{") == std::string::npos ||
        install_only_summary.find("install{") == std::string::npos ||
        install_only_summary.find("frame{") == std::string::npos ||
        install_only_summary.find("present{") == std::string::npos) {
        std::cerr << "plugin root install observation must preserve backend gate diagnostics\n";
        return EXIT_FAILURE;
    }

    const auto install_only_placeholder = plugin_root.observe_native_bridge_install(
        fluxma::KwinNativeInstallContext {}
    );
    if (!install_only_placeholder.is_placeholder_state() ||
        install_only_placeholder.preflight.frame.gate.deferred_reason !=
            fluxma::KwinNativeDeferredReason::PlaceholderOnly ||
        install_only_placeholder.preflight.present.gate.deferred_reason !=
            fluxma::KwinNativeDeferredReason::PlaceholderOnly ||
        install_only_placeholder.install.frame.deferred_reason !=
            fluxma::KwinNativeDeferredReason::PlaceholderOnly ||
        install_only_placeholder.install.present.deferred_reason !=
            fluxma::KwinNativeDeferredReason::PlaceholderOnly ||
        !install_only_placeholder.all_gates_match() ||
        install_only_placeholder.summary().find("state=") ==
            std::string::npos) {
        std::cerr << "plugin root install observation must preserve placeholder diagnostics\n";
        return EXIT_FAILURE;
    }

    const auto install_only_version_gate = plugin_root.observe_native_bridge_install(
        fluxma::KwinNativeInstallContext {
            .kwin_version_supported = false,
            .backend_supported = true,
            .kwin_version = "6.3.84",
            .backend_name = "drm",
        }
    );
    if (install_only_version_gate.preflight.frame.gate.deferred_reason !=
            fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        install_only_version_gate.preflight.present.gate.deferred_reason !=
            fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        install_only_version_gate.install.frame.deferred_reason !=
            fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        install_only_version_gate.install.present.deferred_reason !=
            fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        !install_only_version_gate.all_gates_match() ||
        !install_only_version_gate.all_installs_deferred() ||
        install_only_version_gate.preflight.present.symbol != "OutputFrame::presented(...)" ||
        install_only_version_gate.install.present.checklist_hint_secondary !=
            "confirm backend completion timestamp semantics across backends" ||
        install_only_version_gate.summary().find(
            "reason=kwin version gate blocked install for 6.3.84"
        ) == std::string::npos) {
        std::cerr << "plugin root install observation must preserve version gate diagnostics\n";
        return EXIT_FAILURE;
    }

    const auto install_only_precedence = plugin_root.observe_native_bridge_install(
        fluxma::KwinNativeInstallContext {
            .kwin_version_supported = false,
            .backend_supported = false,
            .kwin_version = "6.3.85",
            .backend_name = "wayland",
        }
    );
    if (install_only_precedence.preflight.frame.gate.deferred_reason !=
            fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        install_only_precedence.preflight.present.gate.deferred_reason !=
            fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        install_only_precedence.install.frame.deferred_reason !=
            fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        install_only_precedence.install.present.deferred_reason !=
            fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        !install_only_precedence.all_gates_match() ||
        !install_only_precedence.all_installs_deferred() ||
        install_only_precedence.summary().find(
            "reason=kwin version gate blocked install for 6.3.85"
        ) == std::string::npos) {
        std::cerr << "plugin root install observation must keep version gate precedence\n";
        return EXIT_FAILURE;
    }

    auto incomplete_frame = complete_frame_inputs();
    incomplete_frame.frame_id = 0;
    incomplete_frame.width = 0;
    auto incomplete_present = complete_present_inputs();
    incomplete_present.frame_id = 0;
    incomplete_present.refresh_interval_ns = 0;
    const auto incomplete_observation = plugin_root.observe_native_bridge(
        incomplete_frame,
        incomplete_present,
        fluxma::KwinNativeInstallContext {}
    );
    const auto incomplete_summary = incomplete_observation.summary();
    const auto incomplete_frame_summary = incomplete_observation.bringup.frame_summary();
    const auto incomplete_present_summary = incomplete_observation.bringup.present_summary();
    if (incomplete_observation.bringup.frame_complete() ||
        incomplete_observation.bringup.present_complete() ||
        incomplete_observation.bringup_complete() ||
        !incomplete_observation.frame_bringup_has_unresolved() ||
        !incomplete_observation.present_bringup_has_unresolved() ||
        !incomplete_observation.bringup_has_unresolved_candidates() ||
        incomplete_observation.frame_bringup_has_unresolved() !=
            incomplete_observation.bringup.frame_has_unresolved() ||
        incomplete_observation.present_bringup_has_unresolved() !=
            incomplete_observation.bringup.present_has_unresolved() ||
        incomplete_observation.bringup_has_unresolved_candidates() !=
            incomplete_observation.bringup.has_unresolved_candidates() ||
        incomplete_frame_summary.find("ready=no") == std::string::npos ||
        incomplete_frame_summary.find("missing=frame-id,width") ==
            std::string::npos ||
        incomplete_present_summary.find(
            "missing=frame-id,refresh-interval-ns"
        ) ==
            std::string::npos ||
        incomplete_observation.preflight.frame.gate.deferred_reason !=
            fluxma::KwinNativeDeferredReason::PlaceholderOnly ||
        !incomplete_observation.all_gates_match() ||
        incomplete_summary.find("preflight{") ==
            std::string::npos ||
        incomplete_summary.find("install{") == std::string::npos) {
        std::cerr << "plugin root observation must preserve incomplete bringup diagnostics\n"
                  << "frame_summary=" << incomplete_frame_summary << '\n'
                  << "present_summary=" << incomplete_present_summary << '\n'
                  << "summary=" << incomplete_summary << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
