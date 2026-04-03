#include <cstdlib>
#include <iostream>

#include "fluxma_kwin_hook_builders.h"

namespace {

fluxma::KwinCompositorFrameInputs make_frame_inputs() {
    return fluxma::KwinCompositorFrameInputs {
        .output_id = 3,
        .frame_id = 42,
        .timestamp_ns = 9'000'000,
        .target_presentation_timestamp_ns = 16'666'667,
        .predicted_render_time_ns = 8'000'000,
        .content_type = fluxma::ContentType::Video,
        .width = 2560,
        .height = 1440,
        .pixel_format = 7,
        .color_space = 9,
        .protected_content = false,
        .damage_ratio = 0.6,
        .cursor_visible = true,
        .cursor_x = 17.5,
        .cursor_y = 33.25,
        .cursor_velocity_x = 4.5,
        .cursor_velocity_y = -2.0,
        .gpu_handle = fluxma::GpuFrameHandle {.backend_kind = 2, .handle_id = 555},
        .cursor_composited_in_frame = true,
        .overlay_promoted = false,
    };
}

fluxma::KwinPresentFeedbackInputs make_present_inputs() {
    return fluxma::KwinPresentFeedbackInputs {
        .output_id = 3,
        .frame_id = 42,
        .presented_timestamp_ns = 18'000'000,
        .refresh_interval_ns = 16'666'667,
        .presentation_mode = fluxma::PresentationMode::AdaptiveSync,
        .present_success = true,
        .dropped_synthetic = false,
    };
}

}  // namespace

int main() {
    const auto frame_inputs = make_frame_inputs();
    if (!fluxma::KfiKwinFrameBuilder::is_complete(frame_inputs)) {
        std::cerr << "complete frame inputs must be accepted\n";
        return EXIT_FAILURE;
    }

    const auto frame_hook = fluxma::KfiKwinFrameBuilder::compositor_bundle(frame_inputs);
    if (frame_hook.context.hook_point != fluxma::KwinFrameHookPoint::CompositorOutputFrameReady ||
        frame_hook.event.output_id != frame_inputs.output_id ||
        frame_hook.event.metadata.frame_id != frame_inputs.frame_id ||
        frame_hook.event.metadata.timestamp_ns != frame_inputs.timestamp_ns ||
        frame_hook.event.metadata.target_presentation_timestamp_ns !=
            frame_inputs.target_presentation_timestamp_ns ||
        frame_hook.event.metadata.predicted_render_time_ns !=
            frame_inputs.predicted_render_time_ns ||
        frame_hook.event.metadata.content_type != frame_inputs.content_type ||
        frame_hook.event.payload.width != frame_inputs.width ||
        frame_hook.event.payload.height != frame_inputs.height ||
        frame_hook.event.payload.pixel_format != frame_inputs.pixel_format ||
        frame_hook.event.payload.color_space != frame_inputs.color_space ||
        frame_hook.event.payload.damage_ratio != frame_inputs.damage_ratio ||
        frame_hook.event.payload.cursor_visible != frame_inputs.cursor_visible ||
        frame_hook.event.payload.cursor_x != frame_inputs.cursor_x ||
        frame_hook.event.payload.cursor_y != frame_inputs.cursor_y ||
        frame_hook.event.payload.cursor_velocity_x != frame_inputs.cursor_velocity_x ||
        frame_hook.event.payload.cursor_velocity_y != frame_inputs.cursor_velocity_y ||
        frame_hook.event.payload.gpu_handle.handle_id != frame_inputs.gpu_handle.handle_id) {
        std::cerr << "frame bundle must preserve KWin hook input fields\n";
        return EXIT_FAILURE;
    }

    auto incomplete_frame_inputs = frame_inputs;
    incomplete_frame_inputs.gpu_handle = {};
    if (fluxma::KfiKwinFrameBuilder::is_complete(incomplete_frame_inputs)) {
        std::cerr << "incomplete frame inputs must be rejected\n";
        return EXIT_FAILURE;
    }
    const auto incomplete_frame_hook =
        fluxma::KfiKwinFrameBuilder::compositor_bundle(incomplete_frame_inputs);
    if (incomplete_frame_hook.event.output_id != 0 || incomplete_frame_hook.event.metadata.frame_id != 0 ||
        incomplete_frame_hook.event.payload.gpu_handle.handle_id != 0) {
        std::cerr << "incomplete frame inputs must collapse to sentinel event\n";
        return EXIT_FAILURE;
    }

    const auto present_inputs = make_present_inputs();
    if (!fluxma::KfiKwinPresentBuilder::is_complete(present_inputs)) {
        std::cerr << "complete present inputs must be accepted\n";
        return EXIT_FAILURE;
    }

    const auto present_hook =
        fluxma::KfiKwinPresentBuilder::render_loop_presented_bundle(present_inputs);
    if (present_hook.context.hook_point != fluxma::KwinPresentHookPoint::RenderLoopFramePresented ||
        present_hook.event.output_id != present_inputs.output_id ||
        present_hook.event.frame_id != present_inputs.frame_id ||
        present_hook.event.presented_timestamp_ns != present_inputs.presented_timestamp_ns ||
        present_hook.event.refresh_interval_ns != present_inputs.refresh_interval_ns ||
        present_hook.event.presentation_mode != present_inputs.presentation_mode ||
        present_hook.event.present_success != present_inputs.present_success ||
        present_hook.event.dropped_synthetic != present_inputs.dropped_synthetic) {
        std::cerr << "present bundle must preserve KWin feedback input fields\n";
        return EXIT_FAILURE;
    }

    auto incomplete_present_inputs = present_inputs;
    incomplete_present_inputs.refresh_interval_ns = 0;
    if (fluxma::KfiKwinPresentBuilder::is_complete(incomplete_present_inputs)) {
        std::cerr << "incomplete present inputs must be rejected\n";
        return EXIT_FAILURE;
    }
    const auto incomplete_present_hook =
        fluxma::KfiKwinPresentBuilder::output_frame_presented_bundle(incomplete_present_inputs);
    if (incomplete_present_hook.event.output_id != 0 || incomplete_present_hook.event.frame_id != 0 ||
        incomplete_present_hook.event.refresh_interval_ns != 0) {
        std::cerr << "incomplete present inputs must collapse to sentinel event\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
