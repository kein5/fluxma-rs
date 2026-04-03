#include "fluxma_bypass_pipeline.h"

namespace fluxma {

FrameDescriptor KfiBypassPipeline::capture_frame(const FrameDescriptor& input_frame) const noexcept {
    return frame_tap_.capture(input_frame);
}

PassthroughSubmission KfiBypassPipeline::submit_passthrough(
    std::uint32_t output_id,
    const FrameDescriptor& tapped_frame,
    const OutputDecision& decision
) const noexcept {
    return gpu_services_.submit_passthrough(output_id, tapped_frame, decision);
}

}  // namespace fluxma
