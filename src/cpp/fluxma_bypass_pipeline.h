#pragma once

#include "fluxma_frame_tap.h"
#include "fluxma_gpu_services.h"
#include "fluxma_types.h"

namespace fluxma {

class KfiBypassPipeline {
  public:
    [[nodiscard]] FrameDescriptor capture_frame(const FrameDescriptor& input_frame) const noexcept;
    [[nodiscard]] PassthroughSubmission submit_passthrough(
        std::uint32_t output_id,
        const FrameDescriptor& tapped_frame,
        const OutputDecision& decision
    ) const noexcept;

  private:
    KfiFrameTap frame_tap_ {};
    KfiGpuServices gpu_services_ {};
};

}  // namespace fluxma
