#include <cstdlib>
#include <iostream>

#include "fluxma_kwin_hook_builders.h"
#include "fluxma_kwin_hook_candidates.h"
#include "fluxma_plugin_root.h"

int main() {
    fluxma::KfiPluginRoot plugin_root(
        {.enabled = true, .show_hud = true, .log_interval_frames = 1, .max_log_messages = 4}
    );

    const auto& bridge = plugin_root.native_bridge();
    if (bridge.is_installed() ||
        bridge.state() != fluxma::KwinNativeBridgeState::PlaceholderOnly ||
        fluxma::to_string(bridge.state()) != "placeholder-only") {
        std::cerr << "native bridge must remain placeholder-only until real hook landing\n";
        return EXIT_FAILURE;
    }

    const auto frame_candidate = bridge.frame_candidate();
    const auto present_candidate = bridge.present_candidate();
    if (frame_candidate.hook_point != fluxma::KwinFrameHookPoint::CompositorOutputFrameReady ||
        present_candidate.hook_point != fluxma::KwinPresentHookPoint::OutputFramePresented) {
        std::cerr << "native bridge must reuse adapter preferred candidates\n";
        return EXIT_FAILURE;
    }
    if (bridge.frame_installer_entry() != "KfiKwinNativeBridge::install_frame_stub" ||
        bridge.present_installer_entry() != "KfiKwinNativeBridge::install_present_stub") {
        std::cerr << "native bridge must expose installer entry names\n";
        return EXIT_FAILURE;
    }
    const auto frame_checklist = bridge.frame_checklist();
    const auto present_checklist = bridge.present_checklist();
    if (frame_checklist[0] != "confirm final composed frame-id provenance" ||
        present_checklist[0] != "confirm presented frame-id still correlates with submitted frame") {
        std::cerr << "native bridge must expose preferred candidate checklists\n";
        return EXIT_FAILURE;
    }

    const auto report = bridge.build_report(
        fluxma::KwinCompositorFrameInputs {
            .output_id = 0,
            .frame_id = 1,
            .timestamp_ns = 2,
            .width = 1920,
            .height = 1080,
            .gpu_handle = fluxma::GpuFrameHandle {.backend_kind = 0, .handle_id = 7},
        },
        fluxma::KwinPresentFeedbackInputs {
            .output_id = 0,
            .frame_id = 1,
            .presented_timestamp_ns = 2,
            .refresh_interval_ns = 16'666'667,
        }
    );
    if (report.state != fluxma::KwinNativeBridgeState::PlaceholderOnly ||
        report.frame_summary.find("hook=compositor-output-frame-ready") == std::string::npos ||
        report.present_summary.find("hook=output-frame-presented") == std::string::npos) {
        std::cerr << "native bridge report must surface preferred candidate summaries\n";
        return EXIT_FAILURE;
    }
    const auto combined = report.combined_summary();
    if (combined.find("state=placeholder-only") == std::string::npos ||
        combined.find("frame{hook=compositor-output-frame-ready") == std::string::npos ||
        combined.find("present{hook=output-frame-presented") == std::string::npos) {
        std::cerr << "native bridge combined summary must stay stable\n";
        return EXIT_FAILURE;
    }

    const auto frame_install = bridge.install_frame_stub();
    const auto present_install = bridge.install_present_stub();
    if (frame_install.result != fluxma::KwinNativeInstallResult::Deferred ||
        frame_install.deferred_reason != fluxma::KwinNativeDeferredReason::PlaceholderOnly ||
        frame_install.reason != "native bridge is still placeholder-only" ||
        frame_install.context_summary !=
            "kwin=unspecified backend=unspecified version_supported=true backend_supported=true" ||
        frame_install.target != "compositor-output-frame-ready" ||
        frame_install.installer_entry != "KfiKwinNativeBridge::install_frame_stub" ||
        frame_install.source_file != "src/compositor_wayland.cpp" ||
        frame_install.symbol != "WaylandCompositor::composite(RenderLoop *)" ||
        frame_install.checklist_hint != "confirm final composed frame-id provenance" ||
        frame_install.checklist_hint_secondary !=
            "confirm compositor timestamp maps to final output frame" ||
        frame_install.checklist_all.find(
            "confirm stable gpu handle ownership at present handoff"
        ) == std::string::npos ||
        present_install.result != fluxma::KwinNativeInstallResult::Deferred ||
        present_install.deferred_reason != fluxma::KwinNativeDeferredReason::PlaceholderOnly ||
        present_install.target != "output-frame-presented" ||
        present_install.installer_entry != "KfiKwinNativeBridge::install_present_stub" ||
        present_install.source_file != "src/core/renderbackend.cpp" ||
        present_install.symbol != "OutputFrame::presented(...)" ||
        present_install.checklist_hint !=
            "confirm presented frame-id still correlates with submitted frame" ||
        present_install.checklist_hint_secondary !=
            "confirm backend completion timestamp semantics across backends" ||
        present_install.checklist_all.find(
            "confirm refresh interval is available without backend-specific fallback"
        ) == std::string::npos) {
        std::cerr << "native bridge install stubs must surface deferred state\n";
        return EXIT_FAILURE;
    }
    const auto install = bridge.install_stub();
    const auto placeholder_gate = bridge.assess_install_gate(fluxma::KwinNativeInstallContext {});
    const auto combined_preflight = bridge.preflight_install(fluxma::KwinNativeInstallContext {});
    const auto frame_preflight = bridge.preflight_frame_install(fluxma::KwinNativeInstallContext {});
    const auto present_preflight = bridge.preflight_present_install(
        fluxma::KwinNativeInstallContext {}
    );
    const auto version_gate = bridge.assess_install_gate(
        fluxma::KwinNativeInstallContext {
            .kwin_version_supported = false,
            .backend_supported = true,
            .kwin_version = "6.3.80",
            .backend_name = "drm",
        }
    );
    const auto backend_gate = bridge.assess_install_gate(
        fluxma::KwinNativeInstallContext {
            .kwin_version_supported = true,
            .backend_supported = false,
            .kwin_version = "6.3.80",
            .backend_name = "wayland",
        }
    );
    const auto version_gate_install = bridge.install_stub(
        fluxma::KwinNativeInstallContext {
            .kwin_version_supported = false,
            .backend_supported = true,
            .kwin_version = "6.3.80",
            .backend_name = "drm",
        }
    );
    const auto backend_gate_install = bridge.install_stub(
        fluxma::KwinNativeInstallContext {
            .kwin_version_supported = true,
            .backend_supported = false,
            .kwin_version = "6.3.80",
            .backend_name = "wayland",
        }
    );
    const auto precedence_install = bridge.install_stub(
        fluxma::KwinNativeInstallContext {
            .kwin_version_supported = false,
            .backend_supported = false,
            .kwin_version = "6.3.81",
            .backend_name = "drm",
        }
    );
    const auto frame_install_summary = frame_install.summary();
    const auto present_install_summary = present_install.summary();
    const auto install_summary = install.summary();
    const auto frame_preflight_summary = frame_preflight.summary();
    const auto present_preflight_summary = present_preflight.summary();
    const auto combined_preflight_summary = combined_preflight.summary();
    const auto placeholder_gate_summary = placeholder_gate.summary();
    const auto version_gate_assessment_summary = version_gate.summary();
    const auto backend_gate_assessment_summary = backend_gate.summary();
    const auto version_gate_summary = version_gate_install.summary();
    const auto backend_gate_summary = backend_gate_install.summary();
    const auto precedence_summary = precedence_install.summary();
    if (frame_install_summary.find("target=compositor-output-frame-ready") == std::string::npos ||
        frame_install_summary.find("deferred_reason=placeholder-only") == std::string::npos ||
        frame_install_summary.find(
            "installer_entry=KfiKwinNativeBridge::install_frame_stub"
        ) == std::string::npos ||
        frame_install_summary.find("source=src/compositor_wayland.cpp") == std::string::npos ||
        frame_install_summary.find(
            "checklist_hint=confirm final composed frame-id provenance"
        ) == std::string::npos ||
        frame_install_summary.find(
            "checklist_hint_secondary=confirm compositor timestamp maps to final output frame"
        ) == std::string::npos ||
        frame_install_summary.find(
            "checklist_all=confirm final composed frame-id provenance"
        ) == std::string::npos ||
        present_install_summary.find("target=output-frame-presented") == std::string::npos ||
        present_install_summary.find("deferred_reason=placeholder-only") == std::string::npos ||
        present_install_summary.find(
            "installer_entry=KfiKwinNativeBridge::install_present_stub"
        ) == std::string::npos ||
        present_install_summary.find("source=src/core/renderbackend.cpp") == std::string::npos ||
        present_install_summary.find(
            "checklist_hint=confirm presented frame-id still correlates with submitted frame"
        ) == std::string::npos ||
        present_install_summary.find(
            "checklist_hint_secondary=confirm backend completion timestamp semantics across backends"
        ) == std::string::npos ||
        present_install_summary.find(
            "checklist_all=confirm presented frame-id still correlates with submitted frame"
        ) == std::string::npos ||
        install_summary.find("frame{result=deferred") == std::string::npos ||
        install_summary.find("present{result=deferred") == std::string::npos ||
        frame_preflight.gate.deferred_reason != fluxma::KwinNativeDeferredReason::PlaceholderOnly ||
        frame_preflight.target != "compositor-output-frame-ready" ||
        frame_preflight.installer_entry != "KfiKwinNativeBridge::install_frame_stub" ||
        frame_preflight.source_file != "src/compositor_wayland.cpp" ||
        frame_preflight.symbol != "WaylandCompositor::composite(RenderLoop *)" ||
        frame_preflight.checklist_hint != "confirm final composed frame-id provenance" ||
        frame_preflight.checklist_hint_secondary !=
            "confirm compositor timestamp maps to final output frame" ||
        frame_preflight_summary.find("deferred_reason=placeholder-only") == std::string::npos ||
        frame_preflight_summary.find("target=compositor-output-frame-ready") ==
            std::string::npos ||
        combined_preflight.frame.target != "compositor-output-frame-ready" ||
        combined_preflight.present.target != "output-frame-presented" ||
        combined_preflight_summary.find("frame{deferred_reason=placeholder-only") ==
            std::string::npos ||
        combined_preflight_summary.find("present{deferred_reason=placeholder-only") ==
            std::string::npos ||
        present_preflight.gate.deferred_reason !=
            fluxma::KwinNativeDeferredReason::PlaceholderOnly ||
        present_preflight.target != "output-frame-presented" ||
        present_preflight.installer_entry != "KfiKwinNativeBridge::install_present_stub" ||
        present_preflight.source_file != "src/core/renderbackend.cpp" ||
        present_preflight.symbol != "OutputFrame::presented(...)" ||
        present_preflight.checklist_hint !=
            "confirm presented frame-id still correlates with submitted frame" ||
        present_preflight.checklist_hint_secondary !=
            "confirm backend completion timestamp semantics across backends" ||
        present_preflight_summary.find("target=output-frame-presented") ==
            std::string::npos ||
        placeholder_gate.deferred_reason != fluxma::KwinNativeDeferredReason::PlaceholderOnly ||
        placeholder_gate.version_blocked || placeholder_gate.backend_blocked ||
        placeholder_gate_summary.find("deferred_reason=placeholder-only") == std::string::npos ||
        version_gate.deferred_reason != fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        !version_gate.version_blocked || version_gate.backend_blocked ||
        version_gate_assessment_summary.find("version_blocked=true") == std::string::npos ||
        version_gate_assessment_summary.find("backend_blocked=false") == std::string::npos ||
        backend_gate.deferred_reason != fluxma::KwinNativeDeferredReason::BackendGate ||
        backend_gate.version_blocked || !backend_gate.backend_blocked ||
        backend_gate_assessment_summary.find("version_blocked=false") == std::string::npos ||
        backend_gate_assessment_summary.find("backend_blocked=true") == std::string::npos ||
        version_gate_install.frame.deferred_reason != fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        version_gate_install.present.deferred_reason != fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        version_gate_install.frame.reason != "kwin version gate blocked install for 6.3.80" ||
        version_gate_install.frame.context_summary !=
            "kwin=6.3.80 backend=drm version_supported=false backend_supported=true" ||
        version_gate_summary.find("deferred_reason=kwin-version-gate") == std::string::npos ||
        version_gate_summary.find("reason=kwin version gate blocked install for 6.3.80") ==
            std::string::npos ||
        version_gate_summary.find(
            "context=kwin=6.3.80 backend=drm version_supported=false backend_supported=true"
        ) == std::string::npos ||
        backend_gate_install.frame.deferred_reason != fluxma::KwinNativeDeferredReason::BackendGate ||
        backend_gate_install.present.deferred_reason != fluxma::KwinNativeDeferredReason::BackendGate ||
        backend_gate_install.present.reason != "backend gate blocked install for wayland" ||
        backend_gate_install.present.context_summary !=
            "kwin=6.3.80 backend=wayland version_supported=true backend_supported=false" ||
        backend_gate_summary.find("deferred_reason=backend-gate") == std::string::npos ||
        backend_gate_summary.find("reason=backend gate blocked install for wayland") ==
            std::string::npos ||
        backend_gate_summary.find(
            "context=kwin=6.3.80 backend=wayland version_supported=true backend_supported=false"
        ) == std::string::npos ||
        precedence_install.frame.deferred_reason != fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        precedence_install.present.deferred_reason != fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        precedence_summary.find("deferred_reason=kwin-version-gate") == std::string::npos ||
        precedence_summary.find("reason=kwin version gate blocked install for 6.3.81") ==
            std::string::npos ||
        precedence_summary.find(
            "context=kwin=6.3.81 backend=drm version_supported=false backend_supported=false"
        ) ==
            std::string::npos) {
        std::cerr << "native bridge install summaries must stay stable\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
