#include "fluxma_kwin_native_bridge.h"
#include "fluxma_kwin_hook_candidates.h"

namespace fluxma {

namespace {

template <std::size_t N>
std::string join_checklist(const std::array<std::string_view, N>& items) {
    std::string output;
    bool first = true;
    for (const auto item : items) {
        if (item.empty()) {
            continue;
        }
        if (!first) {
            output += ",";
        }
        output += item;
        first = false;
    }
    if (output.empty()) {
        return "none";
    }
    return output;
}

KwinNativeInstallReport make_install_report(
    const KwinNativeDeferredReason deferred_reason,
    const std::string& reason,
    const std::string_view target,
    const std::string_view installer_entry,
    const std::string_view source_file,
    const std::string_view symbol,
    const std::string_view checklist_hint,
    const std::string_view checklist_hint_secondary,
    const std::string& checklist_all
) {
    return KwinNativeInstallReport {
        .result = KwinNativeInstallResult::Deferred,
        .deferred_reason = deferred_reason,
        .reason = reason,
        .target = std::string(target),
        .installer_entry = std::string(installer_entry),
        .source_file = std::string(source_file),
        .symbol = std::string(symbol),
        .checklist_hint = std::string(checklist_hint),
        .checklist_hint_secondary = std::string(checklist_hint_secondary),
        .checklist_all = checklist_all,
    };
}

KwinNativeInstallReport make_deferred_install_report(
    const KwinNativeInstallContext& context,
    const std::string_view target,
    const std::string_view installer_entry,
    const std::string_view source_file,
    const std::string_view symbol,
    const std::string_view checklist_hint,
    const std::string_view checklist_hint_secondary,
    const std::string& checklist_all
) {
    if (!context.kwin_version_supported) {
        std::string reason = "kwin version gate blocked install for ";
        reason += context.kwin_version;
        return make_install_report(
            KwinNativeDeferredReason::KwinVersionGate,
            reason,
            target,
            installer_entry,
            source_file,
            symbol,
            checklist_hint,
            checklist_hint_secondary,
            checklist_all
        );
    }

    if (!context.backend_supported) {
        std::string reason = "backend gate blocked install for ";
        reason += context.backend_name;
        return make_install_report(
            KwinNativeDeferredReason::BackendGate,
            reason,
            target,
            installer_entry,
            source_file,
            symbol,
            checklist_hint,
            checklist_hint_secondary,
            checklist_all
        );
    }

    return make_install_report(
        KwinNativeDeferredReason::PlaceholderOnly,
        "native bridge is still placeholder-only",
        target,
        installer_entry,
        source_file,
        symbol,
        checklist_hint,
        checklist_hint_secondary,
        checklist_all
    );
}

}  // namespace

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
    output += " deferred_reason=";
    output += std::string(to_string(deferred_reason));
    output += " reason=";
    output += reason;
    output += " target=";
    output += target;
    output += " installer_entry=";
    output += installer_entry;
    output += " source=";
    output += source_file;
    output += " symbol=";
    output += symbol;
    output += " checklist_hint=";
    output += checklist_hint;
    output += " checklist_hint_secondary=";
    output += checklist_hint_secondary;
    output += " checklist_all=";
    output += checklist_all;
    return output;
}

std::string KwinNativeCombinedInstallReport::summary() const {
    std::string output;
    output += "frame{";
    output += frame.summary();
    output += "} present{";
    output += present.summary();
    output += "}";
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

std::string_view KfiKwinNativeBridge::frame_installer_entry() const noexcept {
    return "KfiKwinNativeBridge::install_frame_stub";
}

std::string_view KfiKwinNativeBridge::present_installer_entry() const noexcept {
    return "KfiKwinNativeBridge::install_present_stub";
}

std::array<std::string_view, 5> KfiKwinNativeBridge::frame_checklist() const noexcept {
    return hook_adapter_.preferred_frame_checklist();
}

std::array<std::string_view, 3> KfiKwinNativeBridge::present_checklist() const noexcept {
    return hook_adapter_.preferred_present_checklist();
}

KwinNativeInstallReport KfiKwinNativeBridge::install_frame_stub() const {
    return install_frame_stub(KwinNativeInstallContext {});
}

KwinNativeInstallReport KfiKwinNativeBridge::install_frame_stub(
    const KwinNativeInstallContext& context
) const {
    const auto candidate = frame_candidate();
    const auto checklist = frame_checklist();
    return make_deferred_install_report(
        context,
        to_string(candidate.hook_point),
        frame_installer_entry(),
        candidate.source_file,
        candidate.symbol,
        checklist[0],
        checklist[1],
        join_checklist(checklist)
    );
}

KwinNativeInstallReport KfiKwinNativeBridge::install_present_stub() const {
    return install_present_stub(KwinNativeInstallContext {});
}

KwinNativeInstallReport KfiKwinNativeBridge::install_present_stub(
    const KwinNativeInstallContext& context
) const {
    const auto candidate = present_candidate();
    const auto checklist = present_checklist();
    return make_deferred_install_report(
        context,
        to_string(candidate.hook_point),
        present_installer_entry(),
        candidate.source_file,
        candidate.symbol,
        checklist[0],
        checklist[1],
        join_checklist(checklist)
    );
}

KwinNativeCombinedInstallReport KfiKwinNativeBridge::install_stub() const {
    return install_stub(KwinNativeInstallContext {});
}

KwinNativeCombinedInstallReport KfiKwinNativeBridge::install_stub(
    const KwinNativeInstallContext& context
) const {
    return KwinNativeCombinedInstallReport {
        .frame = install_frame_stub(context),
        .present = install_present_stub(context),
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

std::string_view to_string(KwinNativeDeferredReason reason) noexcept {
    switch (reason) {
    case KwinNativeDeferredReason::PlaceholderOnly:
        return "placeholder-only";
    case KwinNativeDeferredReason::KwinVersionGate:
        return "kwin-version-gate";
    case KwinNativeDeferredReason::BackendGate:
        return "backend-gate";
    }

    return "unknown";
}

}  // namespace fluxma
