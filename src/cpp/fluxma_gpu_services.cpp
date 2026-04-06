#include "fluxma_gpu_services.h"

namespace fluxma {

PassthroughSubmission KfiGpuServices::submit_passthrough(
    std::uint32_t output_id,
    const FrameDescriptor& frame,
    const OutputDecision& decision
) const noexcept {
    // TODO: Replace this with the real KWin native present path once the internal hook is
    // confirmed. This skeleton only models passthrough submission metadata.
    return PassthroughSubmission {
        .output_id = output_id,
        .frame_id = frame.frame_id,
        .source_frame = frame.gpu_handle,
        .accepted = decision.passthrough_only,
        .interpolation_armed = decision.interpolation_armed,
        .protected_content = frame.protected_content,
        .bypass_reason = decision.bypass_reason,
    };
}

}  // namespace fluxma
