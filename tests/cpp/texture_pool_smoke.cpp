#include <cstdlib>
#include <iostream>

#include "fluxma_texture_pool.h"

int main() {
    fluxma::KfiTexturePool pool(2);

    const auto first = pool.acquire(
        fluxma::GpuTextureDescriptor {.width = 1920, .height = 1080, .pixel_format = 1}
    );
    const auto second = pool.acquire(
        fluxma::GpuTextureDescriptor {.width = 960, .height = 540, .pixel_format = 2}
    );
    const auto overflow = pool.acquire(
        fluxma::GpuTextureDescriptor {.width = 480, .height = 270, .pixel_format = 3}
    );

    if (!first.acquired || !second.acquired || overflow.acquired || !first.placeholder_only ||
        !second.placeholder_only || pool.capacity() != 2 || pool.in_use_count() != 2) {
        std::cerr << "texture pool acquisition contract mismatch\n";
        return EXIT_FAILURE;
    }

    if (!pool.release(first.slot_id) || pool.in_use_count() != 1) {
        std::cerr << "texture pool release mismatch\n";
        return EXIT_FAILURE;
    }

    const auto reused = pool.acquire(
        fluxma::GpuTextureDescriptor {.width = 320, .height = 180, .pixel_format = 4}
    );
    if (!reused.acquired || reused.slot_id != first.slot_id || pool.in_use_count() != 2) {
        std::cerr << "texture pool reuse mismatch\n";
        return EXIT_FAILURE;
    }

    if (pool.acquire({}).acquired || pool.release(0) || pool.release(99)) {
        std::cerr << "texture pool invalid descriptor handling mismatch\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
