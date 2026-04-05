#include "fluxma_kwin_native_bridge.h"
#include "fluxma_kwin_hook_candidates.h"

namespace fluxma {

std::string KwinNativeBringupReport::combined_summary() const {
    std::string summary;
    summary += "state=";
    summary += std::string(to_string(state));
    summary += " frame{";
    summary += frame_summary;
    summary += "} present{";
    summary += present_summary;
    summary += "}";
    return summary;
}

std::string KwinNativeInstallReport::summary() const {
    std::string output;
    output += "result=";
    output += std::string(to_string(result));
    output += " reason=";
    output += reason;
    output += " frame=";
    output += frame_candidate;
    output += " present=";
    output += present_candidate;
    return output;
}

KfiKwinNativeBridge::KfiKwinNativeBridge(KfiKwinHookAdapter& hook_adapter) noexcept
    : hook_adapter_(hook_adapter) {}

KwinNativeBridgeState KfiKwinNativeBridge::state() const noexcept {
    return KwinNativeBridgeState::PlaceholderOnly;
}

bool KfiKwinNativeBridge::is_installed() const noexcept {
    return state() == KwinNativeBridgeState::Hooked;
}

KwinFrameHookCandidatePlan KfiKwinNativeBridge::frame_candidate() const noexcept {
    return hook_adapter_.preferred_frame_candidate();
}

KwinPresentHookCandidatePlan KfiKwinNativeBridge::present_candidate() const noexcept {
    return hook_adapter_.preferred_present_candidate();
}

std::array<std::string_view, 5> KfiKwinNativeBridge::frame_checklist() const noexcept {
    return hook_adapter_.preferred_frame_checklist();
}

std::array<std::string_view, 3> KfiKwinNativeBridge::present_checklist() const noexcept {
    return hook_adapter_.preferred_present_checklist();
}

KwinNativeInstallReport KfiKwinNativeBridge::install_stub() const {
    return KwinNativeInstallReport {
        .result = KwinNativeInstallResult::Deferred,
        .reason = "native bridge is still placeholder-only",
        .frame_candidate = std::string(to_string(frame_candidate().hook_point)),
        .present_candidate = std::string(to_string(present_candidate().hook_point)),
    };
}

KwinNativeBringupReport KfiKwinNativeBridge::build_report(
    const KwinCompositorFrameInputs& frame_inputs,
    const KwinPresentFeedbackInputs& present_inputs
) const {
    return KwinNativeBringupReport {
        .state = state(),
        .frame_summary = hook_adapter_.summarize_frame_candidate(frame_inputs),
        .present_summary = hook_adapter_.summarize_present_candidate(present_inputs),
    };
}

std::string_view to_string(KwinNativeBridgeState state) noexcept {
    switch (state) {
    case KwinNativeBridgeState::PlaceholderOnly:
        return "placeholder-only";
    case KwinNativeBridgeState::Hooked:
        return "hooked";
    }

    return "unknown";
}

std::string_view to_string(KwinNativeInstallResult result) noexcept {
    switch (result) {
    case KwinNativeInstallResult::Deferred:
        return "deferred";
    case KwinNativeInstallResult::Installed:
        return "installed";
    }

    return "unknown";
}

}  // namespace fluxma
