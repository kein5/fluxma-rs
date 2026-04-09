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
        .mode = fluxma::ModuleMode::Synthetic2x,
        .show_hud = true,
        .subtitle_protection = true,
        .cursor_protection = true,
        .log_interval_frames = 7,
        .max_log_messages = 9,
    };
    fluxma::KfiPluginRoot plugin_root(config);
    const fluxma::KfiKcmBridge kcm_bridge(plugin_root);

    const auto settings = kcm_bridge.settings();
    if (!settings.enabled || settings.mode != fluxma::ModuleMode::Synthetic2x ||
        !settings.show_hud || !settings.subtitle_protection ||
        !settings.cursor_protection || settings.log_interval_frames != 7 ||
        settings.summary().find("mode=synthetic-2x") == std::string::npos ||
        settings.summary().find("show-hud=yes") == std::string::npos ||
        settings.max_log_messages != 9) {
        std::cerr << "kcm settings snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    const auto complete_bringup = kcm_bridge.native_bridge_bringup(
        fluxma::KwinCompositorFrameInputs {
            .output_id = 0,
            .frame_id = 11,
            .timestamp_ns = 1'000'000,
            .target_presentation_timestamp_ns = 16'666'667,
            .predicted_render_time_ns = 2'000'000,
            .content_type = fluxma::ContentType::Video,
            .width = 1920,
            .height = 1080,
            .gpu_handle = fluxma::GpuFrameHandle {.backend_kind = 1, .handle_id = 211},
        },
        fluxma::KwinPresentFeedbackInputs {
            .output_id = 0,
            .frame_id = 11,
            .presented_timestamp_ns = 2'000'000,
            .refresh_interval_ns = 16'666'667,
            .presentation_mode = fluxma::PresentationMode::VSync,
            .present_success = true,
            .dropped_synthetic = false,
        }
    );
    if (complete_bringup.state != fluxma::KwinNativeBridgeState::PlaceholderOnly ||
        !complete_bringup.frame_complete || !complete_bringup.present_complete ||
        !complete_bringup.frame_has_unresolved || !complete_bringup.present_has_unresolved ||
        complete_bringup.summary().find("frame-complete=yes") == std::string::npos ||
        complete_bringup.bringup_summary.find("frame{hook=compositor-output-frame-ready") ==
            std::string::npos) {
        std::cerr << "kcm native bridge bringup snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    const auto incomplete_bringup = kcm_bridge.native_bridge_bringup(
        fluxma::KwinCompositorFrameInputs {
            .output_id = 0,
            .timestamp_ns = 1'000'000,
            .width = 1920,
            .height = 1080,
        },
        fluxma::KwinPresentFeedbackInputs {
            .output_id = 0,
            .presented_timestamp_ns = 2'000'000,
        }
    );
    if (incomplete_bringup.state != fluxma::KwinNativeBridgeState::PlaceholderOnly ||
        incomplete_bringup.frame_complete || incomplete_bringup.present_complete ||
        !incomplete_bringup.frame_has_unresolved || !incomplete_bringup.present_has_unresolved ||
        incomplete_bringup.summary().find("present-complete=no") == std::string::npos ||
        incomplete_bringup.bringup_summary.find("missing=frame-id,gpu-handle") ==
            std::string::npos ||
        incomplete_bringup.bringup_summary.find("missing=frame-id,refresh-interval") ==
            std::string::npos) {
        std::cerr << "kcm native bridge incomplete bringup snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    const auto placeholder_native_diagnostics = kcm_bridge.native_bridge_diagnostics(
        fluxma::KwinCompositorFrameInputs {
            .output_id = 0,
            .frame_id = 12,
            .timestamp_ns = 2'000'000,
            .target_presentation_timestamp_ns = 18'000'000,
            .predicted_render_time_ns = 2'000'000,
            .content_type = fluxma::ContentType::Video,
            .width = 1920,
            .height = 1080,
            .gpu_handle = fluxma::GpuFrameHandle {.backend_kind = 1, .handle_id = 212},
        },
        fluxma::KwinPresentFeedbackInputs {
            .output_id = 0,
            .frame_id = 12,
            .presented_timestamp_ns = 3'000'000,
            .refresh_interval_ns = 16'666'667,
            .presentation_mode = fluxma::PresentationMode::VSync,
            .present_success = true,
            .dropped_synthetic = false,
        },
        fluxma::KwinNativeInstallContext {}
    );
    if (placeholder_native_diagnostics.state != fluxma::KwinNativeBridgeState::PlaceholderOnly ||
        !placeholder_native_diagnostics.bringup_complete ||
        !placeholder_native_diagnostics.has_unresolved_candidates ||
        !placeholder_native_diagnostics.frame_gate_matches ||
        !placeholder_native_diagnostics.present_gate_matches ||
        placeholder_native_diagnostics.frame_has_any_blocker ||
        placeholder_native_diagnostics.present_has_any_blocker ||
        placeholder_native_diagnostics.frame_deferred_reason !=
            fluxma::KwinNativeDeferredReason::PlaceholderOnly ||
        placeholder_native_diagnostics.present_deferred_reason !=
            fluxma::KwinNativeDeferredReason::PlaceholderOnly ||
        placeholder_native_diagnostics.summary().find("frame-blocked=no") ==
            std::string::npos ||
        placeholder_native_diagnostics.diagnostics_summary.find(
            "deferred_reason=placeholder-only"
        ) == std::string::npos) {
        std::cerr << "kcm placeholder native diagnostics snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    const auto backend_native_diagnostics = kcm_bridge.native_bridge_diagnostics(
        fluxma::KwinCompositorFrameInputs {
            .output_id = 0,
            .frame_id = 13,
            .timestamp_ns = 2'500'000,
            .target_presentation_timestamp_ns = 18'500'000,
            .predicted_render_time_ns = 2'000'000,
            .content_type = fluxma::ContentType::Video,
            .width = 1920,
            .height = 1080,
            .gpu_handle = fluxma::GpuFrameHandle {.backend_kind = 1, .handle_id = 213},
        },
        fluxma::KwinPresentFeedbackInputs {
            .output_id = 0,
            .frame_id = 13,
            .presented_timestamp_ns = 3'500'000,
            .refresh_interval_ns = 16'666'667,
            .presentation_mode = fluxma::PresentationMode::VSync,
            .present_success = true,
            .dropped_synthetic = false,
        },
        fluxma::KwinNativeInstallContext {
            .kwin_version_supported = true,
            .backend_supported = false,
            .kwin_version = "6.3.93",
            .backend_name = "wayland",
        }
    );
    if (backend_native_diagnostics.state != fluxma::KwinNativeBridgeState::PlaceholderOnly ||
        !backend_native_diagnostics.bringup_complete ||
        !backend_native_diagnostics.has_unresolved_candidates ||
        !backend_native_diagnostics.frame_gate_matches ||
        !backend_native_diagnostics.present_gate_matches ||
        !backend_native_diagnostics.frame_has_any_blocker ||
        !backend_native_diagnostics.present_has_any_blocker ||
        backend_native_diagnostics.frame_deferred_reason !=
            fluxma::KwinNativeDeferredReason::BackendGate ||
        backend_native_diagnostics.present_deferred_reason !=
            fluxma::KwinNativeDeferredReason::BackendGate ||
        backend_native_diagnostics.summary().find("present-blocked=yes") ==
            std::string::npos ||
        backend_native_diagnostics.diagnostics_summary.find(
            "backend gate blocked install for wayland"
        ) == std::string::npos) {
        std::cerr << "kcm backend native diagnostics snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    const auto native_diagnostics = kcm_bridge.native_bridge_diagnostics(
        fluxma::KwinCompositorFrameInputs {
            .output_id = 0,
            .frame_id = 12,
            .timestamp_ns = 2'000'000,
            .target_presentation_timestamp_ns = 18'000'000,
            .predicted_render_time_ns = 2'000'000,
            .content_type = fluxma::ContentType::Video,
            .width = 1920,
            .height = 1080,
            .gpu_handle = fluxma::GpuFrameHandle {.backend_kind = 1, .handle_id = 212},
        },
        fluxma::KwinPresentFeedbackInputs {
            .output_id = 0,
            .frame_id = 12,
            .presented_timestamp_ns = 3'000'000,
            .refresh_interval_ns = 16'666'667,
            .presentation_mode = fluxma::PresentationMode::VSync,
            .present_success = true,
            .dropped_synthetic = false,
        },
        fluxma::KwinNativeInstallContext {
            .kwin_version_supported = false,
            .backend_supported = true,
            .kwin_version = "6.3.92",
            .backend_name = "drm",
        }
    );
    if (native_diagnostics.state != fluxma::KwinNativeBridgeState::PlaceholderOnly ||
        !native_diagnostics.bringup_complete ||
        !native_diagnostics.has_unresolved_candidates ||
        !native_diagnostics.frame_gate_matches ||
        !native_diagnostics.present_gate_matches ||
        !native_diagnostics.frame_has_any_blocker ||
        !native_diagnostics.present_has_any_blocker ||
        native_diagnostics.frame_deferred_reason !=
            fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        native_diagnostics.present_deferred_reason !=
            fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        native_diagnostics.summary().find("bringup-complete=yes") == std::string::npos ||
        native_diagnostics.diagnostics_summary.find("preflight{") == std::string::npos ||
        native_diagnostics.diagnostics_summary.find(
            "reason=kwin version gate blocked install for 6.3.92"
        ) == std::string::npos) {
        std::cerr << "kcm native diagnostics snapshot mismatch\n";
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
        runtime.governor_mode != fluxma::GovernorMode::QualityHigh ||
        runtime.scheduler_mode != fluxma::SchedulerMode::Synthetic2x ||
        !runtime.classifier_allows_interpolation ||
        runtime.protected_content || !runtime.passthrough_only ||
        !runtime.synthetic_armed || !runtime.synthetic_queued ||
        runtime.synthetic_suppressed_by_protection ||
        !runtime.cursor_passthrough || !runtime.cursor_recomposite ||
        !runtime.subtitle_band_active || !runtime.overlay_passthrough ||
        !runtime.protection_placeholder_only ||
        runtime.state_transition_count != 3 ||
        runtime.frame_tap_count != 5 || runtime.present_feedback_count != 1 ||
        runtime.deadline_miss_count != 0 || runtime.dropped_synthetic_count != 0 ||
        runtime.last_presented_frame_id != 5 ||
        runtime.last_presented_timestamp_ns != 166'666'665 ||
        runtime.refresh_interval_ns != 16'666'667 ||
        runtime.last_target_presentation_timestamp_ns != 166'666'665 ||
        runtime.last_predicted_render_time_ns != 2'000'000 ||
        runtime.last_presentation_mode != fluxma::PresentationMode::VSync ||
        runtime.last_content_type != fluxma::ContentType::Video ||
        runtime.cadence_hz_millihz != 30000 ||
        runtime.hud_text.find("scheduler=synthetic-2x") == std::string::npos ||
        runtime.summary().find("synthetic-armed=yes") == std::string::npos ||
        runtime.summary().find("classifier=yes") == std::string::npos ||
        runtime.summary().find("cursor-passthrough=yes") == std::string::npos ||
        runtime.summary().find("subtitle-band=yes") == std::string::npos ||
        runtime.summary().find("overlay-passthrough=yes") == std::string::npos ||
        runtime.summary().find("protection-placeholder=yes") == std::string::npos ||
        runtime.summary().find("state-transitions=") == std::string::npos ||
        runtime.summary().find("last-presented-frame=5") == std::string::npos ||
        runtime.summary().find("last-presented-ts=166666665") == std::string::npos ||
        runtime.summary().find("target-present-ns=166666665") == std::string::npos ||
        runtime.summary().find("predicted-render-ns=2000000") == std::string::npos ||
        runtime.summary().find("present-mode=vsync") == std::string::npos ||
        runtime.summary().find("content-type=video") == std::string::npos ||
        runtime.summary().find("governor=quality-high") == std::string::npos ||
        runtime.summary().find("scheduler=synthetic-2x") == std::string::npos ||
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
        protected_runtime.governor_mode != fluxma::GovernorMode::Bypass ||
        protected_runtime.scheduler_mode != fluxma::SchedulerMode::PassthroughOnly ||
        protected_runtime.classifier_allows_interpolation ||
        !protected_runtime.protected_content || !protected_runtime.passthrough_only ||
        protected_runtime.synthetic_armed || protected_runtime.synthetic_queued ||
        !protected_runtime.synthetic_suppressed_by_protection ||
        !protected_runtime.cursor_passthrough || !protected_runtime.cursor_recomposite ||
        protected_runtime.subtitle_band_active || !protected_runtime.overlay_passthrough ||
        !protected_runtime.protection_placeholder_only ||
        protected_runtime.state_transition_count != 3 ||
        protected_runtime.frame_tap_count != 5 ||
        protected_runtime.present_feedback_count != 0 ||
        protected_runtime.deadline_miss_count != 0 ||
        protected_runtime.dropped_synthetic_count != 0 ||
        protected_runtime.last_presented_frame_id != 0 ||
        protected_runtime.last_presented_timestamp_ns != 0 ||
        protected_runtime.refresh_interval_ns != 0 ||
        protected_runtime.last_target_presentation_timestamp_ns != 166'666'665 ||
        protected_runtime.last_predicted_render_time_ns != 2'000'000 ||
        protected_runtime.last_presentation_mode != fluxma::PresentationMode::VSync ||
        protected_runtime.last_content_type != fluxma::ContentType::Video ||
        protected_runtime.summary().find("synthetic-suppressed-by-protection=yes") ==
            std::string::npos ||
        protected_runtime.summary().find("classifier=no") == std::string::npos ||
        protected_runtime.summary().find("cursor-passthrough=yes") ==
            std::string::npos ||
        protected_runtime.summary().find("subtitle-band=no") == std::string::npos ||
        protected_runtime.summary().find("overlay-passthrough=yes") ==
            std::string::npos ||
        protected_runtime.summary().find("protection-placeholder=yes") ==
            std::string::npos ||
        protected_runtime.summary().find("state-transitions=") == std::string::npos ||
        protected_runtime.summary().find("last-presented-frame=0") == std::string::npos ||
        protected_runtime.summary().find("last-presented-ts=0") == std::string::npos ||
        protected_runtime.summary().find("target-present-ns=166666665") ==
            std::string::npos ||
        protected_runtime.summary().find("predicted-render-ns=2000000") ==
            std::string::npos ||
        protected_runtime.summary().find("present-mode=vsync") == std::string::npos ||
        protected_runtime.summary().find("content-type=video") == std::string::npos ||
        protected_runtime.summary().find("governor=bypass") == std::string::npos ||
        protected_runtime.summary().find("scheduler=passthrough-only") ==
            std::string::npos ||
        protected_runtime.summary().find("frame-taps=5") == std::string::npos ||
        protected_runtime.summary().find("present-feedback=0") == std::string::npos ||
        protected_runtime.hud_text.find("protected=yes") == std::string::npos) {
        std::cerr << "kcm protected runtime snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    const fluxma::ModuleConfig disabled_config {
        .enabled = false,
        .mode = fluxma::ModuleMode::PassthroughOnly,
        .show_hud = false,
        .subtitle_protection = false,
        .cursor_protection = false,
        .log_interval_frames = 3,
        .max_log_messages = 5,
    };
    fluxma::KfiPluginRoot disabled_root(disabled_config);
    const fluxma::KfiKcmBridge disabled_bridge(disabled_root);
    const auto disabled_settings = disabled_bridge.settings();
    if (disabled_settings.enabled ||
        disabled_settings.mode != fluxma::ModuleMode::PassthroughOnly ||
        disabled_settings.show_hud ||
        disabled_settings.subtitle_protection || disabled_settings.cursor_protection ||
        disabled_settings.log_interval_frames != 3 ||
        disabled_settings.summary().find("mode=passthrough-only") ==
            std::string::npos ||
        disabled_settings.summary().find("show-hud=no") == std::string::npos ||
        disabled_settings.max_log_messages != 5) {
        std::cerr << "kcm disabled settings snapshot mismatch\n";
        return EXIT_FAILURE;
    }
    const auto _ = disabled_root.primary_output().on_frame_tapped(frame(1));
    const auto disabled_runtime = disabled_bridge.runtime(10'000'000);
    if (disabled_runtime.state != fluxma::OutputState::Disabled ||
        disabled_runtime.bypass_reason != fluxma::BypassReason::Disabled ||
        disabled_runtime.governor_mode != fluxma::GovernorMode::Bypass ||
        disabled_runtime.scheduler_mode != fluxma::SchedulerMode::PassthroughOnly ||
        disabled_runtime.classifier_allows_interpolation ||
        disabled_runtime.state_transition_count != 1 ||
        !disabled_runtime.passthrough_only ||
        disabled_runtime.cursor_passthrough || disabled_runtime.cursor_recomposite ||
        disabled_runtime.subtitle_band_active || disabled_runtime.overlay_passthrough ||
        !disabled_runtime.protection_placeholder_only ||
        disabled_runtime.last_presentation_mode != fluxma::PresentationMode::VSync ||
        disabled_runtime.last_presented_frame_id != 0 ||
        disabled_runtime.last_presented_timestamp_ns != 0 ||
        disabled_runtime.refresh_interval_ns != 0 ||
        disabled_runtime.last_target_presentation_timestamp_ns != 33'333'333 ||
        disabled_runtime.last_predicted_render_time_ns != 2'000'000 ||
        disabled_runtime.last_content_type != fluxma::ContentType::Video ||
        disabled_runtime.summary().find("classifier=no") == std::string::npos ||
        disabled_runtime.summary().find("cursor-passthrough=no") ==
            std::string::npos ||
        disabled_runtime.summary().find("subtitle-band=no") == std::string::npos ||
        disabled_runtime.summary().find("overlay-passthrough=no") ==
            std::string::npos ||
        disabled_runtime.summary().find("protection-placeholder=yes") ==
            std::string::npos ||
        disabled_runtime.summary().find("state-transitions=") == std::string::npos ||
        disabled_runtime.summary().find("last-presented-frame=0") == std::string::npos ||
        disabled_runtime.summary().find("last-presented-ts=0") == std::string::npos ||
        disabled_runtime.summary().find("target-present-ns=33333333") ==
            std::string::npos ||
        disabled_runtime.summary().find("predicted-render-ns=2000000") ==
            std::string::npos ||
        disabled_runtime.summary().find("present-mode=vsync") == std::string::npos ||
        disabled_runtime.summary().find("content-type=video") == std::string::npos ||
        disabled_runtime.summary().find("governor=bypass") == std::string::npos ||
        disabled_runtime.summary().find("scheduler=passthrough-only") ==
            std::string::npos) {
        std::cerr << "kcm disabled runtime snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    const auto placeholder_native_bridge =
        kcm_bridge.native_bridge_install(fluxma::KwinNativeInstallContext {});
    if (placeholder_native_bridge.state != fluxma::KwinNativeBridgeState::PlaceholderOnly ||
        placeholder_native_bridge.frame_deferred_reason !=
            fluxma::KwinNativeDeferredReason::PlaceholderOnly ||
        placeholder_native_bridge.present_deferred_reason !=
            fluxma::KwinNativeDeferredReason::PlaceholderOnly ||
        placeholder_native_bridge.frame_version_blocked ||
        placeholder_native_bridge.present_version_blocked ||
        placeholder_native_bridge.frame_backend_blocked ||
        placeholder_native_bridge.present_backend_blocked ||
        placeholder_native_bridge.summary().find("frame-deferred=placeholder-only") ==
            std::string::npos ||
        placeholder_native_bridge.install_summary.find("install{") == std::string::npos) {
        std::cerr << "kcm native bridge placeholder snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    const auto backend_blocked_native_bridge = kcm_bridge.native_bridge_install(
        fluxma::KwinNativeInstallContext {
            .kwin_version_supported = true,
            .backend_supported = false,
            .kwin_version = "6.3.90",
            .backend_name = "wayland",
        }
    );
    if (backend_blocked_native_bridge.frame_deferred_reason !=
            fluxma::KwinNativeDeferredReason::BackendGate ||
        backend_blocked_native_bridge.present_deferred_reason !=
            fluxma::KwinNativeDeferredReason::BackendGate ||
        backend_blocked_native_bridge.state !=
            fluxma::KwinNativeBridgeState::PlaceholderOnly ||
        backend_blocked_native_bridge.frame_version_blocked ||
        backend_blocked_native_bridge.present_version_blocked ||
        !backend_blocked_native_bridge.frame_backend_blocked ||
        !backend_blocked_native_bridge.present_backend_blocked ||
        backend_blocked_native_bridge.summary().find("frame-backend-blocked=yes") ==
            std::string::npos ||
        backend_blocked_native_bridge.install_summary.find("backend gate blocked install for wayland")
            == std::string::npos) {
        std::cerr << "kcm native bridge backend gate snapshot mismatch\n";
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
        degraded_runtime.governor_mode != fluxma::GovernorMode::QualityHigh ||
        degraded_runtime.scheduler_mode != fluxma::SchedulerMode::Synthetic2x ||
        !degraded_runtime.classifier_allows_interpolation ||
        degraded_runtime.state_transition_count != 3 ||
        !degraded_runtime.cursor_passthrough || !degraded_runtime.cursor_recomposite ||
        !degraded_runtime.subtitle_band_active || !degraded_runtime.overlay_passthrough ||
        !degraded_runtime.protection_placeholder_only ||
        degraded_runtime.last_presented_frame_id != 5 ||
        degraded_runtime.last_presented_timestamp_ns != 166'666'665 ||
        degraded_runtime.refresh_interval_ns != 16'666'667 ||
        degraded_runtime.last_target_presentation_timestamp_ns != 166'666'665 ||
        degraded_runtime.last_predicted_render_time_ns != 2'000'000 ||
        degraded_runtime.last_presentation_mode != fluxma::PresentationMode::VSync ||
        degraded_runtime.last_content_type != fluxma::ContentType::Video ||
        degraded_runtime.summary().find("classifier=yes") == std::string::npos ||
        degraded_runtime.summary().find("cursor-passthrough=yes") ==
            std::string::npos ||
        degraded_runtime.summary().find("subtitle-band=yes") == std::string::npos ||
        degraded_runtime.summary().find("overlay-passthrough=yes") ==
            std::string::npos ||
        degraded_runtime.summary().find("protection-placeholder=yes") ==
            std::string::npos ||
        degraded_runtime.summary().find("state-transitions=") == std::string::npos ||
        degraded_runtime.summary().find("last-presented-frame=5") == std::string::npos ||
        degraded_runtime.summary().find("last-presented-ts=166666665") ==
            std::string::npos ||
        degraded_runtime.summary().find("target-present-ns=166666665") ==
            std::string::npos ||
        degraded_runtime.summary().find("predicted-render-ns=2000000") ==
            std::string::npos ||
        degraded_runtime.summary().find("present-mode=vsync") == std::string::npos ||
        degraded_runtime.summary().find("content-type=video") == std::string::npos ||
        degraded_runtime.summary().find("governor=quality-high") ==
            std::string::npos ||
        degraded_runtime.summary().find("scheduler=synthetic-2x") ==
            std::string::npos ||
        degraded_runtime.summary().find("deadline-miss=1") == std::string::npos ||
        degraded_runtime.summary().find("synthetic-dropped=1") == std::string::npos) {
        std::cerr << "kcm degraded runtime counters mismatch\n";
        return EXIT_FAILURE;
    }

    const auto version_blocked_native_bridge = degraded_bridge.native_bridge_install(
        fluxma::KwinNativeInstallContext {
            .kwin_version_supported = false,
            .backend_supported = true,
            .kwin_version = "6.3.91",
            .backend_name = "drm",
        }
    );
    if (version_blocked_native_bridge.frame_deferred_reason !=
            fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        version_blocked_native_bridge.present_deferred_reason !=
            fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        version_blocked_native_bridge.state !=
            fluxma::KwinNativeBridgeState::PlaceholderOnly ||
        !version_blocked_native_bridge.frame_version_blocked ||
        !version_blocked_native_bridge.present_version_blocked ||
        version_blocked_native_bridge.frame_backend_blocked ||
        version_blocked_native_bridge.present_backend_blocked ||
        version_blocked_native_bridge.summary().find("present-version-blocked=yes") ==
            std::string::npos ||
        version_blocked_native_bridge.install_summary.find(
            "kwin version gate blocked install for 6.3.91"
        ) == std::string::npos) {
        std::cerr << "kcm native bridge version gate snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
