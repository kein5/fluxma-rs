#pragma once

#include "fluxma_types.h"

namespace fluxma {

struct MidframeSynthesisResult;

class KfiSyntheticPresentQueue {
  public:
    [[nodiscard]] SyntheticPresentSubmission enqueue_placeholder(
        const SyntheticFrameArtifact& artifact
    ) const noexcept;
    [[nodiscard]] SyntheticPresentSubmission enqueue_synthesized_placeholder(
        std::uint32_t output_id,
        const MidframeSynthesisResult& result
    ) const noexcept;
};

}  // namespace fluxma
