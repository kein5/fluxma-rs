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
        frame_install.reason != "native bridge is still placeholder-only" ||
        frame_install.target != "compositor-output-frame-ready" ||
        frame_install.source_file != "src/compositor_wayland.cpp" ||
        frame_install.symbol != "WaylandCompositor::composite(RenderLoop *)" ||
        present_install.result != fluxma::KwinNativeInstallResult::Deferred ||
        present_install.target != "output-frame-presented" ||
        present_install.source_file != "src/core/renderbackend.cpp" ||
        present_install.symbol != "OutputFrame::presented(...)") {
        std::cerr << "native bridge install stubs must surface deferred state\n";
        return EXIT_FAILURE;
    }
    const auto install = bridge.install_stub();
    const auto frame_install_summary = frame_install.summary();
    const auto present_install_summary = present_install.summary();
    const auto install_summary = install.summary();
    if (frame_install_summary.find("target=compositor-output-frame-ready") == std::string::npos ||
        frame_install_summary.find("source=src/compositor_wayland.cpp") == std::string::npos ||
        present_install_summary.find("target=output-frame-presented") == std::string::npos ||
        present_install_summary.find("source=src/core/renderbackend.cpp") == std::string::npos ||
        install_summary.find("frame{result=deferred") == std::string::npos ||
        install_summary.find("present{result=deferred") == std::string::npos) {
        std::cerr << "native bridge install summaries must stay stable\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
