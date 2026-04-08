#include <cstdlib>
#include <iostream>

#include "fluxma_confidence_map.h"

int main() {
    fluxma::KfiFlowInputsBuilder flow_builder(16);
    const auto bundle = flow_builder.build(
        300,
        fluxma::GpuTextureDescriptor {.width = 1920, .height = 1080, .pixel_format = 1},
        301,
        fluxma::GpuTextureDescriptor {.width = 1920, .height = 1080, .pixel_format = 1}
    );

    fluxma::KfiConfidenceMapBuilder confidence_builder;
    const auto confidence = confidence_builder.build(bundle);
    if (!confidence.valid || !confidence.placeholder_only || confidence.previous_frame_id != 300 ||
        confidence.current_frame_id != 301 || confidence.built_levels == 0 ||
        confidence.levels[0].width != 1920 || confidence.levels[0].height != 1080 ||
        confidence.levels[0].confidence_bias != 255 ||
        confidence.levels[1].confidence_bias != 192) {
        std::cerr << "confidence map build mismatch\n";
        return EXIT_FAILURE;
    }

    flow_builder.release(bundle);

    const auto invalid = confidence_builder.build(fluxma::FlowInputBundle {});
    if (invalid.valid || invalid.built_levels != 0) {
        std::cerr << "confidence map invalid bundle mismatch\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
