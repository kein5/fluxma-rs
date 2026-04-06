#include "fluxma_output_controller.h"

namespace fluxma {

RustOutputCore::RustOutputCore(ModuleConfig config, bool force_unavailable_for_tests) {
    if (force_unavailable_for_tests) {
        core_ = nullptr;
        return;
    }

    FluxmaRustConfig ffi_config {
        .enabled = static_cast<std::uint8_t>(config.enabled ? 1 : 0),
        .reserved = {0, 0, 0, 0, 0, 0, 0},
    };

    core_ = fluxma_rust_core_create(ffi_config);
}

RustOutputCore::~RustOutputCore() {
    fluxma_rust_core_destroy(core_);
}

OutputDecision RustOutputCore::evaluate(const FrameDescriptor& frame) const noexcept {
    if (core_ == nullptr) {
        return OutputDecision {
            .state = OutputState::Bypass,
            .bypass_reason = BypassReason::HookUnavailable,
            .passthrough_only = true,
        };
    }

    return OutputDecision::from_ffi(fluxma_rust_core_evaluate_frame(core_, frame.to_ffi()));
}

void RustOutputCore::note_present_feedback(const PresentFeedback& feedback) const noexcept {
    if (core_ == nullptr) {
        return;
    }

    fluxma_rust_core_note_present_feedback(core_, feedback.to_ffi());
}

MetricsSnapshot RustOutputCore::snapshot_metrics() const noexcept {
    if (core_ == nullptr) {
        return MetricsSnapshot {
            .state = OutputState::Bypass,
            .bypass_reason = BypassReason::HookUnavailable,
            .protected_content = false,
            .passthrough_only = true,
        };
    }

    return MetricsSnapshot::from_ffi(fluxma_rust_core_snapshot_metrics(core_));
}

bool RustOutputCore::is_ready() const noexcept {
    return core_ != nullptr;
}

std::uint64_t RustOutputCore::ping(std::uint64_t value) const noexcept {
    if (core_ == nullptr) {
        return value;
    }

    return fluxma_rust_core_ping(core_, value);
}

KfiOutputController::KfiOutputController(
    std::uint32_t output_id,
    ModuleConfig config,
    bool force_rust_core_unavailable_for_tests
)
    : output_id_(output_id),
      config_(config),
      rust_core_(config, force_rust_core_unavailable_for_tests),
      logger_(config.log_interval_frames, config.max_log_messages)
{
    rust_core_ready_ = rust_core_.is_ready();
    if (!rust_core_ready_) {
        runtime_.state = OutputState::Bypass;
        runtime_.bypass_reason = BypassReason::HookUnavailable;
        logger_.note_state_transition(
            output_id_,
            0,
            0,
            runtime_.state,
            runtime_.bypass_reason,
            CadenceStatus::Unknown,
            GovernorMode::Bypass,
            SchedulerMode::PassthroughOnly,
            false,
            false
        );
    }
}

std::uint32_t KfiOutputController::output_id() const noexcept {
    return output_id_;
}

OutputDecision KfiOutputController::on_frame_tapped(const FrameDescriptor& frame) noexcept {
    // TODO: Attach this controller to a real per-output KWin integration point once the
    // final post-composition hook is confirmed.
    const auto tapped = bypass_pipeline_.capture_frame(frame);
    last_decision_ = rust_core_.evaluate(tapped);
    last_submission_ = bypass_pipeline_.submit_passthrough(output_id_, tapped, last_decision_);
    remember_submission(last_submission_.frame_id);
    const auto snapshot = rust_core_.snapshot_metrics();
    maybe_log_state_change(snapshot);
    maybe_log_synthetic_plan(snapshot);
    maybe_log_synthetic_artifact(snapshot);
    maybe_log_synthetic_submission(snapshot);
    update_runtime(snapshot);
    return last_decision_;
}

PassthroughSubmission KfiOutputController::last_submission() const noexcept {
    return last_submission_;
}

OutputRuntimeSample KfiOutputController::sample_runtime(std::uint64_t now_ns) const noexcept {
    const auto snapshot = snapshot_metrics();
    const auto synthetic_plan = synthetic_scheduler_.plan_placeholder_synthetic(
        output_id_,
        last_submission_,
        last_decision_,
        snapshot,
        now_ns
    );
    const auto synthetic_artifact = fake_synth_generator_.generate(synthetic_plan);
    return OutputRuntimeSample {
        .snapshot = snapshot,
        .synthetic_plan = synthetic_plan,
        .synthetic_artifact = synthetic_artifact,
        .synthetic_submission = synthetic_present_queue_.enqueue_placeholder(synthetic_artifact),
    };
}

SyntheticFramePlan KfiOutputController::plan_synthetic_frame(std::uint64_t now_ns) const noexcept {
    return sample_runtime(now_ns).synthetic_plan;
}

SyntheticFrameArtifact KfiOutputController::generate_fake_synthetic_frame(
    std::uint64_t now_ns
) const noexcept {
    return sample_runtime(now_ns).synthetic_artifact;
}

SyntheticPresentSubmission KfiOutputController::submit_fake_synthetic_frame(
    std::uint64_t now_ns
) const noexcept {
    return sample_runtime(now_ns).synthetic_submission;
}

void KfiOutputController::on_present_feedback(const PresentFeedback& feedback) noexcept {
    rust_core_.note_present_feedback(feedback);
    const auto snapshot = rust_core_.snapshot_metrics();
    maybe_log_present_feedback_mismatch(feedback, snapshot);
    maybe_log_present_feedback(feedback, snapshot);
    maybe_log_synthetic_plan(snapshot);
    maybe_log_synthetic_artifact(snapshot);
    maybe_log_synthetic_submission(snapshot);
    update_runtime(snapshot);
}

MetricsSnapshot KfiOutputController::snapshot_metrics() const noexcept {
    return rust_core_.snapshot_metrics();
}

std::string KfiOutputController::render_hud_text() const {
    if (!config_.show_hud) {
        return {};
    }

    return render_hud_text(sample_runtime(snapshot_metrics().last_presented_timestamp_ns));
}

std::string KfiOutputController::render_hud_text(const OutputRuntimeSample& sample) const {
    if (!config_.show_hud) {
        return {};
    }

    return hud_renderer_.compose_text(
        output_id_,
        sample.snapshot,
        sample.synthetic_plan,
        sample.synthetic_artifact,
        sample.synthetic_submission
    );
}

std::vector<std::string> KfiOutputController::log_messages() const {
    return logger_.snapshot_messages();
}

void KfiOutputController::remember_submission(std::uint64_t frame_id) noexcept {
    if (frame_id == 0) {
        return;
    }

    for (auto& candidate : runtime_.recent_submitted_frame_ids) {
        if (candidate == 0) {
            candidate = frame_id;
            return;
        }
    }

    for (std::size_t index = 1; index < runtime_.recent_submitted_frame_ids.size(); ++index) {
        runtime_.recent_submitted_frame_ids[index - 1] =
            runtime_.recent_submitted_frame_ids[index];
    }
    runtime_.recent_submitted_frame_ids.back() = frame_id;
}

bool KfiOutputController::consume_matching_submission(std::uint64_t frame_id) noexcept {
    if (frame_id == 0) {
        return false;
    }

    for (auto& candidate : runtime_.recent_submitted_frame_ids) {
        if (candidate == frame_id) {
            candidate = 0;
            return true;
        }
    }

    return false;
}

void KfiOutputController::update_runtime(const MetricsSnapshot& snapshot) noexcept {
    runtime_.state = snapshot.state;
    runtime_.bypass_reason = snapshot.bypass_reason;
    runtime_.protected_content = snapshot.protected_content;
}

void KfiOutputController::maybe_log_state_change(const MetricsSnapshot& snapshot) {
    if (runtime_.state == snapshot.state &&
        runtime_.bypass_reason == snapshot.bypass_reason &&
        runtime_.protected_content == snapshot.protected_content) {
        return;
    }

    logger_.note_state_transition(
        output_id_,
        snapshot.frame_tap_count,
        snapshot.state_transition_count,
        snapshot.state,
        snapshot.bypass_reason,
        snapshot.cadence_status,
        snapshot.governor_mode,
        snapshot.scheduler_mode,
        snapshot.classifier_allows_interpolation,
        snapshot.protected_content
    );
}

void KfiOutputController::maybe_log_present_feedback(
    const PresentFeedback& feedback,
    const MetricsSnapshot& snapshot
) {
    if (feedback.present_success && !feedback.dropped_synthetic) {
        return;
    }

    logger_.note_present_feedback_issue(
        output_id_,
        snapshot.frame_tap_count,
        snapshot.present_feedback_count,
        feedback.present_success,
        feedback.dropped_synthetic
    );
}

void KfiOutputController::maybe_log_present_feedback_mismatch(
    const PresentFeedback& feedback,
    const MetricsSnapshot& snapshot
) {
    if (consume_matching_submission(feedback.frame_id)) {
        return;
    }

    logger_.note_present_feedback_mismatch(
        output_id_,
        snapshot.frame_tap_count,
        snapshot.present_feedback_count,
        last_submission_.frame_id,
        feedback.frame_id
    );
}

void KfiOutputController::maybe_log_synthetic_plan(const MetricsSnapshot& snapshot) {
    logger_.note_synthetic_plan(
        output_id_,
        snapshot.frame_tap_count,
        synthetic_scheduler_.plan_placeholder_synthetic(
            output_id_,
            last_submission_,
            last_decision_,
            snapshot,
            snapshot.last_presented_timestamp_ns
        )
    );
}

void KfiOutputController::maybe_log_synthetic_artifact(const MetricsSnapshot& snapshot) {
    logger_.note_synthetic_artifact(
        output_id_,
        snapshot.frame_tap_count,
        fake_synth_generator_.generate(
            synthetic_scheduler_.plan_placeholder_synthetic(
                output_id_,
                last_submission_,
                last_decision_,
                snapshot,
                snapshot.last_presented_timestamp_ns
            )
        )
    );
}

void KfiOutputController::maybe_log_synthetic_submission(const MetricsSnapshot& snapshot) {
    logger_.note_synthetic_submission(
        output_id_,
        snapshot.frame_tap_count,
        synthetic_present_queue_.enqueue_placeholder(
            fake_synth_generator_.generate(
                synthetic_scheduler_.plan_placeholder_synthetic(
                    output_id_,
                    last_submission_,
                    last_decision_,
                    snapshot,
                    snapshot.last_presented_timestamp_ns
                )
            )
        )
    );
}

}  // namespace fluxma
