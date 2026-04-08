#pragma once

#include <string>

#include "fluxma_types.h"

namespace fluxma {

class KfiHudRenderer {
  public:
    [[nodiscard]] std::string compose_text(
        std::uint32_t output_id,
        const MetricsSnapshot& snapshot,
        const SyntheticFramePlan& synthetic_plan,
        const SyntheticFrameArtifact& synthetic_artifact,
        const SyntheticPresentSubmission& synthetic_submission,
        const ProtectionPlan& protection_plan
    ) const;
};

}  // namespace fluxma
