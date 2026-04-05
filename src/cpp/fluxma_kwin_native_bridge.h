#pragma once

#include <array>
#include <string>

#include "fluxma_kwin_hook_adapter.h"

namespace fluxma {

enum class KwinNativeBridgeState : std::uint8_t {
    PlaceholderOnly = 0,
    Hooked = 1,
};

struct KwinNativeBringupReport {
    KwinNativeBridgeState state = KwinNativeBridgeState::PlaceholderOnly;
    std::string frame_summary {};
    std::string present_summary {};

    [[nodiscard]] std::string combined_summary() const;
};

class KfiKwinNativeBridge {
  public:
    explicit KfiKwinNativeBridge(KfiKwinHookAdapter& hook_adapter) noexcept;

    [[nodiscard]] KwinNativeBridgeState state() const noexcept;
    [[nodiscard]] bool is_installed() const noexcept;
    [[nodiscard]] KwinFrameHookCandidatePlan frame_candidate() const noexcept;
    [[nodiscard]] KwinPresentHookCandidatePlan present_candidate() const noexcept;
    [[nodiscard]] std::array<std::string_view, 5> frame_checklist() const noexcept;
    [[nodiscard]] std::array<std::string_view, 3> present_checklist() const noexcept;
    [[nodiscard]] KwinNativeBringupReport build_report(
        const KwinCompositorFrameInputs& frame_inputs,
        const KwinPresentFeedbackInputs& present_inputs
    ) const;

  private:
    KfiKwinHookAdapter& hook_adapter_;
};

[[nodiscard]] std::string_view to_string(KwinNativeBridgeState state) noexcept;

}  // namespace fluxma
