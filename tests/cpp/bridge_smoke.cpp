#include <algorithm>
#include <cstdlib>
#include <iostream>

#include "fluxma_plugin_root.h"
#include "fluxma_kwin_hook_builders.h"

namespace {

fluxma::KwinCompositorFrameInputs make_frame_inputs(bool protected_content) {
    return fluxma::KwinCompositorFrameInputs {
        .output_id = 0,
        .frame_id = 7,
        .timestamp_ns = 1'000'000,
        .target_presentation_timestamp_ns = 16'666'667,
        .predicted_render_time_ns = 2'000'000,
        .content_type = fluxma::ContentType::Video,
        .width = 1920,
        .height = 1080,
        .pixel_format = 0,
        .color_space = 0,
        .protected_content = protected_content,
        .damage_ratio = 0.25,
        .cursor_visible = false,
        .cursor_x = 0.0,
        .cursor_y = 0.0,
        .cursor_velocity_x = 0.0,
        .cursor_velocity_y = 0.0,
        .gpu_handle = fluxma::GpuFrameHandle {.backend_kind = 0, .handle_id = 99},
        .cursor_composited_in_frame = true,
        .overlay_promoted = false,
    };
}

fluxma::KwinPresentFeedbackInputs make_present_inputs(
    std::uint64_t frame_id,
    std::uint64_t timestamp_ns,
    fluxma::PresentationMode mode,
    bool present_success,
    bool dropped_synthetic
) {
    return fluxma::KwinPresentFeedbackInputs {
        .output_id = 0,
        .frame_id = frame_id,
        .presented_timestamp_ns = timestamp_ns,
        .refresh_interval_ns = 16'666'667,
        .presentation_mode = mode,
        .present_success = present_success,
        .dropped_synthetic = dropped_synthetic,
    };
}

}  // namespace

int main() {
    const fluxma::KwinNativeBridgeObservationReport asymmetric_observation {
        .preflight = fluxma::KwinNativeCombinedPreflightReport {
            .frame = fluxma::KwinNativeInstallPreflightReport {
                .gate = fluxma::KwinNativeInstallGateAssessment {
                    .deferred_reason = fluxma::KwinNativeDeferredReason::PlaceholderOnly,
                },
            },
            .present = fluxma::KwinNativeInstallPreflightReport {
                .gate = fluxma::KwinNativeInstallGateAssessment {
                    .deferred_reason = fluxma::KwinNativeDeferredReason::BackendGate,
                    .backend_blocked = true,
                },
            },
        },
        .install = fluxma::KwinNativeCombinedInstallReport {
            .frame = fluxma::KwinNativeInstallReport {
                .result = fluxma::KwinNativeInstallResult::Deferred,
                .deferred_reason = fluxma::KwinNativeDeferredReason::PlaceholderOnly,
            },
            .present = fluxma::KwinNativeInstallReport {
                .result = fluxma::KwinNativeInstallResult::Deferred,
                .deferred_reason = fluxma::KwinNativeDeferredReason::BackendGate,
            },
        },
    };
    if (asymmetric_observation.frame_preflight_deferred_reason() !=
            fluxma::KwinNativeDeferredReason::PlaceholderOnly ||
        asymmetric_observation.present_preflight_deferred_reason() !=
            fluxma::KwinNativeDeferredReason::BackendGate ||
        asymmetric_observation.frame_install_deferred_reason() !=
            fluxma::KwinNativeDeferredReason::PlaceholderOnly ||
        asymmetric_observation.present_install_deferred_reason() !=
            fluxma::KwinNativeDeferredReason::BackendGate ||
        asymmetric_observation.frame_preflight_has_any_blocker() ||
        !asymmetric_observation.present_preflight_has_any_blocker()) {
        std::cerr << "bridge smoke must preserve asymmetric observation helper wiring\n";
        return EXIT_FAILURE;
    }

    fluxma::KfiPluginRoot plugin_root(
        {.enabled = true, .show_hud = true, .log_interval_frames = 2, .max_log_messages = 4}
    );

    auto& output = plugin_root.primary_output();
    auto& hook_adapter = plugin_root.primary_output_hook_adapter();
    const auto normal_frame_inputs = make_frame_inputs(false);
    const auto normal_frame_hook =
        fluxma::KfiKwinFrameBuilder::compositor_bundle(normal_frame_inputs);
    const auto normal = hook_adapter.on_compositor_output_frame_ready(normal_frame_hook);
    if (!normal.passthrough_only ||
        normal.bypass_reason != fluxma::BypassReason::GpuPathNotReady) {
        std::cerr << "unexpected normal decision: "
                  << fluxma::to_string(normal.bypass_reason) << '\n';
        return EXIT_FAILURE;
    }
    const auto normal_submission = output.last_submission();
    if (!normal_submission.accepted ||
        normal_submission.bypass_reason != fluxma::BypassReason::GpuPathNotReady ||
        normal_submission.frame_id != 7) {
        std::cerr << "unexpected passthrough submission\n";
        return EXIT_FAILURE;
    }

    const auto first_present = make_present_inputs(
        7,
        2'000'000,
        fluxma::PresentationMode::VSync,
        true,
        false
    );
    const auto first_present_hook =
        fluxma::KfiKwinPresentBuilder::output_frame_presented_bundle(first_present);
    hook_adapter.on_output_frame_presented(first_present_hook);
    const auto second_present = make_present_inputs(
        8,
        3'000'000,
        fluxma::PresentationMode::AdaptiveSync,
        false,
        true
    );
    const auto second_present_hook =
        fluxma::KfiKwinPresentBuilder::output_frame_presented_bundle(second_present);
    hook_adapter.on_output_frame_presented(second_present_hook);

    const auto protected_frame_inputs = make_frame_inputs(true);
    const auto protected_frame_hook =
        fluxma::KfiKwinFrameBuilder::compositor_bundle(protected_frame_inputs);
    const auto protected_frame = hook_adapter.on_compositor_output_frame_ready(protected_frame_hook);
    if (!protected_frame.passthrough_only ||
        protected_frame.bypass_reason != fluxma::BypassReason::ProtectedContent) {
        std::cerr << "protected content must remain passthrough-only\n";
        return EXIT_FAILURE;
    }
    const auto protected_submission = output.last_submission();
    if (!protected_submission.accepted || !protected_submission.protected_content ||
        protected_submission.bypass_reason != fluxma::BypassReason::ProtectedContent) {
        std::cerr << "protected submission must remain passthrough-only\n";
        return EXIT_FAILURE;
    }

    const auto bridge_observation = plugin_root.observe_native_bridge(
        normal_frame_inputs,
        first_present,
        fluxma::KwinNativeInstallContext {
            .kwin_version_supported = true,
            .backend_supported = false,
            .kwin_version = "6.3.80",
            .backend_name = "wayland",
        }
    );
    const auto bridge_observation_summary = bridge_observation.summary();
    const auto install_observation = plugin_root.observe_native_bridge_install(
        fluxma::KwinNativeInstallContext {
            .kwin_version_supported = false,
            .backend_supported = true,
            .kwin_version = "6.3.81",
            .backend_name = "drm",
        }
    );
    const auto install_observation_summary = install_observation.summary();
    const auto bridge_observation_frame_summary = bridge_observation.bringup.frame_summary();
    if (!bridge_observation.is_placeholder_state() ||
        bridge_observation.bringup.state != fluxma::KwinNativeBridgeState::PlaceholderOnly ||
        !bridge_observation.bringup_complete() ||
        !bridge_observation.frame_gate_matches() ||
        !bridge_observation.present_gate_matches() ||
        !bridge_observation.all_gates_match() ||
        !bridge_observation.frame_install_deferred() ||
        !bridge_observation.present_install_deferred() ||
        !bridge_observation.all_installs_deferred() ||
        bridge_observation.frame_preflight_deferred_reason() !=
            fluxma::KwinNativeDeferredReason::BackendGate ||
        bridge_observation.present_preflight_deferred_reason() !=
            fluxma::KwinNativeDeferredReason::BackendGate ||
        bridge_observation.frame_install_deferred_reason() !=
            fluxma::KwinNativeDeferredReason::BackendGate ||
        bridge_observation.present_install_deferred_reason() !=
            fluxma::KwinNativeDeferredReason::BackendGate ||
        bridge_observation.frame_preflight_version_blocked() ||
        bridge_observation.present_preflight_version_blocked() ||
        !bridge_observation.frame_preflight_backend_blocked() ||
        !bridge_observation.present_preflight_backend_blocked() ||
        !bridge_observation.frame_preflight_has_any_blocker() ||
        !bridge_observation.present_preflight_has_any_blocker() ||
        bridge_observation_frame_summary.find("hook=compositor-output-frame-ready") ==
            std::string::npos ||
        bridge_observation.preflight.frame.gate.deferred_reason !=
            fluxma::KwinNativeDeferredReason::BackendGate ||
        bridge_observation.install.frame.reason != "backend gate blocked install for wayland" ||
        bridge_observation_summary.find("preflight{") ==
            std::string::npos ||
        bridge_observation_summary.find("install{") ==
            std::string::npos ||
        !install_observation.is_placeholder_state() ||
        !install_observation.frame_gate_matches() ||
        !install_observation.present_gate_matches() ||
        !install_observation.all_gates_match() ||
        !install_observation.frame_install_deferred() ||
        !install_observation.present_install_deferred() ||
        !install_observation.all_installs_deferred() ||
        install_observation.frame_preflight_deferred_reason() !=
            fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        install_observation.present_preflight_deferred_reason() !=
            fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        install_observation.frame_install_deferred_reason() !=
            fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        install_observation.present_install_deferred_reason() !=
            fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        !install_observation.frame_preflight_version_blocked() ||
        !install_observation.present_preflight_version_blocked() ||
        install_observation.frame_preflight_backend_blocked() ||
        install_observation.present_preflight_backend_blocked() ||
        !install_observation.frame_preflight_has_any_blocker() ||
        !install_observation.present_preflight_has_any_blocker() ||
        install_observation.preflight.frame.gate.deferred_reason !=
            fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        install_observation.install.frame.reason !=
            "kwin version gate blocked install for 6.3.81" ||
        install_observation_summary.find("preflight{") ==
            std::string::npos ||
        install_observation_summary.find("install{") ==
            std::string::npos) {
        std::cerr << "plugin root must surface native bridge observation summary\n";
        return EXIT_FAILURE;
    }

    fluxma::KfiPluginRoot synthetic_plugin_root(
        {.enabled = true, .show_hud = true, .log_interval_frames = 1, .max_log_messages = 8}
    );
    for (std::uint64_t frame_id = 1; frame_id <= 5; ++frame_id) {
        const auto _ = synthetic_plugin_root.primary_output().on_frame_tapped(
            fluxma::FrameDescriptor {
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
                .gpu_handle =
                    fluxma::GpuFrameHandle {.backend_kind = 1, .handle_id = frame_id + 100},
            }
        );
    }
    synthetic_plugin_root.primary_output().on_present_feedback(
        fluxma::PresentFeedback {
            .frame_id = 5,
            .presented_timestamp_ns = 166'666'665,
            .refresh_interval_ns = 16'666'667,
            .presentation_mode = fluxma::PresentationMode::VSync,
            .present_success = true,
            .dropped_synthetic = false,
        }
    );
    const auto output_runtime_observation =
        synthetic_plugin_root.observe_output_runtime(47'999'999);
    if (!output_runtime_observation.synthetic_armed() ||
        output_runtime_observation.synthetic_should_drop() ||
        !output_runtime_observation.synthetic_generated() ||
        output_runtime_observation.summary().find("synthetic-armed=yes") ==
            std::string::npos) {
        std::cerr << "plugin root must surface synthetic runtime observation summary\n";
        return EXIT_FAILURE;
    }

    const auto snapshot = output.snapshot_metrics();
    if (snapshot.frame_tap_count != 2 || snapshot.present_feedback_count != 2 ||
        snapshot.deadline_miss_count != 1 || snapshot.dropped_synthetic_count != 1 ||
        snapshot.last_presented_frame_id != 8 ||
        snapshot.last_target_presentation_timestamp_ns != 16'666'667 ||
        snapshot.last_predicted_render_time_ns != 2'000'000 ||
        snapshot.last_presentation_mode != fluxma::PresentationMode::AdaptiveSync ||
        snapshot.last_content_type != fluxma::ContentType::Video ||
        snapshot.state != fluxma::OutputState::ProtectedBypass || !snapshot.protected_content) {
        std::cerr << "unexpected metrics snapshot\n";
        return EXIT_FAILURE;
    }

    const auto hud = output.render_hud_text();
    if (hud.find("state=protected-bypass") == std::string::npos ||
        hud.find("bypass=protected-content") == std::string::npos ||
        hud.find("protected=yes") == std::string::npos ||
        hud.find("last_presented_frame_id=8") == std::string::npos ||
        hud.find("target_present_ns=16666667") == std::string::npos ||
        hud.find("predicted_render_ns=2000000") == std::string::npos ||
        hud.find("present_mode=adaptive-sync") == std::string::npos ||
        hud.find("content_type=video") == std::string::npos ||
        hud.find("deadline_miss=1") == std::string::npos) {
        std::cerr << "unexpected hud text\n";
        return EXIT_FAILURE;
    }

    const auto& logs = output.log_messages();
    if (logs.size() < 2) {
        std::cerr << "expected state and present feedback logs\n";
        return EXIT_FAILURE;
    }
    const auto has_dropped_synthetic_log =
        std::any_of(logs.begin(), logs.end(), [](const std::string& log) {
            return log.find("dropped-synthetic=yes") != std::string::npos;
        });
    if (!has_dropped_synthetic_log) {
        std::cerr << "unexpected log message\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
