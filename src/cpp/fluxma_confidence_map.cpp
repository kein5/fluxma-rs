#include "fluxma_confidence_map.h"

namespace fluxma {

ConfidenceMapBuildResult KfiConfidenceMapBuilder::build(
    const FlowInputBundle& bundle
) const noexcept {
    ConfidenceMapBuildResult result {
        .previous_frame_id = bundle.previous.frame_id,
        .current_frame_id = bundle.current.frame_id,
    };
    if (!bundle.is_usable()) {
        return result;
    }

    const auto level_count = bundle.previous.luma_pyramid.built_levels <
            bundle.current.luma_pyramid.built_levels
        ? bundle.previous.luma_pyramid.built_levels
        : bundle.current.luma_pyramid.built_levels;

    for (std::size_t index = 0; index < level_count && index < result.levels.size(); ++index) {
        const auto& previous_level = bundle.previous.luma_pyramid.levels[index];
        const auto& current_level = bundle.current.luma_pyramid.levels[index];
        result.levels[index] = ConfidenceMapLevel {
            .level_index = static_cast<std::uint32_t>(index),
            .width = previous_level.width,
            .height = previous_level.height,
            .confidence_bias = static_cast<std::uint8_t>(index == 0 ? 255 : 192),
            .placeholder_only = previous_level.placeholder_only && current_level.placeholder_only,
        };
        ++result.built_levels;
    }

    result.valid = result.built_levels != 0;
    return result;
}

}  // namespace fluxma
