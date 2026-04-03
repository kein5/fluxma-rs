#include <algorithm>
#include <cstdlib>
#include <iostream>

#include "fluxma_plugin_root.h"

namespace {

fluxma::FinalComposedFrameEvent make_frame() {
    return fluxma::FinalComposedFrameEvent {
        .output_id = 0,
        .metadata = fluxma::FinalComposedFrameMetadata {
            .frame_id = 21,
            .timestamp_ns = 9'000'000,
        },
        .payload = fluxma::FinalComposedFramePayload {
            .width = 1920,
            .height = 1080,
            .pixel_format = 0,
            .color_space = 0,
            .protected_content = false,
            .damage_ratio = 0.2,
            .cursor_visible = false,
            .cursor_x = 0.0,
            .cursor_y = 0.0,
            .cursor_velocity_x = 0.0,
            .cursor_velocity_y = 0.0,
            .gpu_handle = fluxma::GpuFrameHandle {.backend_kind = 0, .handle_id = 777},
        },
    };
}

}  // namespace

int main() {
    fluxma::KfiPluginRoot plugin_root(
        {
            .enabled = true,
            .show_hud = true,
            .log_interval_frames = 1,
            .max_log_messages = 4,
        },
        true
    );

    auto& output = plugin_root.primary_output();
    auto& hook_adapter = plugin_root.primary_output_hook_adapter();
    const auto decision = hook_adapter.on_final_composed_frame(make_frame());
    if (!decision.passthrough_only ||
        decision.state != fluxma::OutputState::Bypass ||
        decision.bypass_reason != fluxma::BypassReason::HookUnavailable) {
        std::cerr << "hook-unavailable output must stay passthrough-only\n";
        return EXIT_FAILURE;
    }

    const auto submission = output.last_submission();
    if (!submission.accepted ||
        submission.bypass_reason != fluxma::BypassReason::HookUnavailable ||
        submission.frame_id != 21) {
        std::cerr << "hook-unavailable submission mismatch\n";
        return EXIT_FAILURE;
    }

    const auto snapshot = output.snapshot_metrics();
    if (snapshot.state != fluxma::OutputState::Bypass ||
        snapshot.bypass_reason != fluxma::BypassReason::HookUnavailable ||
        snapshot.frame_tap_count != 0) {
        std::cerr << "hook-unavailable metrics mismatch\n";
        return EXIT_FAILURE;
    }

    const auto hud = output.render_hud_text();
    if (hud.find("bypass=hook-unavailable") == std::string::npos) {
        std::cerr << "hook-unavailable hud mismatch\n";
        return EXIT_FAILURE;
    }

    const auto logs = output.log_messages();
    const auto has_hook_unavailable_log =
        std::any_of(logs.begin(), logs.end(), [](const std::string& log) {
            return log.find("bypass=hook-unavailable") != std::string::npos;
        });
    if (!has_hook_unavailable_log) {
        std::cerr << "missing hook-unavailable log\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
