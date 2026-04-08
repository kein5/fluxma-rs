#include <cstdlib>
#include <iostream>

#include "fluxma_kwin_hook_adapter.h"
#include "fluxma_kwin_hook_builders.h"
#include "fluxma_plugin_root.h"

int main() {
    fluxma::KfiPluginRoot plugin_root(
        {.enabled = true, .show_hud = true, .log_interval_frames = 1, .max_log_messages = 4}
    );

    auto& output = plugin_root.primary_output();
    auto& hook_adapter = plugin_root.primary_output_hook_adapter();

    const fluxma::KwinCompositorFrameInputs inputs {
        .output_id = 0,
        .frame_id = 91,
        .timestamp_ns = 31'000'000,
        .target_presentation_timestamp_ns = 47'666'667,
        .predicted_render_time_ns = 1'000'000,
        .content_type = fluxma::ContentType::Video,
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
        .gpu_handle = fluxma::GpuFrameHandle {.backend_kind = 0, .handle_id = 9191},
        .cursor_composited_in_frame = false,
        .overlay_promoted = true,
    };

    const auto frame = fluxma::KfiKwinFrameBuilder::compositor_event(inputs);
    const auto decision = hook_adapter.on_compositor_output_frame_ready(
        fluxma::KfiKwinFrameBuilder::backend_present_handoff(
            inputs.cursor_composited_in_frame,
            inputs.overlay_promoted
        ),
        inputs.output_id,
        frame.metadata,
        frame.payload
    );

    if (!decision.passthrough_only ||
        decision.bypass_reason != fluxma::BypassReason::HookUnavailable) {
        std::cerr << "unsupported frame hook context must bypass\n";
        return EXIT_FAILURE;
    }

    const auto submission = output.last_submission();
    if (submission.accepted || submission.frame_id != 0) {
        std::cerr << "unsupported frame hook context must not submit\n";
        return EXIT_FAILURE;
    }

    hook_adapter.on_render_loop_frame_presented(
        fluxma::KwinPresentHookContext {.hook_point = fluxma::KwinPresentHookPoint::Unknown},
        0,
        fluxma::KfiKwinPresentBuilder::metadata(
            91,
            32'000'000,
            16'666'667,
            fluxma::PresentationMode::VSync
        ),
        fluxma::KfiKwinPresentBuilder::status(true, false)
    );

    const auto snapshot = output.snapshot_metrics();
    if (snapshot.frame_tap_count != 0 || snapshot.present_feedback_count != 0) {
        std::cerr << "unknown present hook context must be ignored\n";
        return EXIT_FAILURE;
    }

    const fluxma::KwinCompositorFrameInputs compositor_inputs {
        .output_id = 0,
        .frame_id = 92,
        .timestamp_ns = 48'000'000,
        .target_presentation_timestamp_ns = 64'666'667,
        .predicted_render_time_ns = 1'500'000,
        .content_type = fluxma::ContentType::Video,
        .width = 1920,
        .height = 1080,
        .pixel_format = 0,
        .color_space = 0,
        .protected_content = false,
        .damage_ratio = 0.2,
        .cursor_visible = true,
        .cursor_x = 960.0,
        .cursor_y = 970.0,
        .cursor_velocity_x = 8.0,
        .cursor_velocity_y = 7.0,
        .gpu_handle = fluxma::GpuFrameHandle {.backend_kind = 0, .handle_id = 9292},
        .cursor_composited_in_frame = true,
        .overlay_promoted = true,
    };
    const auto compositor_frame = fluxma::KfiKwinFrameBuilder::compositor_event(compositor_inputs);
    const auto compositor_decision = hook_adapter.on_compositor_output_frame_ready(
        fluxma::KfiKwinFrameBuilder::compositor_context(compositor_inputs),
        compositor_inputs.output_id,
        compositor_frame.metadata,
        compositor_frame.payload
    );
    const auto sample = output.sample_runtime(64'666'667);
    if (compositor_decision.bypass_reason == fluxma::BypassReason::HookUnavailable ||
        sample.snapshot.frame_tap_count == 0 ||
        !sample.protection_plan.transient_overlay_passthrough ||
        !sample.protection_plan.cursor_passthrough) {
        std::cerr << "compositor hook context must propagate overlay/cursor flags\n";
        return EXIT_FAILURE;
    }

    const fluxma::KwinCompositorFrameInputs no_overlay_inputs {
        .output_id = 0,
        .frame_id = 93,
        .timestamp_ns = 65'000'000,
        .target_presentation_timestamp_ns = 81'666'667,
        .predicted_render_time_ns = 1'500'000,
        .content_type = fluxma::ContentType::Video,
        .width = 1920,
        .height = 1080,
        .pixel_format = 0,
        .color_space = 0,
        .protected_content = false,
        .damage_ratio = 0.2,
        .cursor_visible = true,
        .cursor_x = 960.0,
        .cursor_y = 970.0,
        .cursor_velocity_x = 8.0,
        .cursor_velocity_y = 7.0,
        .gpu_handle = fluxma::GpuFrameHandle {.backend_kind = 0, .handle_id = 9393},
        .cursor_composited_in_frame = true,
        .overlay_promoted = false,
    };
    const auto no_overlay_frame = fluxma::KfiKwinFrameBuilder::compositor_event(no_overlay_inputs);
    const auto no_overlay_decision = hook_adapter.on_compositor_output_frame_ready(
        fluxma::KfiKwinFrameBuilder::compositor_context(no_overlay_inputs),
        no_overlay_inputs.output_id,
        no_overlay_frame.metadata,
        no_overlay_frame.payload
    );
    const auto no_overlay_sample = output.sample_runtime(81'666'667);
    if (no_overlay_decision.bypass_reason == fluxma::BypassReason::HookUnavailable ||
        no_overlay_sample.snapshot.frame_tap_count == 0 ||
        no_overlay_sample.protection_plan.transient_overlay_passthrough) {
        std::cerr << "compositor hook context must preserve overlay false path\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
