#include "fluxma_synthetic_scheduler.h"

namespace fluxma {

SyntheticFramePlan KfiSyntheticScheduler::plan_placeholder_synthetic(
    std::uint32_t output_id,
    const PassthroughSubmission& submission,
    const OutputDecision& decision,
    const MetricsSnapshot& snapshot,
    std::uint64_t now_ns
) const noexcept {
    SyntheticFramePlan plan {
        .output_id = output_id,
        .source_frame_id = submission.frame_id,
        .synthetic_frame_id = (submission.frame_id * 2) + 1,
        .target_present_timestamp_ns = 0,
        .deadline_timestamp_ns = 0,
        .armed = false,
        .should_drop = false,
    };

    if (!submission.accepted || !decision.interpolation_armed ||
        snapshot.scheduler_mode != SchedulerMode::Synthetic2x ||
        snapshot.refresh_interval_ns == 0 ||
        snapshot.last_target_presentation_timestamp_ns == 0) {
        return plan;
    }

    plan.armed = true;
    plan.target_present_timestamp_ns =
        snapshot.last_target_presentation_timestamp_ns + snapshot.refresh_interval_ns;
    if (snapshot.last_predicted_render_time_ns >= plan.target_present_timestamp_ns) {
        plan.deadline_timestamp_ns = 0;
        plan.should_drop = true;
        return plan;
    }

    plan.deadline_timestamp_ns =
        plan.target_present_timestamp_ns - snapshot.last_predicted_render_time_ns;
    plan.should_drop = now_ns >= plan.deadline_timestamp_ns;
    return plan;
}

}  // namespace fluxma
