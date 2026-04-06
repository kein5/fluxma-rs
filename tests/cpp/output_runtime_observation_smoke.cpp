#include <cstdlib>
#include <iostream>
#include <string>

#include "fluxma_plugin_root.h"

namespace {

fluxma::FrameDescriptor frame(std::uint64_t frame_id) {
    return fluxma::FrameDescriptor {
        .frame_id = frame_id,
        .timestamp_ns = 33'333'333 * frame_id,
        .target_presentation_timestamp_ns = 33'333'333 * frame_id,
        .predicted_render_time_ns = 2'000'000,
        .width = 1920,
        .height = 1080,
        .pixel_format = 0,
        .color_space = 0,
        .content_type = fluxma::ContentType::Video,
        .protected_content = false,
        .damage_ratio = 0.5,
        .cursor_visible = false,
        .cursor_x = 0.0,
        .cursor_y = 0.0,
        .cursor_velocity_x = 0.0,
        .cursor_velocity_y = 0.0,
        .gpu_handle = fluxma::GpuFrameHandle {.backend_kind = 1, .handle_id = frame_id + 100},
    };
}

}  // namespace

int main() {
    fluxma::KfiPluginRoot plugin_root;

    for (std::uint64_t frame_id = 1; frame_id <= 5; ++frame_id) {
        const auto _ = plugin_root.primary_output().on_frame_tapped(frame(frame_id));
    }
    plugin_root.primary_output().on_present_feedback(
        fluxma::PresentFeedback {
            .frame_id = 5,
            .presented_timestamp_ns = 166'666'665,
            .refresh_interval_ns = 16'666'667,
            .presentation_mode = fluxma::PresentationMode::VSync,
            .present_success = true,
            .dropped_synthetic = false,
        }
    );

    const auto report = plugin_root.observe_output_runtime(47'999'999);
    if (report.snapshot.state != fluxma::OutputState::Active2x ||
        !report.synthetic_armed() || report.synthetic_should_drop() ||
        !report.synthetic_generated() || !report.synthetic_queued() ||
        report.synthetic_plan.synthetic_frame_id != 11 ||
        report.synthetic_artifact.synthetic_frame_id != 11 ||
        report.synthetic_submission.synthetic_frame_id != 11 ||
        report.hud_text.find("scheduler=synthetic-2x") == std::string::npos ||
        report.hud_text.find("synthetic_generated=yes") == std::string::npos ||
        report.summary().find("synthetic-armed=yes") == std::string::npos ||
        report.summary().find("synthetic-generated=yes") == std::string::npos ||
        report.summary().find("synthetic-queued=yes") == std::string::npos) {
        std::cerr << "output runtime observation mismatch\n";
        return EXIT_FAILURE;
    }

    const auto late_report = plugin_root.observe_output_runtime(181'333'332);
    if (!late_report.synthetic_should_drop() ||
        late_report.synthetic_generated() || late_report.synthetic_queued() ||
        late_report.summary().find("synthetic-drop=yes") == std::string::npos ||
        late_report.summary().find("synthetic-placeholder=yes") == std::string::npos) {
        std::cerr << "output runtime observation must surface synthetic deadline miss\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
