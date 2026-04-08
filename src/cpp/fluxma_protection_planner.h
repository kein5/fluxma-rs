#pragma once

#include "fluxma_config.h"
#include "fluxma_types.h"

namespace fluxma {

class KfiProtectionPlanner {
  public:
    [[nodiscard]] ProtectionPlan plan(
        const FrameDescriptor& frame,
        const MetricsSnapshot& snapshot,
        const ModuleConfig& config
    ) const noexcept;
};

}  // namespace fluxma
