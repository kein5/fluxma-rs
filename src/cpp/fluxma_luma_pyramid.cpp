#include "fluxma_luma_pyramid.h"

namespace fluxma {

LumaPyramidBuildResult KfiLumaPyramidBuilder::build(
    std::uint64_t source_frame_id,
    const GpuTextureDescriptor& source_descriptor,
    KfiTexturePool& pool
) const noexcept {
    LumaPyramidBuildResult result {.source_frame_id = source_frame_id};
    if (!source_descriptor.is_valid()) {
        return result;
    }

    std::uint32_t width = source_descriptor.width;
    std::uint32_t height = source_descriptor.height;

    for (std::size_t level_index = 0; level_index < result.levels.size(); ++level_index) {
        const auto lease = pool.acquire(
            GpuTextureDescriptor {
                .width = width,
                .height = height,
                .pixel_format = source_descriptor.pixel_format,
            }
        );
        if (!lease.acquired) {
            result.truncated = true;
            break;
        }

        result.levels[level_index] = LumaPyramidLevel {
            .level_index = static_cast<std::uint32_t>(level_index),
            .width = width,
            .height = height,
            .texture_slot_id = lease.slot_id,
            .texture_generation = lease.generation,
            .acquired = true,
            .placeholder_only = lease.placeholder_only,
        };
        ++result.built_levels;
        result.valid = true;

        if (width == 1 && height == 1) {
            break;
        }

        width = width > 1 ? width / 2 : 1;
        height = height > 1 ? height / 2 : 1;
    }

    return result;
}

void KfiLumaPyramidBuilder::release(
    KfiTexturePool& pool,
    const LumaPyramidBuildResult& pyramid
) const noexcept {
    for (std::size_t index = 0; index < pyramid.built_levels; ++index) {
        const auto& level = pyramid.levels[index];
        if (level.acquired) {
            static_cast<void>(pool.release(GpuTextureLease {
                .slot_id = level.texture_slot_id,
                .generation = level.texture_generation,
                .acquired = true,
                .placeholder_only = level.placeholder_only,
            }));
        }
    }
}

}  // namespace fluxma
