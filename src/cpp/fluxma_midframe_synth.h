#pragma once

#include <cstdint>

#include "fluxma_confidence_map.h"
#include "fluxma_types.h"

namespace fluxma {

struct MidframeSynthesisRequest {
    FlowInputBundle flow_inputs {};
    ConfidenceMapBuildResult confidence_map {};
    ProtectionPlan protection_plan {};
    std::uint64_t synthetic_frame_id = 0;
    std::uint64_t target_present_timestamp_ns = 0;
    bool placeholder_only = true;

    [[nodiscard]] bool is_usable() const noexcept {
        return flow_inputs.is_usable() && confidence_map.valid;
    }
};

struct MidframeSynthesisResult {
    std::uint64_t previous_frame_id = 0;
    std::uint64_t current_frame_id = 0;
    std::uint64_t synthetic_frame_id = 0;
    std::uint64_t target_present_timestamp_ns = 0;
    ProtectionPlan protection_plan {};
    bool prefer_current_in_subtitle_band = false;
    bool synthesized = false;
    bool placeholder_only = true;

    [[nodiscard]] bool cursor_passthrough() const noexcept {
        return protection_plan.cursor_passthrough;
    }

    [[nodiscard]] bool cursor_recomposite() const noexcept {
        return protection_plan.cursor_recomposite;
    }

    [[nodiscard]] bool subtitle_band_active() const noexcept {
        return protection_plan.subtitle_band_active;
    }

    [[nodiscard]] bool overlay_passthrough() const noexcept {
        return protection_plan.transient_overlay_passthrough;
    }

    [[nodiscard]] bool protection_placeholder_only() const noexcept {
        return protection_plan.placeholder_only;
    }
};

class KfiMidframeSynthesizer {
  public:
    [[nodiscard]] MidframeSynthesisResult synthesize(
        const MidframeSynthesisRequest& request
    ) const noexcept;
};

}  // namespace fluxma
