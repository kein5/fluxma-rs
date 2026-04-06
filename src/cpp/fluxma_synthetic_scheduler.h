#pragma once

#include <cstdint>

#include "fluxma_types.h"

namespace fluxma {

class KfiSyntheticScheduler {
  public:
    [[nodiscard]] SyntheticFramePlan plan_placeholder_synthetic(
        std::uint32_t output_id,
        const PassthroughSubmission& submission,
        const OutputDecision& decision,
        const MetricsSnapshot& snapshot,
        std::uint64_t now_ns
    ) const noexcept;
};

}  // namespace fluxma
