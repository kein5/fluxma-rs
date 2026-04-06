#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "fluxma_bypass_pipeline.h"
#include "fluxma_config.h"
#include "fluxma_hud_renderer.h"
#include "fluxma_logger.h"
#include "fluxma_synthetic_scheduler.h"
#include "fluxma_types.h"

namespace fluxma {

class RustOutputCore {
  public:
    explicit RustOutputCore(ModuleConfig config, bool force_unavailable_for_tests = false);
    ~RustOutputCore();

    RustOutputCore(const RustOutputCore&) = delete;
    RustOutputCore& operator=(const RustOutputCore&) = delete;

    [[nodiscard]] OutputDecision evaluate(const FrameDescriptor& frame) const noexcept;
    void note_present_feedback(const PresentFeedback& feedback) const noexcept;
    [[nodiscard]] MetricsSnapshot snapshot_metrics() const noexcept;
    [[nodiscard]] bool is_ready() const noexcept;
    [[nodiscard]] std::uint64_t ping(std::uint64_t value) const noexcept;

  private:
    FluxmaRustCore* core_ = nullptr;
};

class KfiOutputController {
  public:
    KfiOutputController(
        std::uint32_t output_id,
        ModuleConfig config,
        bool force_rust_core_unavailable_for_tests = false
    );

    [[nodiscard]] std::uint32_t output_id() const noexcept;
    [[nodiscard]] OutputDecision on_frame_tapped(const FrameDescriptor& frame) noexcept;
    [[nodiscard]] PassthroughSubmission last_submission() const noexcept;
    [[nodiscard]] SyntheticFramePlan plan_synthetic_frame(std::uint64_t now_ns) const noexcept;
    void on_present_feedback(const PresentFeedback& feedback) noexcept;
    [[nodiscard]] MetricsSnapshot snapshot_metrics() const noexcept;
    [[nodiscard]] std::string render_hud_text() const;
    [[nodiscard]] std::vector<std::string> log_messages() const;

  private:
    struct ControllerRuntime {
        OutputState state = OutputState::Bypass;
        BypassReason bypass_reason = BypassReason::None;
        bool protected_content = false;
        std::array<std::uint64_t, 8> recent_submitted_frame_ids {};
    };

    void remember_submission(std::uint64_t frame_id) noexcept;
    [[nodiscard]] bool consume_matching_submission(std::uint64_t frame_id) noexcept;
    void update_runtime(const MetricsSnapshot& snapshot) noexcept;
    void maybe_log_state_change(const MetricsSnapshot& snapshot);
    void maybe_log_present_feedback(
        const PresentFeedback& feedback,
        const MetricsSnapshot& snapshot
    );
    void maybe_log_present_feedback_mismatch(
        const PresentFeedback& feedback,
        const MetricsSnapshot& snapshot
    );
    void maybe_log_synthetic_plan(const MetricsSnapshot& snapshot);

    std::uint32_t output_id_ = 0;
    ModuleConfig config_ {};
    KfiBypassPipeline bypass_pipeline_ {};
    KfiHudRenderer hud_renderer_ {};
    KfiSyntheticScheduler synthetic_scheduler_ {};
    RustOutputCore rust_core_;
    bool rust_core_ready_ = false;
    OutputDecision last_decision_ {};
    PassthroughSubmission last_submission_ {};
    ControllerRuntime runtime_ {};
    KfiRateLimitedLogger logger_;
};

}  // namespace fluxma
