#pragma once

#include <array>
#include <string>
#include <string_view>

#include "fluxma_kwin_hook_adapter.h"
#include "fluxma_kwin_hook_candidates.h"

namespace fluxma {

enum class KwinNativeBridgeState : std::uint8_t {
    PlaceholderOnly = 0,
    Hooked = 1,
};

enum class KwinNativeInstallResult : std::uint8_t {
    Deferred = 0,
    Installed = 1,
};

enum class KwinNativeDeferredReason : std::uint8_t {
    PlaceholderOnly = 0,
    KwinVersionGate = 1,
    BackendGate = 2,
};

struct KwinNativeInstallContext {
    bool kwin_version_supported = true;
    bool backend_supported = true;
    std::string_view kwin_version = "unspecified";
    std::string_view backend_name = "unspecified";
};

struct KwinNativeBringupReport {
    KwinNativeBridgeState state = KwinNativeBridgeState::PlaceholderOnly;
    KwinFrameHookReadiness frame {};
    KwinPresentHookReadiness present {};

    [[nodiscard]] bool is_placeholder_state() const noexcept;
    // These report whether the required hook fields are populated; unresolved candidate semantics
    // may still remain until real KWin private/internal hooks are validated.
    [[nodiscard]] bool frame_complete() const noexcept;
    [[nodiscard]] bool present_complete() const noexcept;
    [[nodiscard]] bool fully_populated() const noexcept;
    [[nodiscard]] bool frame_has_unresolved() const noexcept;
    [[nodiscard]] bool present_has_unresolved() const noexcept;
    [[nodiscard]] bool has_unresolved_candidates() const noexcept;
    [[nodiscard]] std::string frame_summary() const;
    [[nodiscard]] std::string present_summary() const;
    [[nodiscard]] std::string combined_summary() const;
};

struct KwinNativeInstallReport {
    KwinNativeInstallResult result = KwinNativeInstallResult::Deferred;
    KwinNativeDeferredReason deferred_reason = KwinNativeDeferredReason::PlaceholderOnly;
    std::string reason {};
    std::string context_summary {};
    std::string target {};
    std::string installer_entry {};
    std::string source_file {};
    std::string symbol {};
    std::string checklist_hint {};
    std::string checklist_hint_secondary {};
    std::string checklist_all {};

    [[nodiscard]] std::string summary() const;
};

struct KwinNativeInstallGateAssessment {
    KwinNativeDeferredReason deferred_reason = KwinNativeDeferredReason::PlaceholderOnly;
    std::string reason {};
    std::string context_summary {};
    bool version_blocked = false;
    bool backend_blocked = false;

    [[nodiscard]] std::string summary() const;
};

struct KwinNativeInstallPreflightReport {
    KwinNativeInstallGateAssessment gate {};
    std::string target {};
    std::string installer_entry {};
    std::string source_file {};
    std::string symbol {};
    std::string checklist_hint {};
    std::string checklist_hint_secondary {};

    [[nodiscard]] std::string summary() const;
};

struct KwinNativeCombinedPreflightReport {
    KwinNativeInstallPreflightReport frame {};
    KwinNativeInstallPreflightReport present {};

    [[nodiscard]] std::string summary() const;
};

struct KwinNativeCombinedInstallReport {
    KwinNativeInstallReport frame {};
    KwinNativeInstallReport present {};

    [[nodiscard]] std::string summary() const;
};

class KfiKwinNativeBridge {
  public:
    explicit KfiKwinNativeBridge(KfiKwinHookAdapter& hook_adapter) noexcept;

    [[nodiscard]] KwinNativeBridgeState state() const noexcept;
    [[nodiscard]] bool is_installed() const noexcept;
    [[nodiscard]] KwinFrameHookCandidatePlan frame_candidate() const noexcept;
    [[nodiscard]] KwinPresentHookCandidatePlan present_candidate() const noexcept;
    [[nodiscard]] std::string_view frame_installer_entry() const noexcept;
    [[nodiscard]] std::string_view present_installer_entry() const noexcept;
    [[nodiscard]] std::array<std::string_view, 5> frame_checklist() const noexcept;
    [[nodiscard]] std::array<std::string_view, 3> present_checklist() const noexcept;
    [[nodiscard]] KwinNativeInstallGateAssessment assess_install_gate(
        const KwinNativeInstallContext& context
    ) const;
    [[nodiscard]] KwinNativeInstallPreflightReport preflight_frame_install(
        const KwinNativeInstallContext& context
    ) const;
    [[nodiscard]] KwinNativeInstallPreflightReport preflight_present_install(
        const KwinNativeInstallContext& context
    ) const;
    [[nodiscard]] KwinNativeCombinedPreflightReport preflight_install(
        const KwinNativeInstallContext& context
    ) const;
    [[nodiscard]] KwinNativeInstallReport install_frame_stub() const;
    [[nodiscard]] KwinNativeInstallReport install_frame_stub(
        const KwinNativeInstallContext& context
    ) const;
    [[nodiscard]] KwinNativeInstallReport install_present_stub() const;
    [[nodiscard]] KwinNativeInstallReport install_present_stub(
        const KwinNativeInstallContext& context
    ) const;
    [[nodiscard]] KwinNativeCombinedInstallReport install_stub() const;
    [[nodiscard]] KwinNativeCombinedInstallReport install_stub(
        const KwinNativeInstallContext& context
    ) const;
    [[nodiscard]] KwinNativeBringupReport build_report(
        const KwinCompositorFrameInputs& frame_inputs,
        const KwinPresentFeedbackInputs& present_inputs
    ) const;

  private:
    KfiKwinHookAdapter& hook_adapter_;
};

[[nodiscard]] std::string_view to_string(KwinNativeBridgeState state) noexcept;
[[nodiscard]] std::string_view to_string(KwinNativeInstallResult result) noexcept;
[[nodiscard]] std::string_view to_string(KwinNativeDeferredReason reason) noexcept;

}  // namespace fluxma
