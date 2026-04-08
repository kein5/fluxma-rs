#include <cstdlib>
#include <iostream>

#include "fluxma_kcm_bridge.h"

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
    const fluxma::ModuleConfig config {
        .enabled = true,
        .show_hud = true,
        .subtitle_protection = true,
        .cursor_protection = true,
        .log_interval_frames = 7,
        .max_log_messages = 9,
    };
    fluxma::KfiPluginRoot plugin_root(config);
    const fluxma::KfiKcmBridge kcm_bridge(plugin_root);

    const auto settings = kcm_bridge.settings();
    if (!settings.enabled || !settings.show_hud || !settings.subtitle_protection ||
        !settings.cursor_protection || settings.log_interval_frames != 7 ||
        settings.max_log_messages != 9) {
        std::cerr << "kcm settings snapshot mismatch\n";
        return EXIT_FAILURE;
    }

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

    const auto runtime = kcm_bridge.runtime(47'999'999);
    if (runtime.state != fluxma::OutputState::Active2x ||
        runtime.bypass_reason != fluxma::BypassReason::None ||
        runtime.protected_content || !runtime.passthrough_only ||
        !runtime.synthetic_armed || !runtime.synthetic_queued ||
        runtime.synthetic_suppressed_by_protection ||
        runtime.frame_tap_count != 5 || runtime.present_feedback_count != 1 ||
        runtime.deadline_miss_count != 0 || runtime.dropped_synthetic_count != 0 ||
        runtime.cadence_hz_millihz != 30000 ||
        runtime.hud_text.find("scheduler=synthetic-2x") == std::string::npos ||
        runtime.summary().find("synthetic-armed=yes") == std::string::npos ||
        runtime.summary().find("frame-taps=5") == std::string::npos ||
        runtime.summary().find("present-feedback=1") == std::string::npos ||
        runtime.summary().find("synthetic-suppressed-by-protection=no") ==
            std::string::npos) {
        std::cerr << "kcm runtime snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    fluxma::KfiPluginRoot protected_root(config);
    const fluxma::KfiKcmBridge protected_bridge(protected_root);
    for (std::uint64_t frame_id = 1; frame_id <= 5; ++frame_id) {
        const auto _ =
            protected_root.primary_output().on_frame_tapped(frame(frame_id, frame_id == 5));
    }
    const auto protected_runtime = protected_bridge.runtime(200'000'000);
    if (protected_runtime.state != fluxma::OutputState::ProtectedBypass ||
        protected_runtime.bypass_reason != fluxma::BypassReason::ProtectedContent ||
        !protected_runtime.protected_content || !protected_runtime.passthrough_only ||
        protected_runtime.synthetic_armed || protected_runtime.synthetic_queued ||
        !protected_runtime.synthetic_suppressed_by_protection ||
        protected_runtime.frame_tap_count != 5 ||
        protected_runtime.present_feedback_count != 0 ||
        protected_runtime.deadline_miss_count != 0 ||
        protected_runtime.dropped_synthetic_count != 0 ||
        protected_runtime.summary().find("synthetic-suppressed-by-protection=yes") ==
            std::string::npos ||
        protected_runtime.summary().find("frame-taps=5") == std::string::npos ||
        protected_runtime.summary().find("present-feedback=0") == std::string::npos ||
        protected_runtime.hud_text.find("protected=yes") == std::string::npos) {
        std::cerr << "kcm protected runtime snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    const fluxma::ModuleConfig disabled_config {
        .enabled = false,
        .show_hud = false,
        .subtitle_protection = false,
        .cursor_protection = false,
        .log_interval_frames = 3,
        .max_log_messages = 5,
    };
    fluxma::KfiPluginRoot disabled_root(disabled_config);
    const fluxma::KfiKcmBridge disabled_bridge(disabled_root);
    const auto disabled_settings = disabled_bridge.settings();
    if (disabled_settings.enabled || disabled_settings.show_hud ||
        disabled_settings.subtitle_protection || disabled_settings.cursor_protection ||
        disabled_settings.log_interval_frames != 3 ||
        disabled_settings.max_log_messages != 5) {
        std::cerr << "kcm disabled settings snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    fluxma::KfiPluginRoot degraded_root(config);
    const fluxma::KfiKcmBridge degraded_bridge(degraded_root);
    for (std::uint64_t frame_id = 1; frame_id <= 5; ++frame_id) {
        const auto _ = degraded_root.primary_output().on_frame_tapped(frame(frame_id));
    }
    degraded_root.primary_output().on_present_feedback(
        fluxma::PresentFeedback {
            .frame_id = 5,
            .presented_timestamp_ns = 166'666'665,
            .refresh_interval_ns = 16'666'667,
            .presentation_mode = fluxma::PresentationMode::VSync,
            .present_success = false,
            .dropped_synthetic = true,
        }
    );
    const auto degraded_runtime = degraded_bridge.runtime(200'000'000);
    if (degraded_runtime.frame_tap_count != 5 ||
        degraded_runtime.present_feedback_count != 1 ||
        degraded_runtime.deadline_miss_count != 1 ||
        degraded_runtime.dropped_synthetic_count != 1 ||
        degraded_runtime.summary().find("deadline-miss=1") == std::string::npos ||
        degraded_runtime.summary().find("synthetic-dropped=1") == std::string::npos) {
        std::cerr << "kcm degraded runtime counters mismatch\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
