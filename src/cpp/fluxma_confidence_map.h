#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "fluxma_flow_inputs.h"

namespace fluxma {

struct ConfidenceMapLevel {
    std::uint32_t level_index = 0;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::uint8_t confidence_bias = 0;
    bool placeholder_only = true;
};

struct ConfidenceMapBuildResult {
    static constexpr std::size_t kMaxLevels = 6;

    std::uint64_t previous_frame_id = 0;
    std::uint64_t current_frame_id = 0;
    std::array<ConfidenceMapLevel, kMaxLevels> levels {};
    std::size_t built_levels = 0;
    bool valid = false;
    bool placeholder_only = true;
};

class KfiConfidenceMapBuilder {
  public:
    [[nodiscard]] ConfidenceMapBuildResult build(
        const FlowInputBundle& bundle
    ) const noexcept;
};

}  // namespace fluxma
