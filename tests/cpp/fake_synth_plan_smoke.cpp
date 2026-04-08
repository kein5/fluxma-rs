#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <string>

#include "fluxma_output_controller.h"

namespace {

fluxma::FrameDescriptor frame(std::uint64_t frame_id, bool protected_content = false) {
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
        .protected_content = protected_content,
        .damage_ratio = 0.5,
        .cursor_visible = false,
        .cursor_x = 0.0,
        .cursor_y = 0.0,
        .cursor_velocity_x = 0.0,
        .cursor_velocity_y = 0.0,
        .gpu_handle = fluxma::GpuFrameHandle {.backend_kind = 1, .handle_id = frame_id + 10},
    };
}

}  // namespace

int main() {
    fluxma::KfiOutputController output(9, fluxma::ModuleConfig {});

    for (std::uint64_t frame_id = 1; frame_id <= 5; ++frame_id) {
        const auto decision = output.on_frame_tapped(frame(frame_id));
        if (frame_id == 5 &&
            (decision.state != fluxma::OutputState::Active2x || !decision.interpolation_armed)) {
            std::cerr << "fake synth scheduler must arm after stable cadence\n";
            return EXIT_FAILURE;
        }
    }

    output.on_present_feedback(
        fluxma::PresentFeedback {
            .frame_id = 5,
            .presented_timestamp_ns = 166'666'665,
            .refresh_interval_ns = 16'666'667,
            .presentation_mode = fluxma::PresentationMode::VSync,
            .present_success = true,
            .dropped_synthetic = false,
        }
    );

    const auto ready_plan = output.plan_synthetic_frame(47'999'999);
    const auto ready_artifact = output.generate_fake_synthetic_frame(47'999'999);
    const auto ready_submission = output.submit_fake_synthetic_frame(47'999'999);
    const auto hud = output.render_hud_text();
    const auto logs = output.log_messages();
    if (!ready_plan.armed || ready_plan.should_drop ||
        ready_plan.source_frame_id != 5 || ready_plan.synthetic_frame_id != 11 ||
        ready_plan.target_present_timestamp_ns != 183'333'332 ||
        ready_plan.deadline_timestamp_ns != 181'333'332 ||
        !ready_artifact.generated || ready_artifact.dropped ||
        ready_artifact.synthetic_frame_id != 11 ||
        !ready_submission.queued || ready_submission.dropped ||
        !ready_submission.placeholder_only ||
        ready_submission.prefer_current_in_subtitle_band ||
        ready_submission.protection_plan.subtitle_band_active ||
        ready_submission.synthetic_frame_id != 11 ||
        hud.find("synthetic_armed=yes") == std::string::npos ||
        hud.find("synthetic_drop=no") == std::string::npos ||
        hud.find("synthetic_target_ns=183333332") == std::string::npos ||
        hud.find("synthetic_generated=yes") == std::string::npos ||
        hud.find("synthetic_queued=yes") == std::string::npos ||
        hud.find("synthetic_placeholder=yes") == std::string::npos ||
        hud.find("synthetic_subtitle_current_priority=no") == std::string::npos) {
        std::cerr << "fake synth plan mismatch\n";
        return EXIT_FAILURE;
    }
    const auto has_synthetic_plan_log =
        std::any_of(logs.begin(), logs.end(), [](const std::string& log) {
            return log.find("synthetic-armed=yes") != std::string::npos &&
                log.find("synthetic-frame-id=11") != std::string::npos;
        });
    if (!has_synthetic_plan_log) {
        std::cerr << "fake synth logger mismatch\n";
        return EXIT_FAILURE;
    }

    const auto dropped_plan = output.plan_synthetic_frame(181'333'332);
    const auto dropped_artifact = output.generate_fake_synthetic_frame(181'333'332);
    const auto dropped_submission = output.submit_fake_synthetic_frame(181'333'332);
    if (!dropped_plan.armed || !dropped_plan.should_drop ||
        dropped_artifact.generated || !dropped_artifact.dropped ||
        dropped_submission.queued || !dropped_submission.dropped ||
        !dropped_submission.placeholder_only ||
        dropped_submission.prefer_current_in_subtitle_band ||
        dropped_submission.protection_plan.subtitle_band_active) {
        std::cerr << "fake synth plan must drop once deadline is missed\n";
        return EXIT_FAILURE;
    }

    const auto protected_decision = output.on_frame_tapped(frame(6, true));
    const auto protected_plan = output.plan_synthetic_frame(200'000'000);
    const auto protected_artifact = output.generate_fake_synthetic_frame(200'000'000);
    const auto protected_submission = output.submit_fake_synthetic_frame(200'000'000);
    if (protected_decision.state != fluxma::OutputState::ProtectedBypass ||
        protected_plan.armed || protected_plan.should_drop ||
        protected_artifact.generated || protected_artifact.dropped ||
        protected_submission.queued || protected_submission.dropped ||
        !protected_submission.placeholder_only ||
        protected_submission.prefer_current_in_subtitle_band ||
        protected_submission.protection_plan.subtitle_band_active) {
        std::cerr << "protected bypass must not arm fake synth plan\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
