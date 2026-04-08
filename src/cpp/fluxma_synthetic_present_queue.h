#pragma once

#include "fluxma_types.h"

namespace fluxma {

struct MidframeSynthesisResult;

class KfiSyntheticPresentQueue {
  public:
    [[nodiscard]] SyntheticPresentSubmission enqueue_placeholder(
        const SyntheticFrameArtifact& artifact,
        const ProtectionPlan& protection_plan = {},
        bool protected_content = false,
        bool suppressed_by_protection = false
    ) const noexcept;
    [[nodiscard]] SyntheticPresentSubmission enqueue_synthesized_placeholder(
        std::uint32_t output_id,
        const MidframeSynthesisResult& result
    ) const noexcept;
};

}  // namespace fluxma
