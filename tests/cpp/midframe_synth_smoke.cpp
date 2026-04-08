#include <cstdlib>
#include <iostream>

#include "fluxma_confidence_map.h"
#include "fluxma_midframe_synth.h"

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
            .synthetic_frame_id = 801,
            .target_present_timestamp_ns = 250'000'000,
            .placeholder_only = true,
        }
    );
    if (!result.synthesized || !result.placeholder_only || result.previous_frame_id != 400 ||
        result.current_frame_id != 401 || result.synthetic_frame_id != 801 ||
        result.target_present_timestamp_ns != 250'000'000) {
        std::cerr << "midframe synth placeholder mismatch\n";
        return EXIT_FAILURE;
    }

    const auto invalid = synthesizer.synthesize(fluxma::MidframeSynthesisRequest {});
    if (invalid.synthesized) {
        std::cerr << "midframe synth invalid request mismatch\n";
        return EXIT_FAILURE;
    }

    flow_builder.release(flow_inputs);
    return EXIT_SUCCESS;
}
