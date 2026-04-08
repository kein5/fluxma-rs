#include "fluxma_synthetic_present_queue.h"

#include "fluxma_midframe_synth.h"

namespace fluxma {

SyntheticPresentSubmission KfiSyntheticPresentQueue::enqueue_placeholder(
    const SyntheticFrameArtifact& artifact,
    const ProtectionPlan& protection_plan,
    bool protected_content,
    bool suppressed_by_protection
) const noexcept {
    return SyntheticPresentSubmission {
        .output_id = artifact.output_id,
        .source_frame_id = artifact.source_frame_id,
        .synthetic_frame_id = artifact.synthetic_frame_id,
        .target_present_timestamp_ns = artifact.target_present_timestamp_ns,
        .queued = artifact.generated,
        .dropped = artifact.dropped,
        .protected_content = protected_content,
        .protection_plan = protection_plan,
        .prefer_current_in_subtitle_band = false,
        .suppressed_by_protection = suppressed_by_protection,
        .placeholder_only = artifact.placeholder_only,
    };
}

SyntheticPresentSubmission KfiSyntheticPresentQueue::enqueue_synthesized_placeholder(
    std::uint32_t output_id,
    const MidframeSynthesisResult& result
) const noexcept {
    return SyntheticPresentSubmission {
        .output_id = output_id,
        .source_frame_id = result.current_frame_id,
        .synthetic_frame_id = result.synthetic_frame_id,
        .target_present_timestamp_ns = result.target_present_timestamp_ns,
        .queued = result.synthesized,
        .dropped = false,
        .protected_content = false,
        .protection_plan = result.protection_plan,
        .prefer_current_in_subtitle_band = result.prefer_current_in_subtitle_band,
        .suppressed_by_protection = false,
        .placeholder_only = result.placeholder_only,
    };
}

}  // namespace fluxma
