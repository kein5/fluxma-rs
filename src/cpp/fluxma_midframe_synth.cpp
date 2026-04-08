#include "fluxma_midframe_synth.h"

namespace fluxma {

MidframeSynthesisResult KfiMidframeSynthesizer::synthesize(
    const MidframeSynthesisRequest& request
) const noexcept {
    if (!request.is_usable() || request.synthetic_frame_id == 0 ||
        request.target_present_timestamp_ns == 0) {
        return {};
    }

    return MidframeSynthesisResult {
        .previous_frame_id = request.flow_inputs.previous.frame_id,
        .current_frame_id = request.flow_inputs.current.frame_id,
        .synthetic_frame_id = request.synthetic_frame_id,
        .target_present_timestamp_ns = request.target_present_timestamp_ns,
        .synthesized = true,
        .placeholder_only = true,
    };
}

}  // namespace fluxma
