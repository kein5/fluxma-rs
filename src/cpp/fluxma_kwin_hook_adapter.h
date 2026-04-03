#pragma once

#include "fluxma_output_policy.h"
#include "fluxma_output_controller.h"
#include "fluxma_present_feedback_tap.h"
#include "fluxma_types.h"

namespace fluxma {

enum class KwinFrameHookPoint : std::uint8_t {
    Unknown = 0,
    CompositorOutputFrameReady = 1,
    BackendPresentHandoff = 2,
};

enum class KwinPresentHookPoint : std::uint8_t {
    Unknown = 0,
    OutputFramePresented = 1,
    RenderLoopFramePresented = 2,
};

struct KwinFrameHookContext {
    KwinFrameHookPoint hook_point = KwinFrameHookPoint::Unknown;
    bool cursor_composited_in_frame = true;
    bool overlay_promoted = false;
};

struct KwinPresentHookContext {
    KwinPresentHookPoint hook_point = KwinPresentHookPoint::Unknown;
};

struct FinalComposedFrameMetadata {
    std::uint64_t frame_id = 0;
    std::uint64_t timestamp_ns = 0;
    std::uint64_t target_presentation_timestamp_ns = 0;
    std::uint64_t predicted_render_time_ns = 0;
    ContentType content_type = ContentType::None;
};

struct FinalComposedFramePayload {
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::uint32_t pixel_format = 0;
    std::uint32_t color_space = 0;
    bool protected_content = false;
    double damage_ratio = 0.0;
    bool cursor_visible = false;
    double cursor_x = 0.0;
    double cursor_y = 0.0;
    double cursor_velocity_x = 0.0;
    double cursor_velocity_y = 0.0;
    GpuFrameHandle gpu_handle {};
};

struct FinalComposedFrameEvent {
    std::uint32_t output_id = 0;
    FinalComposedFrameMetadata metadata {};
    FinalComposedFramePayload payload {};
};

struct PresentCompletedEvent {
    std::uint32_t output_id = 0;
    std::uint64_t frame_id = 0;
    std::uint64_t presented_timestamp_ns = 0;
    std::uint64_t refresh_interval_ns = 0;
    PresentationMode presentation_mode = PresentationMode::VSync;
    bool present_success = false;
    bool dropped_synthetic = false;
};

struct PresentCompletedMetadata {
    std::uint64_t frame_id = 0;
    std::uint64_t presented_timestamp_ns = 0;
    std::uint64_t refresh_interval_ns = 0;
    PresentationMode presentation_mode = PresentationMode::VSync;
};

struct PresentCompletedStatus {
    bool present_success = false;
    bool dropped_synthetic = false;
};

class KfiKwinHookAdapter {
  public:
    KfiKwinHookAdapter(std::uint32_t output_id, KfiOutputController& output_controller);

    [[nodiscard]] OutputDecision on_compositor_output_frame_ready(
        const FinalComposedFrameEvent& event
    ) noexcept;
    [[nodiscard]] OutputDecision on_compositor_output_frame_ready(
        std::uint32_t output_id,
        const FinalComposedFrameMetadata& metadata,
        const FinalComposedFramePayload& payload
    ) noexcept;
    [[nodiscard]] OutputDecision on_compositor_output_frame_ready(
        const KwinFrameHookContext& context,
        std::uint32_t output_id,
        const FinalComposedFrameMetadata& metadata,
        const FinalComposedFramePayload& payload
    ) noexcept;
    void on_render_loop_frame_presented(const PresentCompletedEvent& event) noexcept;
    void on_render_loop_frame_presented(
        std::uint32_t output_id,
        const PresentCompletedMetadata& metadata,
        const PresentCompletedStatus& status
    ) noexcept;
    void on_render_loop_frame_presented(
        const KwinPresentHookContext& context,
        std::uint32_t output_id,
        const PresentCompletedMetadata& metadata,
        const PresentCompletedStatus& status
    ) noexcept;
    void on_output_frame_presented(
        std::uint32_t output_id,
        const PresentCompletedMetadata& metadata,
        const PresentCompletedStatus& status
    ) noexcept;

    [[nodiscard]] OutputDecision on_final_composed_frame(
        const FinalComposedFrameEvent& event
    ) noexcept;
    void on_present_completed(const PresentCompletedEvent& event) noexcept;

  private:
    [[nodiscard]] static FinalComposedFrameEvent make_frame_event(
        std::uint32_t output_id,
        const FinalComposedFrameMetadata& metadata,
        const FinalComposedFramePayload& payload
    ) noexcept;
    [[nodiscard]] static PresentCompletedEvent make_present_completed_event(
        std::uint32_t output_id,
        const PresentCompletedMetadata& metadata,
        const PresentCompletedStatus& status
    ) noexcept;
    [[nodiscard]] static FrameDescriptor to_frame_descriptor(
        const FinalComposedFrameEvent& event
    ) noexcept;
    [[nodiscard]] static PresentFeedback to_present_feedback(
        const PresentCompletedEvent& event
    ) noexcept;

    std::uint32_t output_id_ = 0;
    KfiOutputPolicy output_policy_;
    KfiPresentFeedbackTap present_feedback_tap_ {};
    KfiOutputController& output_controller_;
};

}  // namespace fluxma
