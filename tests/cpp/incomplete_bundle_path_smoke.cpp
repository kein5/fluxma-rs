#include <cstdlib>
#include <iostream>

#include "fluxma_kwin_hook_builders.h"
#include "fluxma_kwin_hook_candidates.h"
#include "fluxma_plugin_root.h"

int main() {
    fluxma::KfiPluginRoot plugin_root(
        {.enabled = true, .show_hud = true, .log_interval_frames = 1, .max_log_messages = 4}
    );

    auto& output = plugin_root.primary_output();
    auto& hook_adapter = plugin_root.primary_output_hook_adapter();

    const auto frame_plan = fluxma::KfiKwinHookCandidates::compositor_output_frame_ready();
    const auto incomplete_frame_inputs = fluxma::KwinCompositorFrameInputs {
        .output_id = 0,
        .frame_id = 31,
        .timestamp_ns = 4'000'000,
        .width = 1920,
        .height = 1080,
        .gpu_handle = {},
    };
    const auto frame_readiness =
        fluxma::KfiKwinHookCandidates::assess(frame_plan, incomplete_frame_inputs);
    if (frame_readiness.ready ||
        !fluxma::has_flag(frame_readiness.missing_fields, fluxma::KwinFrameInputField::GpuHandle)) {
        std::cerr << "incomplete frame plan must surface missing gpu handle\n";
        return EXIT_FAILURE;
    }

    const auto frame_decision = hook_adapter.on_compositor_output_frame_ready(
        fluxma::KfiKwinFrameBuilder::compositor_bundle(incomplete_frame_inputs)
    );
    if (!frame_decision.passthrough_only ||
        frame_decision.bypass_reason != fluxma::BypassReason::UnsupportedOutput) {
        std::cerr << "incomplete frame bundle must bypass as unsupported output\n";
        return EXIT_FAILURE;
    }
    const auto submission = output.last_submission();
    if (submission.accepted || submission.frame_id != 0) {
        std::cerr << "incomplete frame bundle must not submit\n";
        return EXIT_FAILURE;
    }

    const auto present_plan = fluxma::KfiKwinHookCandidates::output_frame_presented();
    const auto incomplete_present_inputs = fluxma::KwinPresentFeedbackInputs {
        .output_id = 0,
        .frame_id = 31,
        .presented_timestamp_ns = 5'000'000,
        .refresh_interval_ns = 0,
        .presentation_mode = fluxma::PresentationMode::VSync,
        .present_success = true,
        .dropped_synthetic = false,
    };
    const auto present_readiness =
        fluxma::KfiKwinHookCandidates::assess(present_plan, incomplete_present_inputs);
    if (present_readiness.ready ||
        !fluxma::has_flag(
            present_readiness.missing_fields,
            fluxma::KwinPresentInputField::RefreshInterval
        )) {
        std::cerr << "incomplete present plan must surface missing refresh interval\n";
        return EXIT_FAILURE;
    }

    hook_adapter.on_output_frame_presented(
        fluxma::KfiKwinPresentBuilder::output_frame_presented_bundle(incomplete_present_inputs)
    );
    const auto snapshot = output.snapshot_metrics();
    if (snapshot.frame_tap_count != 0 || snapshot.present_feedback_count != 0 ||
        snapshot.last_presented_frame_id != 0) {
        std::cerr << "incomplete bundle path must not mutate metrics\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
