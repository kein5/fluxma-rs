#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>

#include "fluxma_output_controller.h"

namespace {

fluxma::FrameDescriptor frame(std::uint64_t frame_id, std::uint64_t timestamp_ns) {
    return fluxma::FrameDescriptor {
        .frame_id = frame_id,
        .timestamp_ns = timestamp_ns,
        .target_presentation_timestamp_ns = timestamp_ns + 16'666'667,
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
    fluxma::ModuleConfig config;
    config.log_interval_frames = 1;
    fluxma::KfiOutputController output(7, config);

    for (std::uint64_t index = 0; index < 5; ++index) {
        const auto decision = output.on_frame_tapped(
            frame(index + 1, 33'333'333 * (index + 1))
        );
        if (index < 2 && decision.state != fluxma::OutputState::Bypass) {
            std::cerr << "state core must stay in bypass before cadence stabilizes\n";
            return EXIT_FAILURE;
        }
    }

    const auto snapshot = output.snapshot_metrics();
    const auto submission = output.last_submission();
    if (snapshot.state != fluxma::OutputState::Active2x ||
        snapshot.bypass_reason != fluxma::BypassReason::None ||
        snapshot.cadence_status != fluxma::CadenceStatus::Stable ||
        snapshot.cadence_hz_millihz != 30'000 ||
        !snapshot.classifier_allows_interpolation ||
        snapshot.governor_mode != fluxma::GovernorMode::QualityHigh ||
        snapshot.scheduler_mode != fluxma::SchedulerMode::Synthetic2x ||
        snapshot.state_transition_count < 2 ||
        !submission.accepted || !submission.interpolation_armed) {
        std::cerr << "state core snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    const auto hud = output.render_hud_text();
    if (hud.find("state=active-2x") == std::string::npos ||
        hud.find("bypass=none") == std::string::npos ||
        hud.find("cadence_status=stable") == std::string::npos ||
        hud.find("cadence_millihz=30000") == std::string::npos ||
        hud.find("classifier=yes") == std::string::npos ||
        hud.find("governor=quality-high") == std::string::npos ||
        hud.find("scheduler=synthetic-2x") == std::string::npos) {
        std::cerr << "state core hud mismatch\n";
        return EXIT_FAILURE;
    }

    const auto logs = output.log_messages();
    const auto has_state_core_log =
        std::any_of(logs.begin(), logs.end(), [](const std::string& log) {
            return log.find("cadence=stable") != std::string::npos &&
                log.find("governor=quality-high") != std::string::npos &&
                log.find("scheduler=synthetic-2x") != std::string::npos &&
                log.find("classifier=yes") != std::string::npos;
        });
    if (!has_state_core_log) {
        std::cerr << "state core log mismatch\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
