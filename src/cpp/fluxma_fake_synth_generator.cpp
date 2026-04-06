#include "fluxma_fake_synth_generator.h"

namespace fluxma {

SyntheticFrameArtifact KfiFakeSynthGenerator::generate(
    const SyntheticFramePlan& plan
) const noexcept {
    return SyntheticFrameArtifact {
        .output_id = plan.output_id,
        .source_frame_id = plan.source_frame_id,
        .synthetic_frame_id = plan.synthetic_frame_id,
        .target_present_timestamp_ns = plan.target_present_timestamp_ns,
        .generated = plan.armed && !plan.should_drop,
        .dropped = plan.armed && plan.should_drop,
        .placeholder_only = true,
    };
}

}  // namespace fluxma
