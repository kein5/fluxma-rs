#include "fluxma_logger.h"

#include <sstream>

namespace fluxma {

KfiRateLimitedLogger::KfiRateLimitedLogger(
    std::uint64_t interval_frames,
    std::size_t max_messages
)
    : interval_frames_(interval_frames == 0 ? 1 : interval_frames),
      max_messages_(max_messages == 0 ? 1 : max_messages) {}

void KfiRateLimitedLogger::note_state_transition(
    std::uint32_t output_id,
    std::uint64_t frame_tap_count,
    std::uint64_t sequence,
    OutputState state,
    BypassReason bypass_reason,
    CadenceStatus cadence_status,
    GovernorMode governor_mode,
    SchedulerMode scheduler_mode,
    bool classifier_allows_interpolation,
    bool protected_content
) noexcept {
    if (has_last_logged_state_transition_frame_tap_count_ &&
        frame_tap_count <
            (last_logged_state_transition_frame_tap_count_ + interval_frames_)) {
        return;
    }

    last_logged_state_transition_frame_tap_count_ = frame_tap_count;
    has_last_logged_state_transition_frame_tap_count_ = true;

    append(
        LogEvent {
            .kind = LogEventKind::StateTransition,
            .output_id = output_id,
            .sequence = sequence,
            .frame_tap_count = frame_tap_count,
            .present_feedback_count = 0,
            .state = state,
            .bypass_reason = bypass_reason,
            .cadence_status = cadence_status,
            .governor_mode = governor_mode,
            .scheduler_mode = scheduler_mode,
            .classifier_allows_interpolation = classifier_allows_interpolation,
            .protected_content = protected_content,
            .present_success = true,
            .dropped_synthetic = false,
            .synthetic_armed = false,
            .synthetic_should_drop = false,
            .synthetic_generated = false,
            .synthetic_placeholder_only = true,
            .synthetic_target_timestamp_ns = 0,
            .synthetic_deadline_timestamp_ns = 0,
        }
    );
}

void KfiRateLimitedLogger::note_present_feedback_issue(
    std::uint32_t output_id,
    std::uint64_t frame_tap_count,
    std::uint64_t present_feedback_count,
    bool present_success,
    bool dropped_synthetic
) noexcept {
    if (has_last_logged_present_feedback_issue_count_ &&
        present_feedback_count <
            (last_logged_present_feedback_issue_count_ + interval_frames_)) {
        return;
    }

    last_logged_present_feedback_issue_count_ = present_feedback_count;
    has_last_logged_present_feedback_issue_count_ = true;

    append(
        LogEvent {
            .kind = LogEventKind::PresentFeedbackIssue,
            .output_id = output_id,
            .sequence = 0,
            .frame_tap_count = frame_tap_count,
            .present_feedback_count = present_feedback_count,
            .state = OutputState::Bypass,
            .bypass_reason = BypassReason::None,
            .cadence_status = CadenceStatus::Unknown,
            .governor_mode = GovernorMode::Bypass,
            .scheduler_mode = SchedulerMode::PassthroughOnly,
            .classifier_allows_interpolation = false,
            .protected_content = false,
            .present_success = present_success,
            .dropped_synthetic = dropped_synthetic,
            .synthetic_armed = false,
            .synthetic_should_drop = false,
            .synthetic_generated = false,
            .synthetic_placeholder_only = true,
            .synthetic_target_timestamp_ns = 0,
            .synthetic_deadline_timestamp_ns = 0,
            .expected_frame_id = 0,
            .actual_frame_id = 0,
        }
    );
}

void KfiRateLimitedLogger::note_present_feedback_mismatch(
    std::uint32_t output_id,
    std::uint64_t frame_tap_count,
    std::uint64_t present_feedback_count,
    std::uint64_t expected_frame_id,
    std::uint64_t actual_frame_id
) noexcept {
    if (has_last_logged_present_feedback_mismatch_count_ &&
        present_feedback_count <
            (last_logged_present_feedback_mismatch_count_ + interval_frames_)) {
        return;
    }

    last_logged_present_feedback_mismatch_count_ = present_feedback_count;
    has_last_logged_present_feedback_mismatch_count_ = true;

    append(
        LogEvent {
            .kind = LogEventKind::PresentFeedbackMismatch,
            .output_id = output_id,
            .sequence = 0,
            .frame_tap_count = frame_tap_count,
            .present_feedback_count = present_feedback_count,
            .state = OutputState::Bypass,
            .bypass_reason = BypassReason::None,
            .cadence_status = CadenceStatus::Unknown,
            .governor_mode = GovernorMode::Bypass,
            .scheduler_mode = SchedulerMode::PassthroughOnly,
            .classifier_allows_interpolation = false,
            .protected_content = false,
            .present_success = true,
            .dropped_synthetic = false,
            .synthetic_armed = false,
            .synthetic_should_drop = false,
            .synthetic_generated = false,
            .synthetic_placeholder_only = true,
            .synthetic_target_timestamp_ns = 0,
            .synthetic_deadline_timestamp_ns = 0,
            .expected_frame_id = expected_frame_id,
            .actual_frame_id = actual_frame_id,
        }
    );
}

void KfiRateLimitedLogger::note_synthetic_plan(
    std::uint32_t output_id,
    std::uint64_t frame_tap_count,
    const SyntheticFramePlan& plan
) noexcept {
    const bool state_changed = !has_last_logged_synthetic_plan_frame_tap_count_ ||
        plan.armed != last_logged_synthetic_armed_ ||
        plan.should_drop != last_logged_synthetic_should_drop_;
    if (!state_changed && has_last_logged_synthetic_plan_frame_tap_count_ &&
        frame_tap_count < (last_logged_synthetic_plan_frame_tap_count_ + interval_frames_)) {
        return;
    }

    last_logged_synthetic_plan_frame_tap_count_ = frame_tap_count;
    has_last_logged_synthetic_plan_frame_tap_count_ = true;
    last_logged_synthetic_armed_ = plan.armed;
    last_logged_synthetic_should_drop_ = plan.should_drop;

    append(
        LogEvent {
            .kind = LogEventKind::SyntheticPlan,
            .output_id = output_id,
            .sequence = 0,
            .frame_tap_count = frame_tap_count,
            .present_feedback_count = 0,
            .state = OutputState::Bypass,
            .bypass_reason = BypassReason::None,
            .cadence_status = CadenceStatus::Unknown,
            .governor_mode = GovernorMode::Bypass,
            .scheduler_mode = SchedulerMode::PassthroughOnly,
            .classifier_allows_interpolation = false,
            .protected_content = false,
            .present_success = true,
            .dropped_synthetic = false,
            .synthetic_armed = plan.armed,
            .synthetic_should_drop = plan.should_drop,
            .synthetic_generated = false,
            .synthetic_placeholder_only = true,
            .synthetic_target_timestamp_ns = plan.target_present_timestamp_ns,
            .synthetic_deadline_timestamp_ns = plan.deadline_timestamp_ns,
            .expected_frame_id = plan.source_frame_id,
            .actual_frame_id = plan.synthetic_frame_id,
        }
    );
}

void KfiRateLimitedLogger::note_synthetic_artifact(
    std::uint32_t output_id,
    std::uint64_t frame_tap_count,
    const SyntheticFrameArtifact& artifact
) noexcept {
    const bool state_changed = !has_last_logged_synthetic_artifact_frame_tap_count_ ||
        artifact.generated != last_logged_synthetic_generated_ ||
        artifact.dropped != last_logged_synthetic_dropped_;
    if (!state_changed && has_last_logged_synthetic_artifact_frame_tap_count_ &&
        frame_tap_count <
            (last_logged_synthetic_artifact_frame_tap_count_ + interval_frames_)) {
        return;
    }

    last_logged_synthetic_artifact_frame_tap_count_ = frame_tap_count;
    has_last_logged_synthetic_artifact_frame_tap_count_ = true;
    last_logged_synthetic_generated_ = artifact.generated;
    last_logged_synthetic_dropped_ = artifact.dropped;

    append(
        LogEvent {
            .kind = LogEventKind::SyntheticArtifact,
            .output_id = output_id,
            .sequence = 0,
            .frame_tap_count = frame_tap_count,
            .present_feedback_count = 0,
            .state = OutputState::Bypass,
            .bypass_reason = BypassReason::None,
            .cadence_status = CadenceStatus::Unknown,
            .governor_mode = GovernorMode::Bypass,
            .scheduler_mode = SchedulerMode::PassthroughOnly,
            .classifier_allows_interpolation = false,
            .protected_content = false,
            .present_success = true,
            .dropped_synthetic = false,
            .synthetic_armed = false,
            .synthetic_should_drop = artifact.dropped,
            .synthetic_generated = artifact.generated,
            .synthetic_placeholder_only = artifact.placeholder_only,
            .synthetic_target_timestamp_ns = artifact.target_present_timestamp_ns,
            .synthetic_deadline_timestamp_ns = 0,
            .expected_frame_id = artifact.source_frame_id,
            .actual_frame_id = artifact.synthetic_frame_id,
        }
    );
}

std::vector<std::string> KfiRateLimitedLogger::snapshot_messages() const {
    std::vector<std::string> rendered;
    rendered.reserve(count_);

    for (std::size_t offset = 0; offset < count_; ++offset) {
        const auto index = (next_index_ + capacity() - count_ + offset) % capacity();
        rendered.push_back(render_event(events_[index]));
    }

    return rendered;
}

void KfiRateLimitedLogger::append(LogEvent event) noexcept {
    events_[next_index_] = event;
    next_index_ = (next_index_ + 1) % capacity();
    if (count_ < capacity()) {
        ++count_;
    }
}

std::string KfiRateLimitedLogger::render_event(const LogEvent& event) {
    std::ostringstream stream;

    switch (event.kind) {
    case LogEventKind::StateTransition:
        stream << "output=" << event.output_id << " transition=" << event.sequence
               << " state=" << to_string(event.state)
               << " bypass=" << to_string(event.bypass_reason)
               << " cadence=" << to_string(event.cadence_status)
               << " governor=" << to_string(event.governor_mode)
               << " scheduler=" << to_string(event.scheduler_mode)
               << " classifier=" << to_bool_string(event.classifier_allows_interpolation)
               << " protected=" << to_bool_string(event.protected_content);
        break;
    case LogEventKind::PresentFeedbackIssue:
        stream << "output=" << event.output_id
               << " present-success=" << to_bool_string(event.present_success)
               << " dropped-synthetic=" << to_bool_string(event.dropped_synthetic)
               << " present-feedback-count=" << event.present_feedback_count;
        break;
    case LogEventKind::PresentFeedbackMismatch:
        stream << "output=" << event.output_id
               << " present-feedback-mismatch=yes"
               << " expected-frame-id=" << event.expected_frame_id
               << " actual-frame-id=" << event.actual_frame_id
               << " present-feedback-count=" << event.present_feedback_count;
        break;
    case LogEventKind::SyntheticPlan:
        stream << "output=" << event.output_id
               << " synthetic-armed=" << to_bool_string(event.synthetic_armed)
               << " synthetic-drop=" << to_bool_string(event.synthetic_should_drop)
               << " source-frame-id=" << event.expected_frame_id
               << " synthetic-frame-id=" << event.actual_frame_id
               << " synthetic-target-ns=" << event.synthetic_target_timestamp_ns
               << " synthetic-deadline-ns=" << event.synthetic_deadline_timestamp_ns;
        break;
    case LogEventKind::SyntheticArtifact:
        stream << "output=" << event.output_id
               << " synthetic-generated=" << to_bool_string(event.synthetic_generated)
               << " synthetic-drop=" << to_bool_string(event.synthetic_should_drop)
               << " synthetic-placeholder=" << to_bool_string(event.synthetic_placeholder_only)
               << " source-frame-id=" << event.expected_frame_id
               << " synthetic-frame-id=" << event.actual_frame_id
               << " synthetic-target-ns=" << event.synthetic_target_timestamp_ns;
        break;
    }

    return stream.str();
}

std::size_t KfiRateLimitedLogger::capacity() const noexcept {
    return max_messages_ < events_.size() ? max_messages_ : events_.size();
}

}  // namespace fluxma
