#pragma once

#include <array>
#include <string>
#include <string_view>

#include "fluxma_kwin_hook_adapter.h"

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
    std::string frame_summary {};
    std::string present_summary {};

    [[nodiscard]] std::string combined_summary() const;
};

struct KwinNativeInstallReport {
    KwinNativeInstallResult result = KwinNativeInstallResult::Deferred;
    KwinNativeDeferredReason deferred_reason = KwinNativeDeferredReason::PlaceholderOnly;
    std::string reason {};
    std::string target {};
    std::string installer_entry {};
    std::string source_file {};
    std::string symbol {};
    std::string checklist_hint {};
    std::string checklist_hint_secondary {};
    std::string checklist_all {};

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
