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
};

class KfiMidframeSynthesizer {
  public:
    [[nodiscard]] MidframeSynthesisResult synthesize(
        const MidframeSynthesisRequest& request
    ) const noexcept;
};

}  // namespace fluxma
