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
    const std::string& context_summary,
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
        .context_summary = context_summary,
        .target = std::string(target),
        .installer_entry = std::string(installer_entry),
        .source_file = std::string(source_file),
        .symbol = std::string(symbol),
        .checklist_hint = std::string(checklist_hint),
        .checklist_hint_secondary = std::string(checklist_hint_secondary),
        .checklist_all = checklist_all,
    };
}

std::string build_install_context_summary(const KwinNativeInstallContext& context) {
    std::string summary = "kwin=";
    summary += context.kwin_version;
    summary += " backend=";
    summary += context.backend_name;
    summary += " version_supported=";
    summary += context.kwin_version_supported ? "true" : "false";
    summary += " backend_supported=";
    summary += context.backend_supported ? "true" : "false";
    return summary;
}

KwinNativeInstallReport make_deferred_install_report(
    const KwinNativeInstallGateAssessment& assessment,
    const std::string_view target,
    const std::string_view installer_entry,
    const std::string_view source_file,
    const std::string_view symbol,
    const std::string_view checklist_hint,
    const std::string_view checklist_hint_secondary,
    const std::string& checklist_all
) {
    return make_install_report(
        assessment.deferred_reason,
        assessment.reason,
        assessment.context_summary,
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

bool KwinNativeBringupReport::frame_ready() const noexcept {
    return frame.ready;
}

bool KwinNativeBringupReport::present_ready() const noexcept {
    return present.ready;
}

bool KwinNativeBringupReport::fully_ready() const noexcept {
    return frame_ready() && present_ready();
}

std::string KwinNativeBringupReport::frame_summary() const {
    return summarize(frame);
}

std::string KwinNativeBringupReport::present_summary() const {
    return summarize(present);
}

std::string KwinNativeBringupReport::combined_summary() const {
    std::string summary;
    summary += "state=";
    summary += std::string(to_string(state));
    summary += " frame{";
    summary += frame_summary();
    summary += "} present{";
    summary += present_summary();
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
    output += " context=";
    output += context_summary;
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

std::string KwinNativeInstallGateAssessment::summary() const {
    std::string output;
    output += "deferred_reason=";
    output += std::string(to_string(deferred_reason));
    output += " reason=";
    output += reason;
    output += " context=";
    output += context_summary;
    output += " version_blocked=";
    output += version_blocked ? "true" : "false";
    output += " backend_blocked=";
    output += backend_blocked ? "true" : "false";
    return output;
}

std::string KwinNativeInstallPreflightReport::summary() const {
    std::string output;
    output += gate.summary();
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
    return output;
}

std::string KwinNativeCombinedPreflightReport::summary() const {
    std::string output;
    output += "frame{";
    output += frame.summary();
    output += "} present{";
    output += present.summary();
    output += "}";
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

KwinNativeInstallGateAssessment KfiKwinNativeBridge::assess_install_gate(
    const KwinNativeInstallContext& context
) const {
    KwinNativeInstallGateAssessment assessment {
        .context_summary = build_install_context_summary(context),
        .version_blocked = !context.kwin_version_supported,
        .backend_blocked = !context.backend_supported,
    };

    if (assessment.version_blocked) {
        assessment.deferred_reason = KwinNativeDeferredReason::KwinVersionGate;
        assessment.reason = "kwin version gate blocked install for ";
        assessment.reason += context.kwin_version;
        return assessment;
    }

    if (assessment.backend_blocked) {
        assessment.deferred_reason = KwinNativeDeferredReason::BackendGate;
        assessment.reason = "backend gate blocked install for ";
        assessment.reason += context.backend_name;
        return assessment;
    }

    assessment.deferred_reason = KwinNativeDeferredReason::PlaceholderOnly;
    assessment.reason = "native bridge is still placeholder-only";
    return assessment;
}

KwinNativeInstallPreflightReport KfiKwinNativeBridge::preflight_frame_install(
    const KwinNativeInstallContext& context
) const {
    const auto candidate = frame_candidate();
    const auto checklist = frame_checklist();
    return KwinNativeInstallPreflightReport {
        .gate = assess_install_gate(context),
        .target = std::string(to_string(candidate.hook_point)),
        .installer_entry = std::string(frame_installer_entry()),
        .source_file = std::string(candidate.source_file),
        .symbol = std::string(candidate.symbol),
        .checklist_hint = std::string(checklist[0]),
        .checklist_hint_secondary = std::string(checklist[1]),
    };
}

KwinNativeInstallPreflightReport KfiKwinNativeBridge::preflight_present_install(
    const KwinNativeInstallContext& context
) const {
    const auto candidate = present_candidate();
    const auto checklist = present_checklist();
    return KwinNativeInstallPreflightReport {
        .gate = assess_install_gate(context),
        .target = std::string(to_string(candidate.hook_point)),
        .installer_entry = std::string(present_installer_entry()),
        .source_file = std::string(candidate.source_file),
        .symbol = std::string(candidate.symbol),
        .checklist_hint = std::string(checklist[0]),
        .checklist_hint_secondary = std::string(checklist[1]),
    };
}

KwinNativeCombinedPreflightReport KfiKwinNativeBridge::preflight_install(
    const KwinNativeInstallContext& context
) const {
    return KwinNativeCombinedPreflightReport {
        .frame = preflight_frame_install(context),
        .present = preflight_present_install(context),
    };
}

KwinNativeInstallReport KfiKwinNativeBridge::install_frame_stub() const {
    return install_frame_stub(KwinNativeInstallContext {});
}

KwinNativeInstallReport KfiKwinNativeBridge::install_frame_stub(
    const KwinNativeInstallContext& context
) const {
    const auto candidate = frame_candidate();
    const auto checklist = frame_checklist();
    const auto assessment = assess_install_gate(context);
    return make_deferred_install_report(
        assessment,
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
    const auto assessment = assess_install_gate(context);
    return make_deferred_install_report(
        assessment,
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
        .frame = hook_adapter_.assess_frame_candidate(frame_inputs),
        .present = hook_adapter_.assess_present_candidate(present_inputs),
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
