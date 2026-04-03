#include <algorithm>
#include <cstdlib>
#include <iostream>

#include "fluxma_kwin_hook_builders.h"
#include "fluxma_plugin_root.h"

int main() {
    fluxma::KfiPluginRoot plugin_root(
        {.enabled = true, .show_hud = true, .log_interval_frames = 1, .max_log_messages = 8}
    );

    auto& output = plugin_root.primary_output();
    auto& hook_adapter = plugin_root.primary_output_hook_adapter();

    const auto decision = hook_adapter.on_final_composed_frame(
        fluxma::FinalComposedFrameEvent {
            .output_id = 0,
            .metadata =
                fluxma::FinalComposedFrameMetadata {
                    .frame_id = 41,
                    .timestamp_ns = 20'000'000,
                },
            .payload =
                fluxma::FinalComposedFramePayload {
                    .width = 1920,
                    .height = 1080,
                    .pixel_format = 0,
                    .color_space = 0,
                    .protected_content = false,
                    .damage_ratio = 0.3,
                    .cursor_visible = false,
                    .cursor_x = 0.0,
                    .cursor_y = 0.0,
                    .cursor_velocity_x = 0.0,
                    .cursor_velocity_y = 0.0,
                    .gpu_handle = fluxma::GpuFrameHandle {.backend_kind = 0, .handle_id = 9001},
                },
        }
    );
    if (decision.bypass_reason != fluxma::BypassReason::GpuPathNotReady) {
        std::cerr << "unexpected decision before mismatch test\n";
        return EXIT_FAILURE;
    }

    hook_adapter.on_render_loop_frame_presented(
        fluxma::KfiKwinPresentBuilder::render_loop_presented_bundle(
            fluxma::KwinPresentFeedbackInputs {
                .output_id = 0,
                .frame_id = 99,
                .presented_timestamp_ns = 21'000'000,
                .refresh_interval_ns = 16'666'667,
                .presentation_mode = fluxma::PresentationMode::VSync,
                .present_success = true,
                .dropped_synthetic = false,
            }
        )
    );

    const auto snapshot = output.snapshot_metrics();
    if (snapshot.last_presented_frame_id != 99 || snapshot.present_feedback_count != 1) {
        std::cerr << "present feedback mismatch snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    const auto logs = output.log_messages();
    const auto has_mismatch_log =
        std::any_of(logs.begin(), logs.end(), [](const std::string& log) {
            return log.find("present-feedback-mismatch=yes") != std::string::npos &&
                   log.find("expected-frame-id=41") != std::string::npos &&
                   log.find("actual-frame-id=99") != std::string::npos;
        });
    if (!has_mismatch_log) {
        std::cerr << "missing present feedback mismatch log\n";
        return EXIT_FAILURE;
    }

    const auto second_decision = hook_adapter.on_final_composed_frame(
        fluxma::FinalComposedFrameEvent {
            .output_id = 0,
            .metadata =
                fluxma::FinalComposedFrameMetadata {
                    .frame_id = 42,
                    .timestamp_ns = 23'000'000,
                },
            .payload =
                fluxma::FinalComposedFramePayload {
                    .width = 1920,
                    .height = 1080,
                    .pixel_format = 0,
                    .color_space = 0,
                    .protected_content = false,
                    .damage_ratio = 0.3,
                    .cursor_visible = false,
                    .cursor_x = 0.0,
                    .cursor_y = 0.0,
                    .cursor_velocity_x = 0.0,
                    .cursor_velocity_y = 0.0,
                    .gpu_handle = fluxma::GpuFrameHandle {.backend_kind = 0, .handle_id = 9002},
                },
        }
    );
    if (second_decision.bypass_reason != fluxma::BypassReason::GpuPathNotReady) {
        std::cerr << "unexpected second decision\n";
        return EXIT_FAILURE;
    }

    hook_adapter.on_render_loop_frame_presented(
        fluxma::KfiKwinPresentBuilder::render_loop_presented_bundle(
            fluxma::KwinPresentFeedbackInputs {
                .output_id = 0,
                .frame_id = 42,
                .presented_timestamp_ns = 24'000'000,
                .refresh_interval_ns = 16'666'667,
                .presentation_mode = fluxma::PresentationMode::VSync,
                .present_success = true,
                .dropped_synthetic = false,
            }
        )
    );
    hook_adapter.on_render_loop_frame_presented(
        fluxma::KfiKwinPresentBuilder::render_loop_presented_bundle(
            fluxma::KwinPresentFeedbackInputs {
                .output_id = 0,
                .frame_id = 41,
                .presented_timestamp_ns = 23'000'000,
                .refresh_interval_ns = 16'666'667,
                .presentation_mode = fluxma::PresentationMode::VSync,
                .present_success = true,
                .dropped_synthetic = false,
            }
        )
    );

    const auto delayed_logs = output.log_messages();
    const auto delayed_mismatch_logs =
        std::count_if(delayed_logs.begin(), delayed_logs.end(), [](const std::string& log) {
            return log.find("present-feedback-mismatch=yes") != std::string::npos;
        });
    if (delayed_mismatch_logs != 1) {
        std::cerr << "delayed valid feedback must not add mismatch logs\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
