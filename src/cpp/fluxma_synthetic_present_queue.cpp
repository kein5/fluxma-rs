#include "fluxma_synthetic_present_queue.h"

namespace fluxma {

SyntheticPresentSubmission KfiSyntheticPresentQueue::enqueue_placeholder(
    const SyntheticFrameArtifact& artifact
) const noexcept {
    return SyntheticPresentSubmission {
        .output_id = artifact.output_id,
        .source_frame_id = artifact.source_frame_id,
        .synthetic_frame_id = artifact.synthetic_frame_id,
        .target_present_timestamp_ns = artifact.target_present_timestamp_ns,
        .queued = artifact.generated,
        .dropped = artifact.dropped,
        .placeholder_only = artifact.placeholder_only,
    };
}

}  // namespace fluxma
