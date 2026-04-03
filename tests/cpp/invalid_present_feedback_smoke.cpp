#include <cstdlib>
#include <iostream>

#include "fluxma_kwin_hook_builders.h"
#include "fluxma_plugin_root.h"

int main() {
    fluxma::KfiPluginRoot plugin_root(
        {.enabled = true, .show_hud = true, .log_interval_frames = 1, .max_log_messages = 4}
    );

    auto& output = plugin_root.primary_output();
    auto& hook_adapter = plugin_root.primary_output_hook_adapter();

    const auto expect_ignored = [&](const fluxma::KwinPresentFeedbackInputs& inputs) {
        hook_adapter.on_output_frame_presented(
            fluxma::KfiKwinPresentBuilder::output_frame_presented_bundle(inputs)
        );
        const auto snapshot = output.snapshot_metrics();
        return snapshot.present_feedback_count == 0 && snapshot.last_presented_frame_id == 0 &&
            snapshot.refresh_interval_ns == 0;
    };

    if (!expect_ignored(
            fluxma::KwinPresentFeedbackInputs {
                .output_id = 0,
                .frame_id = 0,
                .presented_timestamp_ns = 0,
                .refresh_interval_ns = 0,
                .presentation_mode = fluxma::PresentationMode::VSync,
                .present_success = true,
                .dropped_synthetic = false,
            }
        ) ||
        !expect_ignored(
            fluxma::KwinPresentFeedbackInputs {
                .output_id = 0,
                .frame_id = 0,
                .presented_timestamp_ns = 4'000'000,
                .refresh_interval_ns = 16'666'667,
                .presentation_mode = fluxma::PresentationMode::VSync,
                .present_success = true,
                .dropped_synthetic = false,
            }
        ) ||
        !expect_ignored(
            fluxma::KwinPresentFeedbackInputs {
                .output_id = 0,
                .frame_id = 7,
                .presented_timestamp_ns = 0,
                .refresh_interval_ns = 16'666'667,
                .presentation_mode = fluxma::PresentationMode::VSync,
                .present_success = true,
                .dropped_synthetic = false,
            }
        ) ||
        !expect_ignored(
            fluxma::KwinPresentFeedbackInputs {
                .output_id = 0,
                .frame_id = 7,
                .presented_timestamp_ns = 4'000'000,
                .refresh_interval_ns = 0,
                .presentation_mode = fluxma::PresentationMode::VSync,
                .present_success = true,
                .dropped_synthetic = false,
            }
        )) {
        std::cerr << "invalid present feedback variants must be ignored\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
