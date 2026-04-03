#pragma once

#include "fluxma_types.h"

namespace fluxma {

class KfiGpuServices {
  public:
    [[nodiscard]] PassthroughSubmission submit_passthrough(
        std::uint32_t output_id,
        const FrameDescriptor& frame,
        const OutputDecision& decision
    ) const noexcept;
};

}  // namespace fluxma
