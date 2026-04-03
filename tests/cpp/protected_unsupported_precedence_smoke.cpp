#include <cstdlib>
#include <iostream>

#include "fluxma_plugin_root.h"

int main() {
    fluxma::KfiKwinHookAdapter* hook_adapter = nullptr;
    fluxma::KfiOutputController* output = nullptr;
    fluxma::KfiPluginRoot plugin_root(
        {.enabled = true, .show_hud = true, .log_interval_frames = 1, .max_log_messages = 4}
    );
    hook_adapter = &plugin_root.primary_output_hook_adapter();
    output = &plugin_root.primary_output();

    const auto decision = hook_adapter->on_final_composed_frame(
        fluxma::FinalComposedFrameEvent {
            .output_id = 0,
            .metadata =
                fluxma::FinalComposedFrameMetadata {
                    .frame_id = 71,
                    .timestamp_ns = 23'000'000,
                },
            .payload =
                fluxma::FinalComposedFramePayload {
                    .width = 0,
                    .height = 0,
                    .pixel_format = 0,
                    .color_space = 0,
                    .protected_content = true,
                    .damage_ratio = 0.0,
                    .cursor_visible = false,
                    .cursor_x = 0.0,
                    .cursor_y = 0.0,
                    .cursor_velocity_x = 0.0,
                    .cursor_velocity_y = 0.0,
                    .gpu_handle = fluxma::GpuFrameHandle {.backend_kind = 0, .handle_id = 0},
                },
        }
    );

    if (decision.bypass_reason != fluxma::BypassReason::ProtectedContent ||
        decision.state != fluxma::OutputState::ProtectedBypass ||
        !decision.passthrough_only) {
        std::cerr << "protected content must win over unsupported output\n";
        return EXIT_FAILURE;
    }

    const auto submission = output->last_submission();
    if (!submission.protected_content ||
        submission.bypass_reason != fluxma::BypassReason::ProtectedContent) {
        std::cerr << "protected precedence submission mismatch\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
