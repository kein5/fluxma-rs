#include <cstdlib>
#include <iostream>

#include "fluxma_plugin_root.h"

int main() {
    fluxma::KfiPluginRoot plugin_root(
        {.enabled = true, .show_hud = true, .log_interval_frames = 1, .max_log_messages = 4}
    );

    auto& output = plugin_root.primary_output();
    auto& hook_adapter = plugin_root.primary_output_hook_adapter();
    const auto decision = hook_adapter.on_final_composed_frame(
        fluxma::FinalComposedFrameEvent {
            .output_id = 0,
            .metadata =
                fluxma::FinalComposedFrameMetadata {
                    .frame_id = 31,
                    .timestamp_ns = 12'000'000,
                },
            .payload =
                fluxma::FinalComposedFramePayload {
                    .width = 0,
                    .height = 1080,
                    .pixel_format = 0,
                    .color_space = 0,
                    .protected_content = false,
                    .damage_ratio = 0.5,
                    .cursor_visible = false,
                    .cursor_x = 0.0,
                    .cursor_y = 0.0,
                    .cursor_velocity_x = 0.0,
                    .cursor_velocity_y = 0.0,
                    .gpu_handle = fluxma::GpuFrameHandle {.backend_kind = 0, .handle_id = 0},
                },
        }
    );

    if (!decision.passthrough_only ||
        decision.bypass_reason != fluxma::BypassReason::UnsupportedOutput) {
        std::cerr << "unsupported output must remain passthrough-only\n";
        return EXIT_FAILURE;
    }

    const auto submission = output.last_submission();
    if (submission.accepted || submission.frame_id != 0 || submission.source_frame.handle_id != 0) {
        std::cerr << "unsupported output must not mutate controller submission state\n";
        return EXIT_FAILURE;
    }

    const auto snapshot = output.snapshot_metrics();
    if (snapshot.frame_tap_count != 0 || snapshot.present_feedback_count != 0) {
        std::cerr << "unsupported output must not mutate controller metrics\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
