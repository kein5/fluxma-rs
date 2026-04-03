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
            .protected_content = protected_content,
            .present_success = true,
            .dropped_synthetic = false,
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
            .protected_content = false,
            .present_success = present_success,
            .dropped_synthetic = dropped_synthetic,
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
            .protected_content = false,
            .present_success = true,
            .dropped_synthetic = false,
            .expected_frame_id = expected_frame_id,
            .actual_frame_id = actual_frame_id,
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
    }

    return stream.str();
}

std::size_t KfiRateLimitedLogger::capacity() const noexcept {
    return max_messages_ < events_.size() ? max_messages_ : events_.size();
}

}  // namespace fluxma
