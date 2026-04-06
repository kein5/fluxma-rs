#include "fluxma_hud_renderer.h"

#include <sstream>

namespace fluxma {

std::string KfiHudRenderer::compose_text(
    std::uint32_t output_id,
    const MetricsSnapshot& snapshot,
    const SyntheticFramePlan& synthetic_plan
) const {
    std::ostringstream stream;
    stream << "Fluxma output=" << output_id << '\n';
    stream << "state=" << to_string(snapshot.state) << '\n';
    stream << "bypass=" << to_string(snapshot.bypass_reason) << '\n';
    stream << "protected=" << to_bool_string(snapshot.protected_content) << '\n';
    stream << "passthrough=" << to_bool_string(snapshot.passthrough_only) << '\n';
    stream << "frame_taps=" << snapshot.frame_tap_count << '\n';
    stream << "present_feedback=" << snapshot.present_feedback_count << '\n';
    stream << "deadline_miss=" << snapshot.deadline_miss_count << '\n';
    stream << "dropped_synthetic=" << snapshot.dropped_synthetic_count << '\n';
    stream << "last_presented_frame_id=" << snapshot.last_presented_frame_id << '\n';
    stream << "refresh_ns=" << snapshot.refresh_interval_ns << '\n';
    stream << "target_present_ns=" << snapshot.last_target_presentation_timestamp_ns << '\n';
    stream << "predicted_render_ns=" << snapshot.last_predicted_render_time_ns << '\n';
    stream << "present_mode=" << to_string(snapshot.last_presentation_mode) << '\n';
    stream << "content_type=" << to_string(snapshot.last_content_type) << '\n';
    stream << "cadence_status=" << to_string(snapshot.cadence_status) << '\n';
    stream << "cadence_millihz=" << snapshot.cadence_hz_millihz << '\n';
    stream << "classifier=" << to_bool_string(snapshot.classifier_allows_interpolation) << '\n';
    stream << "governor=" << to_string(snapshot.governor_mode) << '\n';
    stream << "scheduler=" << to_string(snapshot.scheduler_mode) << '\n';
    stream << "state_transitions=" << snapshot.state_transition_count << '\n';
    stream << "synthetic_armed=" << to_bool_string(synthetic_plan.armed) << '\n';
    stream << "synthetic_drop=" << to_bool_string(synthetic_plan.should_drop) << '\n';
    stream << "synthetic_target_ns=" << synthetic_plan.target_present_timestamp_ns << '\n';
    stream << "synthetic_deadline_ns=" << synthetic_plan.deadline_timestamp_ns;
    return stream.str();
}

}  // namespace fluxma
