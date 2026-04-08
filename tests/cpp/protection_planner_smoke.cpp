#include <cstdlib>
#include <iostream>

#include "fluxma_protection_planner.h"

namespace {

fluxma::FrameDescriptor video_frame() {
    return fluxma::FrameDescriptor {
        .frame_id = 9,
        .timestamp_ns = 300'000'000,
        .target_presentation_timestamp_ns = 316'666'667,
        .predicted_render_time_ns = 2'000'000,
        .width = 1920,
        .height = 1080,
        .pixel_format = 0,
        .color_space = 0,
        .content_type = fluxma::ContentType::Video,
        .protected_content = false,
        .damage_ratio = 0.4,
        .cursor_visible = true,
        .cursor_x = 960.0,
        .cursor_y = 540.0,
        .cursor_velocity_x = 10.0,
        .cursor_velocity_y = 12.0,
        .gpu_handle = fluxma::GpuFrameHandle {.backend_kind = 1, .handle_id = 109},
    };
}

}  // namespace

int main() {
    const fluxma::KfiProtectionPlanner planner;
    const fluxma::MetricsSnapshot snapshot {
        .state = fluxma::OutputState::Active2x,
        .bypass_reason = fluxma::BypassReason::None,
        .protected_content = false,
        .passthrough_only = false,
        .classifier_allows_interpolation = true,
    };

    const auto video_plan = planner.plan(video_frame(), snapshot, fluxma::ModuleConfig {});
    if (!video_plan.cursor_passthrough || !video_plan.cursor_recomposite ||
        !video_plan.subtitle_band_active || video_plan.subtitle_band_top != 886 ||
        video_plan.subtitle_band_bottom != 1080 || !video_plan.placeholder_only) {
        std::cerr << "video protection plan mismatch\n";
        return EXIT_FAILURE;
    }

    auto no_cursor_config = fluxma::ModuleConfig {};
    no_cursor_config.cursor_protection = false;
    const auto no_cursor_plan = planner.plan(video_frame(), snapshot, no_cursor_config);
    if (no_cursor_plan.cursor_passthrough || no_cursor_plan.cursor_recomposite ||
        !no_cursor_plan.subtitle_band_active) {
        std::cerr << "cursor protection opt-out mismatch\n";
        return EXIT_FAILURE;
    }

    auto no_subtitle_config = fluxma::ModuleConfig {};
    no_subtitle_config.subtitle_protection = false;
    const auto no_subtitle_plan = planner.plan(video_frame(), snapshot, no_subtitle_config);
    if (!no_subtitle_plan.cursor_passthrough || no_subtitle_plan.subtitle_band_active ||
        no_subtitle_plan.subtitle_band_top != 0 || no_subtitle_plan.subtitle_band_bottom != 0) {
        std::cerr << "subtitle protection opt-out mismatch\n";
        return EXIT_FAILURE;
    }

    auto protected_frame = video_frame();
    protected_frame.protected_content = true;
    const auto protected_plan = planner.plan(
        protected_frame,
        fluxma::MetricsSnapshot {.protected_content = true},
        fluxma::ModuleConfig {}
    );
    if (!protected_plan.cursor_passthrough || !protected_plan.cursor_recomposite ||
        protected_plan.subtitle_band_active || protected_plan.subtitle_band_top != 0 ||
        protected_plan.subtitle_band_bottom != 0) {
        std::cerr << "protected content must disable subtitle band only\n";
        return EXIT_FAILURE;
    }

    auto photo_frame = video_frame();
    photo_frame.content_type = fluxma::ContentType::Photo;
    const auto photo_plan = planner.plan(photo_frame, snapshot, fluxma::ModuleConfig {});
    if (!photo_plan.cursor_passthrough || photo_plan.subtitle_band_active) {
        std::cerr << "non-video content must skip subtitle band\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
