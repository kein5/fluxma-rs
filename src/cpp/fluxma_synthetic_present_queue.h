#pragma once

#include "fluxma_types.h"

namespace fluxma {

class KfiSyntheticPresentQueue {
  public:
    [[nodiscard]] SyntheticPresentSubmission enqueue_placeholder(
        const SyntheticFrameArtifact& artifact
    ) const noexcept;
};

}  // namespace fluxma
