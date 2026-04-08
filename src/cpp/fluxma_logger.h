#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "fluxma_types.h"

namespace fluxma {

enum class LogEventKind : std::uint8_t {
    StateTransition = 0,
    PresentFeedbackIssue = 1,
    PresentFeedbackMismatch = 2,
    SyntheticPlan = 3,
    SyntheticArtifact = 4,
    SyntheticSubmission = 5,
};

struct LogEvent {
    LogEventKind kind = LogEventKind::StateTransition;
    std::uint32_t output_id = 0;
    std::uint64_t sequence = 0;
    std::uint64_t frame_tap_count = 0;
    std::uint64_t present_feedback_count = 0;
    OutputState state = OutputState::Bypass;
    BypassReason bypass_reason = BypassReason::None;
    CadenceStatus cadence_status = CadenceStatus::Unknown;
    GovernorMode governor_mode = GovernorMode::Bypass;
    SchedulerMode scheduler_mode = SchedulerMode::PassthroughOnly;
    bool classifier_allows_interpolation = false;
    bool protected_content = false;
    bool present_success = true;
    bool dropped_synthetic = false;
    bool synthetic_armed = false;
    bool synthetic_should_drop = false;
    bool synthetic_generated = false;
    bool synthetic_queued = false;
    bool synthetic_placeholder_only = true;
    bool synthetic_cursor_passthrough = false;
    bool synthetic_overlay_passthrough = false;
    bool synthetic_subtitle_current_priority = false;
    std::uint64_t synthetic_target_timestamp_ns = 0;
    std::uint64_t synthetic_deadline_timestamp_ns = 0;
    std::uint64_t expected_frame_id = 0;
    std::uint64_t actual_frame_id = 0;
};

class KfiRateLimitedLogger {
  public:
    KfiRateLimitedLogger(std::uint64_t interval_frames, std::size_t max_messages);

    void note_state_transition(
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
    ) noexcept;
    void note_present_feedback_issue(
        std::uint32_t output_id,
        std::uint64_t frame_tap_count,
        std::uint64_t present_feedback_count,
        bool present_success,
        bool dropped_synthetic
    ) noexcept;
    void note_present_feedback_mismatch(
        std::uint32_t output_id,
        std::uint64_t frame_tap_count,
        std::uint64_t present_feedback_count,
        std::uint64_t expected_frame_id,
        std::uint64_t actual_frame_id
    ) noexcept;
    void note_synthetic_plan(
        std::uint32_t output_id,
        std::uint64_t frame_tap_count,
        const SyntheticFramePlan& plan
    ) noexcept;
    void note_synthetic_artifact(
        std::uint32_t output_id,
        std::uint64_t frame_tap_count,
        const SyntheticFrameArtifact& artifact
    ) noexcept;
    void note_synthetic_submission(
        std::uint32_t output_id,
        std::uint64_t frame_tap_count,
        const SyntheticPresentSubmission& submission
    ) noexcept;
    [[nodiscard]] std::vector<std::string> snapshot_messages() const;

  private:
    void append(LogEvent event) noexcept;
    [[nodiscard]] static std::string render_event(const LogEvent& event);
    [[nodiscard]] std::size_t capacity() const noexcept;

    std::uint64_t interval_frames_ = 0;
    std::size_t max_messages_ = 0;
    std::uint64_t last_logged_state_transition_frame_tap_count_ = 0;
    bool has_last_logged_state_transition_frame_tap_count_ = false;
    std::uint64_t last_logged_present_feedback_issue_count_ = 0;
    bool has_last_logged_present_feedback_issue_count_ = false;
    std::uint64_t last_logged_present_feedback_mismatch_count_ = 0;
    bool has_last_logged_present_feedback_mismatch_count_ = false;
    std::uint64_t last_logged_synthetic_plan_frame_tap_count_ = 0;
    bool has_last_logged_synthetic_plan_frame_tap_count_ = false;
    bool last_logged_synthetic_armed_ = false;
    bool last_logged_synthetic_should_drop_ = false;
    std::uint64_t last_logged_synthetic_artifact_frame_tap_count_ = 0;
    bool has_last_logged_synthetic_artifact_frame_tap_count_ = false;
    bool last_logged_synthetic_generated_ = false;
    bool last_logged_synthetic_dropped_ = false;
    std::uint64_t last_logged_synthetic_submission_frame_tap_count_ = 0;
    bool has_last_logged_synthetic_submission_frame_tap_count_ = false;
    bool last_logged_synthetic_queued_ = false;
    bool last_logged_synthetic_submission_dropped_ = false;
    std::array<LogEvent, 128> events_ {};
    std::size_t next_index_ = 0;
    std::size_t count_ = 0;
};

}  // namespace fluxma
