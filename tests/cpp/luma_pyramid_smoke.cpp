#include <cstdlib>
#include <iostream>

#include "fluxma_luma_pyramid.h"

int main() {
    fluxma::KfiTexturePool pool(6);
    fluxma::KfiLumaPyramidBuilder builder;

    const auto pyramid = builder.build(
        77,
        fluxma::GpuTextureDescriptor {.width = 1920, .height = 1080, .pixel_format = 1},
        pool
    );
    if (!pyramid.valid || pyramid.truncated || !pyramid.placeholder_only ||
        pyramid.source_frame_id != 77 || pyramid.built_levels != 6 ||
        pyramid.levels[0].width != 1920 || pyramid.levels[0].height != 1080 ||
        pyramid.levels[1].width != 960 || pyramid.levels[1].height != 540 ||
        pyramid.levels[2].width != 480 || pyramid.levels[2].height != 270 ||
        !pyramid.levels[0].acquired || !pyramid.levels[5].acquired || pool.in_use_count() != 6) {
        std::cerr << "luma pyramid build mismatch\n";
        return EXIT_FAILURE;
    }

    builder.release(pool, pyramid);
    if (pool.in_use_count() != 0) {
        std::cerr << "luma pyramid release mismatch\n";
        return EXIT_FAILURE;
    }

    fluxma::KfiTexturePool small_pool(2);
    const auto truncated = builder.build(
        78,
        fluxma::GpuTextureDescriptor {.width = 1280, .height = 720, .pixel_format = 1},
        small_pool
    );
    if (!truncated.valid || !truncated.truncated || truncated.built_levels != 2 ||
        truncated.levels[1].width != 640 || truncated.levels[1].height != 360) {
        std::cerr << "luma pyramid truncation mismatch\n";
        return EXIT_FAILURE;
    }

    const auto invalid = builder.build(79, {}, small_pool);
    if (invalid.valid || invalid.built_levels != 0 || invalid.truncated) {
        std::cerr << "luma pyramid invalid source mismatch\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
