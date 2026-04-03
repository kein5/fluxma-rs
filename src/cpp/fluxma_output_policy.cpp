#include "fluxma_output_policy.h"

#include "fluxma_kwin_hook_adapter.h"

namespace fluxma {

KfiOutputPolicy::KfiOutputPolicy(std::uint32_t target_output_id)
    : target_output_id_(target_output_id) {}

bool KfiOutputPolicy::accepts_output(std::uint32_t output_id) const noexcept {
    return output_id == target_output_id_;
}

bool KfiOutputPolicy::supports_frame_hook_context(
    const KwinFrameHookContext& context
) const noexcept {
    switch (context.hook_point) {
    case KwinFrameHookPoint::CompositorOutputFrameReady:
        return context.cursor_composited_in_frame && !context.overlay_promoted;
    case KwinFrameHookPoint::BackendPresentHandoff:
        return false;
    case KwinFrameHookPoint::Unknown:
        return false;
    }

    return false;
}

bool KfiOutputPolicy::supports_present_hook_context(
    const KwinPresentHookContext& context
) const noexcept {
    switch (context.hook_point) {
    case KwinPresentHookPoint::OutputFramePresented:
    case KwinPresentHookPoint::RenderLoopFramePresented:
        return true;
    case KwinPresentHookPoint::Unknown:
        return false;
    }

    return false;
}

bool KfiOutputPolicy::supports_present_event(
    const PresentCompletedEvent& event
) const noexcept {
    return event.frame_id != 0 && event.presented_timestamp_ns != 0 &&
        event.refresh_interval_ns != 0;
}

bool KfiOutputPolicy::supports_frame_event(
    const FinalComposedFrameEvent& event
) const noexcept {
    return event.payload.width > 0 && event.payload.height > 0 &&
        event.payload.gpu_handle.handle_id != 0;
}

OutputDecision KfiOutputPolicy::classify_frame_hook_context(
    const KwinFrameHookContext& context
) const noexcept {
    if (supports_frame_hook_context(context)) {
        return OutputDecision {
            .state = OutputState::Bypass,
            .bypass_reason = BypassReason::None,
            .passthrough_only = true,
        };
    }

    return OutputDecision {
        .state = OutputState::Bypass,
        .bypass_reason = BypassReason::HookUnavailable,
        .passthrough_only = true,
    };
}

OutputDecision KfiOutputPolicy::classify_frame_event(
    const FinalComposedFrameEvent& event
) const noexcept {
    if (!accepts_output(event.output_id)) {
        return OutputDecision {
            .state = OutputState::Bypass,
            .bypass_reason = BypassReason::UnsupportedOutput,
            .passthrough_only = true,
        };
    }

    if (event.payload.protected_content) {
        return OutputDecision {
            .state = OutputState::ProtectedBypass,
            .bypass_reason = BypassReason::ProtectedContent,
            .passthrough_only = true,
        };
    }

    if (!supports_frame_event(event)) {
        return OutputDecision {
            .state = OutputState::Bypass,
            .bypass_reason = BypassReason::UnsupportedOutput,
            .passthrough_only = true,
        };
    }

    return OutputDecision {
        .state = OutputState::Bypass,
        .bypass_reason = BypassReason::None,
        .passthrough_only = true,
    };
}

}  // namespace fluxma
