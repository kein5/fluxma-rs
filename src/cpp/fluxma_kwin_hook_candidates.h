#pragma once

#include <array>
#include <string>
#include <string_view>

#include "fluxma_kwin_hook_builders.h"

namespace fluxma {

struct KwinFrameHookCandidatePlan {
    KwinFrameHookPoint hook_point = KwinFrameHookPoint::Unknown;
    std::string_view source_file {};
    std::string_view symbol {};
    std::string_view note {};
    KwinFrameFieldSources field_sources {};
    KwinFrameInputField required_fields = KwinFrameInputField::None;
    KwinFrameInputField unresolved_fields = KwinFrameInputField::None;
};

struct KwinPresentHookCandidatePlan {
    KwinPresentHookPoint hook_point = KwinPresentHookPoint::Unknown;
    std::string_view source_file {};
    std::string_view symbol {};
    std::string_view note {};
    KwinPresentFieldSources field_sources {};
    KwinPresentInputField required_fields = KwinPresentInputField::None;
    KwinPresentInputField unresolved_fields = KwinPresentInputField::None;
};

struct KwinFrameHookReadiness {
    KwinFrameHookCandidatePlan plan {};
    KwinFrameInputField missing_fields = KwinFrameInputField::None;
    KwinFrameInputField unresolved_fields = KwinFrameInputField::None;
    bool ready = false;
};

struct KwinPresentHookReadiness {
    KwinPresentHookCandidatePlan plan {};
    KwinPresentInputField missing_fields = KwinPresentInputField::None;
    KwinPresentInputField unresolved_fields = KwinPresentInputField::None;
    bool ready = false;
};

class KfiKwinHookCandidates {
  public:
    [[nodiscard]] static KwinFrameHookCandidatePlan compositor_output_frame_ready() noexcept;
    [[nodiscard]] static KwinFrameHookCandidatePlan backend_present_handoff() noexcept;
    [[nodiscard]] static KwinPresentHookCandidatePlan output_frame_presented() noexcept;
    [[nodiscard]] static KwinPresentHookCandidatePlan render_loop_frame_presented() noexcept;
    [[nodiscard]] static KwinFrameHookReadiness assess(
        const KwinFrameHookCandidatePlan& plan,
        const KwinCompositorFrameInputs& inputs
    ) noexcept;
    [[nodiscard]] static KwinPresentHookReadiness assess(
        const KwinPresentHookCandidatePlan& plan,
        const KwinPresentFeedbackInputs& inputs
    ) noexcept;
};

[[nodiscard]] std::string_view to_string(KwinFrameHookPoint hook_point) noexcept;
[[nodiscard]] std::string_view to_string(KwinPresentHookPoint hook_point) noexcept;
[[nodiscard]] std::array<std::string_view, 5> describe_required(
    KwinFrameHookCandidatePlan plan
) noexcept;
[[nodiscard]] std::array<std::string_view, 3> describe_required(
    KwinPresentHookCandidatePlan plan
) noexcept;
[[nodiscard]] std::array<std::string_view, 5> describe_unresolved(
    KwinFrameHookCandidatePlan plan
) noexcept;
[[nodiscard]] std::array<std::string_view, 3> describe_unresolved(
    KwinPresentHookCandidatePlan plan
) noexcept;
[[nodiscard]] std::string summarize(KwinFrameHookReadiness readiness);
[[nodiscard]] std::string summarize(KwinPresentHookReadiness readiness);

}  // namespace fluxma
