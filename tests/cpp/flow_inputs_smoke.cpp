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

    if (!bundle.valid || !bundle.is_usable() || !bundle.has_live_resources() ||
        !bundle.placeholder_only || !bundle.previous.valid ||
        !bundle.current.valid || bundle.truncated || bundle.previous.frame_id != 100 ||
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

    fluxma::KfiFlowInputsBuilder invalid_builder(16);
    const auto invalid = invalid_builder.build(
        100,
        fluxma::GpuTextureDescriptor {.width = 0, .height = 1080, .pixel_format = 1},
        101,
        fluxma::GpuTextureDescriptor {.width = 1920, .height = 1080, .pixel_format = 1}
    );
    if (invalid.valid || invalid.truncated || invalid.has_live_resources() ||
        invalid_builder.pooled_texture_count() != 0) {
        std::cerr << "flow input invalid descriptor mismatch\n";
        return EXIT_FAILURE;
    }

    fluxma::KfiFlowInputsBuilder truncated_builder(8);
    const auto truncated = truncated_builder.build(
        200,
        fluxma::GpuTextureDescriptor {.width = 1920, .height = 1080, .pixel_format = 1},
        201,
        fluxma::GpuTextureDescriptor {.width = 1920, .height = 1080, .pixel_format = 1}
    );
    if (truncated.valid || truncated.is_usable() || !truncated.truncated ||
        truncated.has_live_resources() || truncated_builder.pooled_texture_count() != 0) {
        std::cerr << "flow input truncation cleanup mismatch\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
