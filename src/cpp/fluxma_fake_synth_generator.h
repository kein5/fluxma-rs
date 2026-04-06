#pragma once

#include "fluxma_types.h"

namespace fluxma {

class KfiFakeSynthGenerator {
  public:
    [[nodiscard]] SyntheticFrameArtifact generate(
        const SyntheticFramePlan& plan
    ) const noexcept;
};

}  // namespace fluxma
