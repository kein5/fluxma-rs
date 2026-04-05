#include "fluxma_kwin_hook_adapter.h"
#include "fluxma_kwin_hook_builders.h"
#include "fluxma_kwin_hook_candidates.h"

namespace fluxma {

KfiKwinHookAdapter::KfiKwinHookAdapter(
    std::uint32_t output_id,
    KfiOutputController& output_controller
)
    : output_id_(output_id),
      output_policy_(output_id),
      output_controller_(output_controller) {}

KwinFrameHookCandidatePlan KfiKwinHookAdapter::preferred_frame_candidate() noexcept {
    return KfiKwinHookCandidates::compositor_output_frame_ready();
}

KwinPresentHookCandidatePlan KfiKwinHookAdapter::preferred_present_candidate() noexcept {
    return KfiKwinHookCandidates::output_frame_presented();
}

KwinFrameHookReadiness KfiKwinHookAdapter::assess_frame_candidate(
    const KwinCompositorFrameInputs& inputs
) noexcept {
    return KfiKwinHookCandidates::assess(preferred_frame_candidate(), inputs);
}

KwinPresentHookReadiness KfiKwinHookAdapter::assess_present_candidate(
    const KwinPresentFeedbackInputs& inputs
) noexcept {
    return KfiKwinHookCandidates::assess(preferred_present_candidate(), inputs);
}

std::array<std::string_view, 5> KfiKwinHookAdapter::preferred_frame_checklist() noexcept {
    return describe_checklist(preferred_frame_candidate());
}

std::array<std::string_view, 3> KfiKwinHookAdapter::preferred_present_checklist() noexcept {
    return describe_checklist(preferred_present_candidate());
}

std::string KfiKwinHookAdapter::summarize_frame_candidate(
    const KwinCompositorFrameInputs& inputs
) {
    return summarize(assess_frame_candidate(inputs));
}

std::string KfiKwinHookAdapter::summarize_present_candidate(
    const KwinPresentFeedbackInputs& inputs
) {
    return summarize(assess_present_candidate(inputs));
}

OutputDecision KfiKwinHookAdapter::on_final_composed_frame(
    const FinalComposedFrameEvent& event
) noexcept {
    return on_compositor_output_frame_ready(event);
}

void KfiKwinHookAdapter::on_present_completed(const PresentCompletedEvent& event) noexcept {
    on_render_loop_frame_presented(event);
}

OutputDecision KfiKwinHookAdapter::on_compositor_output_frame_ready(
    const FinalComposedFrameEvent& event
) noexcept {
    // TODO: Replace this neutral event adapter with the confirmed KWin 6 internal/native hook.
    // The real implementation must annotate any private/internal hook usage explicitly.
    // Current KWin 6 source candidates are tracked in KfiKwinHookCandidates so the code path and
    // docs stay aligned:
    // 1. WaylandCompositor::composite(RenderLoop *) in src/compositor_wayland.cpp
    // 2. BackendOutput::present(..., frame) and backend-specific present() handoff
    // 3. OutputFrame::presented(...) -> RenderLoopPrivate::notifyFrameCompleted(...) feedback path
    // TODO: When wiring the real hook, log summarize_frame_candidate(inputs) once per
    // candidate revision so missing vs unresolved fields are visible during bring-up.
    (void)preferred_frame_candidate();
    const auto policy_decision = output_policy_.classify_frame_event(event);
    if (policy_decision.bypass_reason == BypassReason::UnsupportedOutput ||
        policy_decision.bypass_reason == BypassReason::ProtectedContent) {
        if (policy_decision.bypass_reason == BypassReason::ProtectedContent &&
            output_policy_.accepts_output(event.output_id)) {
            return output_controller_.on_frame_tapped(to_frame_descriptor(event));
        }

        return policy_decision;
    }

    return output_controller_.on_frame_tapped(to_frame_descriptor(event));
}

OutputDecision KfiKwinHookAdapter::on_compositor_output_frame_ready(
    std::uint32_t output_id,
    const FinalComposedFrameMetadata& metadata,
    const FinalComposedFramePayload& payload
) noexcept {
    return on_compositor_output_frame_ready(
        KwinFrameHookContext {
            .hook_point = KwinFrameHookPoint::CompositorOutputFrameReady,
            .cursor_composited_in_frame = true,
            .overlay_promoted = false,
        },
        output_id,
        metadata,
        payload
    );
}

OutputDecision KfiKwinHookAdapter::on_compositor_output_frame_ready(
    const KwinFrameHookContext& context,
    std::uint32_t output_id,
    const FinalComposedFrameMetadata& metadata,
    const FinalComposedFramePayload& payload
) noexcept {
    const auto hook_decision = output_policy_.classify_frame_hook_context(context);
    if (hook_decision.bypass_reason == BypassReason::HookUnavailable) {
        return hook_decision;
    }

    // TODO: When the real KWin hook is connected, extend this with boundary-specific checks
    // for scanout promotion, cursor-only updates, and backend-specific layer layouts.
    return on_compositor_output_frame_ready(make_frame_event(output_id, metadata, payload));
}

OutputDecision KfiKwinHookAdapter::on_compositor_output_frame_ready(
    const KwinResolvedFrameHook& hook
) noexcept {
    return on_compositor_output_frame_ready(
        hook.context,
        hook.event.output_id,
        hook.event.metadata,
        hook.event.payload
    );
}

void KfiKwinHookAdapter::on_render_loop_frame_presented(
    const PresentCompletedEvent& event
) noexcept {
    // TODO: Wire this to the actual KWin present feedback callback once the hook is confirmed.
    // Prefer KfiKwinHookCandidates::output_frame_presented(), then fall back to
    // KfiKwinHookCandidates::render_loop_frame_presented() if backend-specific metadata requires it.
    // TODO: When wiring the real hook, log summarize_present_candidate(inputs) once per
    // candidate revision so missing vs unresolved feedback fields are visible during bring-up.
    (void)preferred_present_candidate();
    if (!output_policy_.accepts_output(event.output_id)) {
        return;
    }
    if (!output_policy_.supports_present_event(event)) {
        return;
    }

    output_controller_.on_present_feedback(
        present_feedback_tap_.capture(to_present_feedback(event))
    );
}

void KfiKwinHookAdapter::on_render_loop_frame_presented(
    std::uint32_t output_id,
    const PresentCompletedMetadata& metadata,
    const PresentCompletedStatus& status
) noexcept {
    on_render_loop_frame_presented(
        KwinPresentHookContext {.hook_point = KwinPresentHookPoint::RenderLoopFramePresented},
        output_id,
        metadata,
        status
    );
}

void KfiKwinHookAdapter::on_render_loop_frame_presented(
    const KwinPresentHookContext& context,
    std::uint32_t output_id,
    const PresentCompletedMetadata& metadata,
    const PresentCompletedStatus& status
) noexcept {
    if (!output_policy_.supports_present_hook_context(context)) {
        return;
    }

    // TODO: Distinguish RenderLoop feedback from OutputFrame feedback once the real hook lands.
    on_render_loop_frame_presented(make_present_completed_event(output_id, metadata, status));
}

void KfiKwinHookAdapter::on_render_loop_frame_presented(
    const KwinResolvedPresentHook& hook
) noexcept {
    on_render_loop_frame_presented(
        hook.context,
        hook.event.output_id,
        PresentCompletedMetadata {
            .frame_id = hook.event.frame_id,
            .presented_timestamp_ns = hook.event.presented_timestamp_ns,
            .refresh_interval_ns = hook.event.refresh_interval_ns,
            .presentation_mode = hook.event.presentation_mode,
        },
        PresentCompletedStatus {
            .present_success = hook.event.present_success,
            .dropped_synthetic = hook.event.dropped_synthetic,
        }
    );
}

void KfiKwinHookAdapter::on_output_frame_presented(
    std::uint32_t output_id,
    const PresentCompletedMetadata& metadata,
    const PresentCompletedStatus& status
) noexcept {
    on_render_loop_frame_presented(
        KwinPresentHookContext {.hook_point = KwinPresentHookPoint::OutputFramePresented},
        output_id,
        metadata,
        status
    );
}

void KfiKwinHookAdapter::on_output_frame_presented(const KwinResolvedPresentHook& hook) noexcept {
    on_render_loop_frame_presented(hook);
}

FinalComposedFrameEvent KfiKwinHookAdapter::make_frame_event(
    std::uint32_t output_id,
    const FinalComposedFrameMetadata& metadata,
    const FinalComposedFramePayload& payload
) noexcept {
    return FinalComposedFrameEvent {
        .output_id = output_id,
        .metadata = metadata,
        .payload = payload,
    };
}

PresentCompletedEvent KfiKwinHookAdapter::make_present_completed_event(
    std::uint32_t output_id,
    const PresentCompletedMetadata& metadata,
    const PresentCompletedStatus& status
) noexcept {
    return PresentCompletedEvent {
        .output_id = output_id,
        .frame_id = metadata.frame_id,
        .presented_timestamp_ns = metadata.presented_timestamp_ns,
        .refresh_interval_ns = metadata.refresh_interval_ns,
        .presentation_mode = metadata.presentation_mode,
        .present_success = status.present_success,
        .dropped_synthetic = status.dropped_synthetic,
    };
}

FrameDescriptor KfiKwinHookAdapter::to_frame_descriptor(
    const FinalComposedFrameEvent& event
) noexcept {
    return FrameDescriptor {
        .frame_id = event.metadata.frame_id,
        .timestamp_ns = event.metadata.timestamp_ns,
        .target_presentation_timestamp_ns = event.metadata.target_presentation_timestamp_ns,
        .predicted_render_time_ns = event.metadata.predicted_render_time_ns,
        .width = event.payload.width,
        .height = event.payload.height,
        .pixel_format = event.payload.pixel_format,
        .color_space = event.payload.color_space,
        .content_type = event.metadata.content_type,
        .protected_content = event.payload.protected_content,
        .damage_ratio = event.payload.damage_ratio,
        .cursor_visible = event.payload.cursor_visible,
        .cursor_x = event.payload.cursor_x,
        .cursor_y = event.payload.cursor_y,
        .cursor_velocity_x = event.payload.cursor_velocity_x,
        .cursor_velocity_y = event.payload.cursor_velocity_y,
        .gpu_handle = event.payload.gpu_handle,
    };
}

PresentFeedback KfiKwinHookAdapter::to_present_feedback(
    const PresentCompletedEvent& event
) noexcept {
    return PresentFeedback {
        .frame_id = event.frame_id,
        .presented_timestamp_ns = event.presented_timestamp_ns,
        .refresh_interval_ns = event.refresh_interval_ns,
        .presentation_mode = event.presentation_mode,
        .present_success = event.present_success,
        .dropped_synthetic = event.dropped_synthetic,
    };
}

}  // namespace fluxma
