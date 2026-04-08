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
        .cursor_visible = frame_id == 5,
        .cursor_x = 960.0,
        .cursor_y = 980.0,
        .cursor_velocity_x = 20.0,
        .cursor_velocity_y = 16.0,
        .overlay_promoted = frame_id == 5,
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
    const auto direct_sample = plugin_root.primary_output().sample_runtime(47'999'999);
    const auto direct_hud = plugin_root.primary_output().render_hud_text(direct_sample);
    if (report.snapshot.state != fluxma::OutputState::Active2x ||
        !report.synthetic_armed() || report.synthetic_should_drop() ||
        !report.synthetic_generated() || !report.synthetic_queued() ||
        report.synthetic_submission_dropped() || !report.synthetic_placeholder_only() ||
        !report.cursor_passthrough() || !report.subtitle_band_active() ||
        !report.protection_plan.transient_overlay_passthrough ||
        report.protection_plan.subtitle_band_top != 886 ||
        report.protection_plan.subtitle_band_bottom != 1080 ||
        report.synthetic_plan.synthetic_frame_id != 11 ||
        report.synthetic_artifact.synthetic_frame_id != 11 ||
        report.synthetic_submission.synthetic_frame_id != 11 ||
        direct_hud != report.hud_text ||
        report.hud_text.find("scheduler=synthetic-2x") == std::string::npos ||
        report.hud_text.find("synthetic_generated=yes") == std::string::npos ||
        report.hud_text.find("synthetic_queued=yes") == std::string::npos ||
        report.hud_text.find("cursor_passthrough=yes") == std::string::npos ||
        report.hud_text.find("subtitle_band=yes") == std::string::npos ||
        report.hud_text.find("overlay_passthrough=yes") == std::string::npos ||
        report.summary().find("synthetic-armed=yes") == std::string::npos ||
        report.summary().find("synthetic-generated=yes") == std::string::npos ||
        report.summary().find("synthetic-queued=yes") == std::string::npos ||
        report.summary().find("cursor-passthrough=yes") == std::string::npos ||
        report.summary().find("subtitle-band=yes") == std::string::npos ||
        report.summary().find("synthetic-submission-drop=no") == std::string::npos) {
        std::cerr << "output runtime observation mismatch\n";
        return EXIT_FAILURE;
    }

    const auto late_report = plugin_root.observe_output_runtime(181'333'332);
    if (!late_report.synthetic_should_drop() ||
        late_report.synthetic_generated() || late_report.synthetic_queued() ||
        !late_report.synthetic_submission_dropped() || !late_report.synthetic_placeholder_only() ||
        !late_report.cursor_passthrough() || !late_report.subtitle_band_active() ||
        !late_report.protection_plan.transient_overlay_passthrough ||
        late_report.synthetic_submission.synthetic_frame_id != 11 ||
        late_report.summary().find("synthetic-drop=yes") == std::string::npos ||
        late_report.summary().find("synthetic-placeholder=yes") == std::string::npos ||
        late_report.summary().find("synthetic-submission-drop=yes") == std::string::npos) {
        std::cerr << "output runtime observation must surface synthetic deadline miss\n";
        return EXIT_FAILURE;
    }

    fluxma::KfiPluginRoot hud_disabled_root(
        {.enabled = true, .show_hud = false, .log_interval_frames = 1, .max_log_messages = 4}
    );
    for (std::uint64_t frame_id = 1; frame_id <= 5; ++frame_id) {
        const auto _ = hud_disabled_root.primary_output().on_frame_tapped(frame(frame_id));
    }
    hud_disabled_root.primary_output().on_present_feedback(
        fluxma::PresentFeedback {
            .frame_id = 5,
            .presented_timestamp_ns = 166'666'665,
            .refresh_interval_ns = 16'666'667,
            .presentation_mode = fluxma::PresentationMode::VSync,
            .present_success = true,
            .dropped_synthetic = false,
        }
    );
    if (!hud_disabled_root.primary_output().render_hud_text().empty() ||
        !hud_disabled_root.primary_output().render_hud_text(
             hud_disabled_root.primary_output().sample_runtime(47'999'999)
         ).empty()) {
        std::cerr << "direct hud path must honor hud opt-out\n";
        return EXIT_FAILURE;
    }

    fluxma::KfiPluginRoot no_overlay_root;
    for (std::uint64_t frame_id = 1; frame_id <= 5; ++frame_id) {
        auto sample_frame = frame(frame_id);
        sample_frame.overlay_promoted = false;
        const auto _ = no_overlay_root.primary_output().on_frame_tapped(sample_frame);
    }
    no_overlay_root.primary_output().on_present_feedback(
        fluxma::PresentFeedback {
            .frame_id = 5,
            .presented_timestamp_ns = 166'666'665,
            .refresh_interval_ns = 16'666'667,
            .presentation_mode = fluxma::PresentationMode::VSync,
            .present_success = true,
            .dropped_synthetic = false,
        }
    );
    const auto no_overlay_report = no_overlay_root.observe_output_runtime(47'999'999);
    if (no_overlay_report.protection_plan.transient_overlay_passthrough ||
        no_overlay_report.hud_text.find("overlay_passthrough=no") == std::string::npos) {
        std::cerr << "runtime observation must preserve overlay false path\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
