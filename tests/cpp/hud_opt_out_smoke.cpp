#include <cstdlib>
#include <iostream>

#include "fluxma_plugin_root.h"

int main() {
    fluxma::KfiPluginRoot plugin_root(
        {.enabled = true, .show_hud = false, .log_interval_frames = 1, .max_log_messages = 4}
    );

    auto& hook_adapter = plugin_root.primary_output_hook_adapter();
    const auto decision = hook_adapter.on_final_composed_frame(
        fluxma::FinalComposedFrameEvent {
            .output_id = 0,
            .metadata =
                fluxma::FinalComposedFrameMetadata {
                    .frame_id = 61,
                    .timestamp_ns = 22'000'000,
                },
            .payload =
                fluxma::FinalComposedFramePayload {
                    .width = 1920,
                    .height = 1080,
                    .pixel_format = 0,
                    .color_space = 0,
                    .protected_content = false,
                    .damage_ratio = 0.4,
                    .cursor_visible = false,
                    .cursor_x = 0.0,
                    .cursor_y = 0.0,
                    .cursor_velocity_x = 0.0,
                    .cursor_velocity_y = 0.0,
                    .gpu_handle = fluxma::GpuFrameHandle {.backend_kind = 0, .handle_id = 6060},
                },
        }
    );

    if (decision.bypass_reason != fluxma::BypassReason::GpuPathNotReady) {
        std::cerr << "unexpected enabled opt-out decision\n";
        return EXIT_FAILURE;
    }

    if (!plugin_root.primary_output().render_hud_text().empty()) {
        std::cerr << "hud opt-out should suppress hud text while enabled\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
