#include "fluxma_output_policy.h"

#include "fluxma_kwin_hook_adapter.h"

namespace fluxma {

KfiOutputPolicy::KfiOutputPolicy(std::uint32_t target_output_id)
    : target_output_id_(target_output_id) {}

bool KfiOutputPolicy::accepts_output(std::uint32_t output_id) const noexcept {
    return output_id == target_output_id_;
}

bool KfiOutputPolicy::supports_frame_event(
    const FinalComposedFrameEvent& event
) const noexcept {
    return event.payload.width > 0 && event.payload.height > 0 &&
        event.payload.gpu_handle.handle_id != 0;
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
