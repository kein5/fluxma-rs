#include <cstdlib>
#include <iostream>

#include "fluxma_flow_inputs.h"

int main() {
    fluxma::KfiFlowInputsBuilder builder(16);
    const auto bundle = builder.build(
        100,
        fluxma::GpuTextureDescriptor {.width = 1920, .height = 1080, .pixel_format = 1},
        101,
        fluxma::GpuTextureDescriptor {.width = 1920, .height = 1080, .pixel_format = 1}
    );

    if (!bundle.valid || !bundle.placeholder_only || !bundle.previous.valid ||
        !bundle.current.valid || bundle.previous.frame_id != 100 ||
        bundle.current.frame_id != 101 || !bundle.previous.source_texture.acquired ||
        !bundle.current.source_texture.acquired || bundle.previous.luma_pyramid.built_levels == 0 ||
        bundle.current.luma_pyramid.built_levels == 0 || builder.pooled_texture_count() == 0) {
        std::cerr << "flow input bundle mismatch\n";
        return EXIT_FAILURE;
    }

    builder.release(bundle);
    if (builder.pooled_texture_count() != 0) {
        std::cerr << "flow input release mismatch\n";
        return EXIT_FAILURE;
    }

    fluxma::KfiFlowInputsBuilder constrained_builder(4);
    const auto invalid = constrained_builder.build(
        100,
        fluxma::GpuTextureDescriptor {.width = 0, .height = 1080, .pixel_format = 1},
        101,
        fluxma::GpuTextureDescriptor {.width = 1920, .height = 1080, .pixel_format = 1}
    );
    if (invalid.valid || constrained_builder.pooled_texture_count() != 0) {
        std::cerr << "flow input invalid descriptor mismatch\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
