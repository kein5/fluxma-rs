#include <cstdlib>
#include <iostream>

#include "fluxma_confidence_map.h"
#include "fluxma_midframe_synth.h"
#include "fluxma_synthetic_present_queue.h"

int main() {
    fluxma::KfiFlowInputsBuilder flow_builder(16);
    const auto flow_inputs = flow_builder.build(
        400,
        fluxma::GpuTextureDescriptor {.width = 1920, .height = 1080, .pixel_format = 1},
        401,
        fluxma::GpuTextureDescriptor {.width = 1920, .height = 1080, .pixel_format = 1}
    );

    fluxma::KfiConfidenceMapBuilder confidence_builder;
    const auto confidence_map = confidence_builder.build(flow_inputs);

    fluxma::KfiMidframeSynthesizer synthesizer;
    const auto result = synthesizer.synthesize(
        fluxma::MidframeSynthesisRequest {
            .flow_inputs = flow_inputs,
            .confidence_map = confidence_map,
            .protection_plan =
                fluxma::ProtectionPlan {
                    .cursor_passthrough = true,
                    .cursor_recomposite = true,
                    .subtitle_band_active = true,
                    .subtitle_band_top = 820,
                    .subtitle_band_bottom = 1080,
                    .placeholder_only = true,
                },
            .synthetic_frame_id = 801,
            .target_present_timestamp_ns = 250'000'000,
            .placeholder_only = true,
        }
    );
    if (!result.synthesized || !result.placeholder_only || result.previous_frame_id != 400 ||
        result.current_frame_id != 401 || result.synthetic_frame_id != 801 ||
        result.target_present_timestamp_ns != 250'000'000 ||
        !result.prefer_current_in_subtitle_band || !result.protection_plan.cursor_passthrough ||
        !result.protection_plan.subtitle_band_active ||
        result.protection_plan.subtitle_band_top != 820 ||
        result.protection_plan.subtitle_band_bottom != 1080) {
        std::cerr << "midframe synth placeholder mismatch\n";
        return EXIT_FAILURE;
    }

    fluxma::KfiSyntheticPresentQueue present_queue;
    const auto submission = present_queue.enqueue_synthesized_placeholder(0, result);
    if (!submission.queued || submission.dropped || !submission.placeholder_only ||
        submission.output_id != 0 || submission.source_frame_id != 401 ||
        submission.synthetic_frame_id != 801 ||
        submission.target_present_timestamp_ns != 250'000'000 ||
        !submission.prefer_current_in_subtitle_band ||
        !submission.protection_plan.cursor_passthrough ||
        !submission.protection_plan.subtitle_band_active ||
        submission.protection_plan.subtitle_band_top != 820 ||
        submission.protection_plan.subtitle_band_bottom != 1080) {
        std::cerr << "midframe synth present submission mismatch\n";
        return EXIT_FAILURE;
    }

    const auto invalid = synthesizer.synthesize(fluxma::MidframeSynthesisRequest {});
    const auto invalid_submission = present_queue.enqueue_synthesized_placeholder(0, invalid);
    if (invalid.synthesized || invalid.prefer_current_in_subtitle_band ||
        invalid.protection_plan.subtitle_band_active || invalid_submission.queued ||
        invalid_submission.prefer_current_in_subtitle_band ||
        invalid_submission.protection_plan.subtitle_band_active) {
        std::cerr << "midframe synth invalid request mismatch\n";
        return EXIT_FAILURE;
    }

    flow_builder.release(flow_inputs);
    return EXIT_SUCCESS;
}
