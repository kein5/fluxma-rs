#include "fluxma_protection_planner.h"

#include <algorithm>

namespace fluxma {

namespace {

constexpr double kSubtitleBandRatio = 0.18;
constexpr std::uint32_t kMinimumSubtitleBandHeight = 48;

}  // namespace

ProtectionPlan KfiProtectionPlanner::plan(
    const FrameDescriptor& frame,
    const MetricsSnapshot& snapshot,
    const ModuleConfig& config
) const noexcept {
    ProtectionPlan result {};

    result.cursor_passthrough = config.cursor_protection && frame.cursor_visible;
    result.cursor_recomposite = result.cursor_passthrough;
    result.transient_overlay_passthrough = frame.overlay_promoted;

    if (snapshot.protected_content || frame.protected_content) {
        return result;
    }

    if (!config.subtitle_protection || frame.content_type != ContentType::Video ||
        frame.height < kMinimumSubtitleBandHeight) {
        return result;
    }

    const auto band_height = std::max<std::uint32_t>(
        kMinimumSubtitleBandHeight,
        static_cast<std::uint32_t>(static_cast<double>(frame.height) * kSubtitleBandRatio)
    );

    if (band_height >= frame.height) {
        result.subtitle_band_active = true;
        result.subtitle_band_top = 0;
        result.subtitle_band_bottom = frame.height;
        return result;
    }

    result.subtitle_band_active = true;
    result.subtitle_band_top = frame.height - band_height;
    result.subtitle_band_bottom = frame.height;
    return result;
}

}  // namespace fluxma
