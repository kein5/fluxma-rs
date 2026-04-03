#pragma once

#include "fluxma_kwin_hook_adapter.h"

namespace fluxma {

struct KwinCompositorFrameInputs {
    std::uint32_t output_id = 0;
    std::uint64_t frame_id = 0;
    std::uint64_t timestamp_ns = 0;
    std::uint64_t target_presentation_timestamp_ns = 0;
    std::uint64_t predicted_render_time_ns = 0;
    ContentType content_type = ContentType::None;
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
    bool cursor_composited_in_frame = true;
    bool overlay_promoted = false;
};

struct KwinPresentFeedbackInputs {
    std::uint32_t output_id = 0;
    std::uint64_t frame_id = 0;
    std::uint64_t presented_timestamp_ns = 0;
    std::uint64_t refresh_interval_ns = 0;
    PresentationMode presentation_mode = PresentationMode::VSync;
    bool present_success = false;
    bool dropped_synthetic = false;
};

struct KwinResolvedFrameHook {
    KwinFrameHookContext context {};
    FinalComposedFrameEvent event {};
};

struct KwinResolvedPresentHook {
    KwinPresentHookContext context {};
    PresentCompletedEvent event {};
};

enum class KwinFrameInputField : std::uint32_t {
    None = 0,
    FrameId = 1u << 0,
    Timestamp = 1u << 1,
    Width = 1u << 2,
    Height = 1u << 3,
    GpuHandle = 1u << 4,
};

enum class KwinPresentInputField : std::uint32_t {
    None = 0,
    FrameId = 1u << 0,
    PresentedTimestamp = 1u << 1,
    RefreshInterval = 1u << 2,
};

enum class KwinFrameFieldSource : std::uint8_t {
    Unknown = 0,
    CompositorHook = 1,
    OutputFrame = 2,
    BackendPresentPath = 3,
    CursorSceneState = 4,
};

enum class KwinPresentFieldSource : std::uint8_t {
    Unknown = 0,
    OutputFramePresented = 1,
    RenderLoopPresented = 2,
    BackendPageFlip = 3,
};

struct KwinFrameFieldSources {
    KwinFrameFieldSource frame_id = KwinFrameFieldSource::Unknown;
    KwinFrameFieldSource timestamp_ns = KwinFrameFieldSource::Unknown;
    KwinFrameFieldSource target_presentation_timestamp_ns = KwinFrameFieldSource::Unknown;
    KwinFrameFieldSource predicted_render_time_ns = KwinFrameFieldSource::Unknown;
    KwinFrameFieldSource content_type = KwinFrameFieldSource::Unknown;
    KwinFrameFieldSource width = KwinFrameFieldSource::Unknown;
    KwinFrameFieldSource height = KwinFrameFieldSource::Unknown;
    KwinFrameFieldSource pixel_format = KwinFrameFieldSource::Unknown;
    KwinFrameFieldSource color_space = KwinFrameFieldSource::Unknown;
    KwinFrameFieldSource damage_ratio = KwinFrameFieldSource::Unknown;
    KwinFrameFieldSource cursor_state = KwinFrameFieldSource::Unknown;
    KwinFrameFieldSource gpu_handle = KwinFrameFieldSource::Unknown;
};

struct KwinPresentFieldSources {
    KwinPresentFieldSource frame_id = KwinPresentFieldSource::Unknown;
    KwinPresentFieldSource presented_timestamp_ns = KwinPresentFieldSource::Unknown;
    KwinPresentFieldSource refresh_interval_ns = KwinPresentFieldSource::Unknown;
    KwinPresentFieldSource presentation_mode = KwinPresentFieldSource::Unknown;
    KwinPresentFieldSource present_success = KwinPresentFieldSource::Unknown;
    KwinPresentFieldSource dropped_synthetic = KwinPresentFieldSource::Unknown;
};

class KfiKwinFrameBuilder {
  public:
    [[nodiscard]] static KwinFrameInputField missing_required_fields(
        const KwinCompositorFrameInputs& inputs
    ) noexcept;
    [[nodiscard]] static bool is_complete(const KwinCompositorFrameInputs& inputs) noexcept;

    // Use when the hook is placed in Compositor::composite(RenderLoop *) after OutputFrame
    // construction and before BackendOutput::present(..., frame).
    [[nodiscard]] static KwinFrameHookContext compositor_output_frame_ready() noexcept;
    // Use when the hook is placed at backend present handoff where cursor/overlay promotion
    // may already be known.
    [[nodiscard]] static KwinFrameHookContext backend_present_handoff(
        bool cursor_composited_in_frame,
        bool overlay_promoted
    ) noexcept;

    [[nodiscard]] static FinalComposedFrameMetadata compositor_metadata(
        std::uint64_t frame_id,
        std::uint64_t timestamp_ns,
        std::uint64_t target_presentation_timestamp_ns,
        std::uint64_t predicted_render_time_ns,
        ContentType content_type
    ) noexcept;

    [[nodiscard]] static FinalComposedFramePayload payload(
        std::uint32_t width,
        std::uint32_t height,
        std::uint32_t pixel_format,
        std::uint32_t color_space,
        bool protected_content,
        double damage_ratio,
        bool cursor_visible,
        double cursor_x,
        double cursor_y,
        double cursor_velocity_x,
        double cursor_velocity_y,
        GpuFrameHandle gpu_handle
    ) noexcept;

    [[nodiscard]] static KwinFrameHookContext compositor_context(
        const KwinCompositorFrameInputs& inputs
    ) noexcept;
    [[nodiscard]] static FinalComposedFrameEvent compositor_event(
        const KwinCompositorFrameInputs& inputs
    ) noexcept;
    [[nodiscard]] static KwinResolvedFrameHook compositor_bundle(
        const KwinCompositorFrameInputs& inputs
    ) noexcept;
    [[nodiscard]] static KwinFrameFieldSources compositor_field_sources() noexcept;
    [[nodiscard]] static KwinFrameFieldSources backend_present_handoff_field_sources() noexcept;
};

class KfiKwinPresentBuilder {
  public:
    [[nodiscard]] static KwinPresentInputField missing_required_fields(
        const KwinPresentFeedbackInputs& inputs
    ) noexcept;
    [[nodiscard]] static bool is_complete(const KwinPresentFeedbackInputs& inputs) noexcept;

    // Use when the hook is placed near OutputFrame::presented(...).
    [[nodiscard]] static KwinPresentHookContext output_frame_presented() noexcept;
    // Use when the hook is placed near RenderLoop::framePresented(...).
    [[nodiscard]] static KwinPresentHookContext render_loop_frame_presented() noexcept;

    [[nodiscard]] static PresentCompletedMetadata metadata(
        std::uint64_t frame_id,
        std::uint64_t presented_timestamp_ns,
        std::uint64_t refresh_interval_ns,
        PresentationMode presentation_mode
    ) noexcept;

    [[nodiscard]] static PresentCompletedStatus status(
        bool present_success,
        bool dropped_synthetic
    ) noexcept;

    [[nodiscard]] static KwinPresentHookContext output_frame_presented_context() noexcept;
    [[nodiscard]] static KwinPresentHookContext render_loop_presented_context() noexcept;
    [[nodiscard]] static PresentCompletedEvent output_frame_presented_event(
        const KwinPresentFeedbackInputs& inputs
    ) noexcept;

    [[nodiscard]] static PresentCompletedEvent render_loop_presented_event(
        const KwinPresentFeedbackInputs& inputs
    ) noexcept;
    [[nodiscard]] static KwinResolvedPresentHook output_frame_presented_bundle(
        const KwinPresentFeedbackInputs& inputs
    ) noexcept;
    [[nodiscard]] static KwinResolvedPresentHook render_loop_presented_bundle(
        const KwinPresentFeedbackInputs& inputs
    ) noexcept;
    [[nodiscard]] static KwinPresentFieldSources output_frame_presented_field_sources() noexcept;
    [[nodiscard]] static KwinPresentFieldSources render_loop_presented_field_sources() noexcept;
};

}  // namespace fluxma
