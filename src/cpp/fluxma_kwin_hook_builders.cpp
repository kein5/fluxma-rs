#include "fluxma_kwin_hook_builders.h"

namespace fluxma {

namespace {

constexpr std::uint32_t raw(KwinFrameInputField field) noexcept {
    return static_cast<std::uint32_t>(field);
}

constexpr std::uint32_t raw(KwinPresentInputField field) noexcept {
    return static_cast<std::uint32_t>(field);
}

constexpr KwinFrameInputField frame_mask(std::uint32_t value) noexcept {
    return static_cast<KwinFrameInputField>(value);
}

constexpr KwinPresentInputField present_mask(std::uint32_t value) noexcept {
    return static_cast<KwinPresentInputField>(value);
}

constexpr bool has(std::uint32_t mask, std::uint32_t flag) noexcept {
    return (mask & flag) == flag;
}

}  // namespace

KwinFrameInputField KfiKwinFrameBuilder::missing_required_fields(
    const KwinCompositorFrameInputs& inputs
) noexcept {
    std::uint32_t missing = raw(KwinFrameInputField::None);
    if (inputs.frame_id == 0) {
        missing |= raw(KwinFrameInputField::FrameId);
    }
    if (inputs.timestamp_ns == 0) {
        missing |= raw(KwinFrameInputField::Timestamp);
    }
    if (inputs.width == 0) {
        missing |= raw(KwinFrameInputField::Width);
    }
    if (inputs.height == 0) {
        missing |= raw(KwinFrameInputField::Height);
    }
    if (inputs.gpu_handle.handle_id == 0) {
        missing |= raw(KwinFrameInputField::GpuHandle);
    }
    return frame_mask(missing);
}

bool KfiKwinFrameBuilder::is_complete(const KwinCompositorFrameInputs& inputs) noexcept {
    return missing_required_fields(inputs) == KwinFrameInputField::None;
}

KwinFrameHookContext KfiKwinFrameBuilder::compositor_output_frame_ready() noexcept {
    return KwinFrameHookContext {
        .hook_point = KwinFrameHookPoint::CompositorOutputFrameReady,
        .cursor_composited_in_frame = true,
        .overlay_promoted = false,
    };
}

KwinFrameHookContext KfiKwinFrameBuilder::backend_present_handoff(
    bool cursor_composited_in_frame,
    bool overlay_promoted
) noexcept {
    return KwinFrameHookContext {
        .hook_point = KwinFrameHookPoint::BackendPresentHandoff,
        .cursor_composited_in_frame = cursor_composited_in_frame,
        .overlay_promoted = overlay_promoted,
    };
}

FinalComposedFrameMetadata KfiKwinFrameBuilder::compositor_metadata(
    std::uint64_t frame_id,
    std::uint64_t timestamp_ns,
    std::uint64_t target_presentation_timestamp_ns,
    std::uint64_t predicted_render_time_ns,
    ContentType content_type
) noexcept {
    return FinalComposedFrameMetadata {
        .frame_id = frame_id,
        .timestamp_ns = timestamp_ns,
        .target_presentation_timestamp_ns = target_presentation_timestamp_ns,
        .predicted_render_time_ns = predicted_render_time_ns,
        .content_type = content_type,
    };
}

FinalComposedFramePayload KfiKwinFrameBuilder::payload(
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
) noexcept {
    return FinalComposedFramePayload {
        .width = width,
        .height = height,
        .pixel_format = pixel_format,
        .color_space = color_space,
        .protected_content = protected_content,
        .damage_ratio = damage_ratio,
        .cursor_visible = cursor_visible,
        .cursor_x = cursor_x,
        .cursor_y = cursor_y,
        .cursor_velocity_x = cursor_velocity_x,
        .cursor_velocity_y = cursor_velocity_y,
        .gpu_handle = gpu_handle,
    };
}

KwinFrameHookContext KfiKwinFrameBuilder::compositor_context(
    const KwinCompositorFrameInputs& inputs
) noexcept {
    return KwinFrameHookContext {
        .hook_point = KwinFrameHookPoint::CompositorOutputFrameReady,
        .cursor_composited_in_frame = inputs.cursor_composited_in_frame,
        .overlay_promoted = inputs.overlay_promoted,
    };
}

FinalComposedFrameEvent KfiKwinFrameBuilder::compositor_event(
    const KwinCompositorFrameInputs& inputs
) noexcept {
    if (!is_complete(inputs)) {
        return {};
    }

    return FinalComposedFrameEvent {
        .output_id = inputs.output_id,
        .metadata =
            compositor_metadata(
                inputs.frame_id,
                inputs.timestamp_ns,
                inputs.target_presentation_timestamp_ns,
                inputs.predicted_render_time_ns,
                inputs.content_type
            ),
        .payload =
            payload(
                inputs.width,
                inputs.height,
                inputs.pixel_format,
                inputs.color_space,
                inputs.protected_content,
                inputs.damage_ratio,
                inputs.cursor_visible,
                inputs.cursor_x,
                inputs.cursor_y,
                inputs.cursor_velocity_x,
                inputs.cursor_velocity_y,
                inputs.gpu_handle
            ),
    };
}

KwinResolvedFrameHook KfiKwinFrameBuilder::compositor_bundle(
    const KwinCompositorFrameInputs& inputs
) noexcept {
    return KwinResolvedFrameHook {
        .context = compositor_context(inputs),
        .event = compositor_event(inputs),
    };
}

KwinFrameFieldSources KfiKwinFrameBuilder::compositor_field_sources() noexcept {
    return KwinFrameFieldSources {
        .frame_id = KwinFrameFieldSource::CompositorHook,
        .timestamp_ns = KwinFrameFieldSource::CompositorHook,
        .target_presentation_timestamp_ns = KwinFrameFieldSource::OutputFrame,
        .predicted_render_time_ns = KwinFrameFieldSource::OutputFrame,
        .content_type = KwinFrameFieldSource::CompositorHook,
        .width = KwinFrameFieldSource::OutputFrame,
        .height = KwinFrameFieldSource::OutputFrame,
        .pixel_format = KwinFrameFieldSource::OutputFrame,
        .color_space = KwinFrameFieldSource::OutputFrame,
        .damage_ratio = KwinFrameFieldSource::CompositorHook,
        .cursor_state = KwinFrameFieldSource::CursorSceneState,
        .gpu_handle = KwinFrameFieldSource::BackendPresentPath,
    };
}

KwinFrameFieldSources KfiKwinFrameBuilder::backend_present_handoff_field_sources() noexcept {
    return KwinFrameFieldSources {
        .frame_id = KwinFrameFieldSource::BackendPresentPath,
        .timestamp_ns = KwinFrameFieldSource::CompositorHook,
        .target_presentation_timestamp_ns = KwinFrameFieldSource::OutputFrame,
        .predicted_render_time_ns = KwinFrameFieldSource::OutputFrame,
        .content_type = KwinFrameFieldSource::CompositorHook,
        .width = KwinFrameFieldSource::BackendPresentPath,
        .height = KwinFrameFieldSource::BackendPresentPath,
        .pixel_format = KwinFrameFieldSource::BackendPresentPath,
        .color_space = KwinFrameFieldSource::BackendPresentPath,
        .damage_ratio = KwinFrameFieldSource::CompositorHook,
        .cursor_state = KwinFrameFieldSource::CursorSceneState,
        .gpu_handle = KwinFrameFieldSource::BackendPresentPath,
    };
}

KwinPresentInputField KfiKwinPresentBuilder::missing_required_fields(
    const KwinPresentFeedbackInputs& inputs
) noexcept {
    std::uint32_t missing = raw(KwinPresentInputField::None);
    if (inputs.frame_id == 0) {
        missing |= raw(KwinPresentInputField::FrameId);
    }
    if (inputs.presented_timestamp_ns == 0) {
        missing |= raw(KwinPresentInputField::PresentedTimestamp);
    }
    if (inputs.refresh_interval_ns == 0) {
        missing |= raw(KwinPresentInputField::RefreshInterval);
    }
    return present_mask(missing);
}

bool KfiKwinPresentBuilder::is_complete(const KwinPresentFeedbackInputs& inputs) noexcept {
    return missing_required_fields(inputs) == KwinPresentInputField::None;
}

KwinPresentHookContext KfiKwinPresentBuilder::output_frame_presented() noexcept {
    return KwinPresentHookContext {.hook_point = KwinPresentHookPoint::OutputFramePresented};
}

KwinPresentHookContext KfiKwinPresentBuilder::render_loop_frame_presented() noexcept {
    return KwinPresentHookContext {.hook_point = KwinPresentHookPoint::RenderLoopFramePresented};
}

PresentCompletedMetadata KfiKwinPresentBuilder::metadata(
    std::uint64_t frame_id,
    std::uint64_t presented_timestamp_ns,
    std::uint64_t refresh_interval_ns,
    PresentationMode presentation_mode
) noexcept {
    return PresentCompletedMetadata {
        .frame_id = frame_id,
        .presented_timestamp_ns = presented_timestamp_ns,
        .refresh_interval_ns = refresh_interval_ns,
        .presentation_mode = presentation_mode,
    };
}

PresentCompletedStatus KfiKwinPresentBuilder::status(
    bool present_success,
    bool dropped_synthetic
) noexcept {
    return PresentCompletedStatus {
        .present_success = present_success,
        .dropped_synthetic = dropped_synthetic,
    };
}

KwinPresentHookContext KfiKwinPresentBuilder::output_frame_presented_context() noexcept {
    return KwinPresentHookContext {.hook_point = KwinPresentHookPoint::OutputFramePresented};
}

KwinPresentHookContext KfiKwinPresentBuilder::render_loop_presented_context() noexcept {
    return KwinPresentHookContext {.hook_point = KwinPresentHookPoint::RenderLoopFramePresented};
}

PresentCompletedEvent KfiKwinPresentBuilder::output_frame_presented_event(
    const KwinPresentFeedbackInputs& inputs
) noexcept {
    if (!is_complete(inputs)) {
        return {};
    }

    return PresentCompletedEvent {
        .output_id = inputs.output_id,
        .frame_id = inputs.frame_id,
        .presented_timestamp_ns = inputs.presented_timestamp_ns,
        .refresh_interval_ns = inputs.refresh_interval_ns,
        .presentation_mode = inputs.presentation_mode,
        .present_success = inputs.present_success,
        .dropped_synthetic = inputs.dropped_synthetic,
    };
}

PresentCompletedEvent KfiKwinPresentBuilder::render_loop_presented_event(
    const KwinPresentFeedbackInputs& inputs
) noexcept {
    return output_frame_presented_event(inputs);
}

KwinResolvedPresentHook KfiKwinPresentBuilder::output_frame_presented_bundle(
    const KwinPresentFeedbackInputs& inputs
) noexcept {
    return KwinResolvedPresentHook {
        .context = output_frame_presented_context(),
        .event = output_frame_presented_event(inputs),
    };
}

KwinResolvedPresentHook KfiKwinPresentBuilder::render_loop_presented_bundle(
    const KwinPresentFeedbackInputs& inputs
) noexcept {
    return KwinResolvedPresentHook {
        .context = render_loop_presented_context(),
        .event = render_loop_presented_event(inputs),
    };
}

KwinPresentFieldSources KfiKwinPresentBuilder::output_frame_presented_field_sources() noexcept {
    return KwinPresentFieldSources {
        .frame_id = KwinPresentFieldSource::OutputFramePresented,
        .presented_timestamp_ns = KwinPresentFieldSource::OutputFramePresented,
        .refresh_interval_ns = KwinPresentFieldSource::OutputFramePresented,
        .presentation_mode = KwinPresentFieldSource::OutputFramePresented,
        .present_success = KwinPresentFieldSource::OutputFramePresented,
        .dropped_synthetic = KwinPresentFieldSource::OutputFramePresented,
    };
}

KwinPresentFieldSources KfiKwinPresentBuilder::render_loop_presented_field_sources() noexcept {
    return KwinPresentFieldSources {
        .frame_id = KwinPresentFieldSource::RenderLoopPresented,
        .presented_timestamp_ns = KwinPresentFieldSource::RenderLoopPresented,
        .refresh_interval_ns = KwinPresentFieldSource::RenderLoopPresented,
        .presentation_mode = KwinPresentFieldSource::RenderLoopPresented,
        .present_success = KwinPresentFieldSource::RenderLoopPresented,
        .dropped_synthetic = KwinPresentFieldSource::RenderLoopPresented,
    };
}

bool has_flag(KwinFrameInputField mask, KwinFrameInputField flag) noexcept {
    return has(raw(mask), raw(flag));
}

bool has_flag(KwinPresentInputField mask, KwinPresentInputField flag) noexcept {
    return has(raw(mask), raw(flag));
}

std::array<std::string_view, 5> describe(KwinFrameInputField mask) noexcept {
    return {
        has_flag(mask, KwinFrameInputField::FrameId) ? "frame-id" : "",
        has_flag(mask, KwinFrameInputField::Timestamp) ? "timestamp-ns" : "",
        has_flag(mask, KwinFrameInputField::Width) ? "width" : "",
        has_flag(mask, KwinFrameInputField::Height) ? "height" : "",
        has_flag(mask, KwinFrameInputField::GpuHandle) ? "gpu-handle" : "",
    };
}

std::array<std::string_view, 3> describe(KwinPresentInputField mask) noexcept {
    return {
        has_flag(mask, KwinPresentInputField::FrameId) ? "frame-id" : "",
        has_flag(mask, KwinPresentInputField::PresentedTimestamp) ? "presented-timestamp-ns" : "",
        has_flag(mask, KwinPresentInputField::RefreshInterval) ? "refresh-interval-ns" : "",
    };
}

}  // namespace fluxma
