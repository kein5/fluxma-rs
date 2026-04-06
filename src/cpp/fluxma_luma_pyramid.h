#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "fluxma_texture_pool.h"

namespace fluxma {

struct LumaPyramidLevel {
    std::uint32_t level_index = 0;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::uint32_t texture_slot_id = 0;
    std::uint32_t texture_generation = 0;
    bool acquired = false;
    bool placeholder_only = true;
};

struct LumaPyramidBuildResult {
    static constexpr std::size_t kMaxLevels = 6;

    std::uint64_t source_frame_id = 0;
    std::array<LumaPyramidLevel, kMaxLevels> levels {};
    std::size_t built_levels = 0;
    bool valid = false;
    bool truncated = false;
    bool placeholder_only = true;
};

class KfiLumaPyramidBuilder {
  public:
    [[nodiscard]] LumaPyramidBuildResult build(
        std::uint64_t source_frame_id,
        const GpuTextureDescriptor& source_descriptor,
        KfiTexturePool& pool
    ) const noexcept;
    void release(KfiTexturePool& pool, const LumaPyramidBuildResult& pyramid) const noexcept;
};

}  // namespace fluxma
