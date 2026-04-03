#include <cstdlib>
#include <iostream>

#include "fluxma_plugin_root.h"

namespace {

fluxma::FinalComposedFrameEvent make_frame() {
    return fluxma::FinalComposedFrameEvent {
        .output_id = 0,
        .metadata = fluxma::FinalComposedFrameMetadata {
            .frame_id = 11,
            .timestamp_ns = 5'000'000,
        },
        .payload = fluxma::FinalComposedFramePayload {
            .width = 1280,
            .height = 720,
            .pixel_format = 0,
            .color_space = 0,
            .protected_content = false,
            .damage_ratio = 0.1,
            .cursor_visible = false,
            .cursor_x = 0.0,
            .cursor_y = 0.0,
            .cursor_velocity_x = 0.0,
            .cursor_velocity_y = 0.0,
            .gpu_handle = fluxma::GpuFrameHandle {.backend_kind = 0, .handle_id = 123},
        },
    };
}

}  // namespace

int main() {
    fluxma::KfiPluginRoot plugin_root(
        {.enabled = false, .show_hud = false, .log_interval_frames = 1, .max_log_messages = 4}
    );

    auto& output = plugin_root.primary_output();
    auto& hook_adapter = plugin_root.primary_output_hook_adapter();
    const auto decision = hook_adapter.on_final_composed_frame(make_frame());
    if (!decision.passthrough_only ||
        decision.state != fluxma::OutputState::Disabled ||
        decision.bypass_reason != fluxma::BypassReason::Disabled) {
        std::cerr << "disabled output must stay passthrough-only\n";
        return EXIT_FAILURE;
    }

    const auto submission = output.last_submission();
    if (!submission.accepted ||
        submission.bypass_reason != fluxma::BypassReason::Disabled ||
        submission.protected_content) {
        std::cerr << "disabled submission must stay passthrough-only\n";
        return EXIT_FAILURE;
    }

    const auto snapshot = output.snapshot_metrics();
    if (snapshot.state != fluxma::OutputState::Disabled ||
        snapshot.bypass_reason != fluxma::BypassReason::Disabled ||
        snapshot.frame_tap_count != 1) {
        std::cerr << "disabled metrics mismatch\n";
        return EXIT_FAILURE;
    }

    if (!output.render_hud_text().empty()) {
        std::cerr << "hud should be hidden when disabled in config\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
