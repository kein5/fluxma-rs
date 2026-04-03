#include "fluxma_kwin_hook_candidates.h"

namespace fluxma {

namespace {

constexpr std::uint32_t raw(KwinFrameInputField field) noexcept {
    return static_cast<std::uint32_t>(field);
}

constexpr std::uint32_t raw(KwinPresentInputField field) noexcept {
    return static_cast<std::uint32_t>(field);
}

constexpr KwinFrameInputField combine(
    KwinFrameInputField first,
    KwinFrameInputField second,
    KwinFrameInputField third,
    KwinFrameInputField fourth,
    KwinFrameInputField fifth
) noexcept {
    return static_cast<KwinFrameInputField>(
        raw(first) | raw(second) | raw(third) | raw(fourth) | raw(fifth)
    );
}

constexpr KwinPresentInputField combine(
    KwinPresentInputField first,
    KwinPresentInputField second,
    KwinPresentInputField third
) noexcept {
    return static_cast<KwinPresentInputField>(raw(first) | raw(second) | raw(third));
}

}  // namespace

KwinFrameHookCandidatePlan KfiKwinHookCandidates::compositor_output_frame_ready() noexcept {
    return KwinFrameHookCandidatePlan {
        .hook_point = KwinFrameHookPoint::CompositorOutputFrameReady,
        .source_file = "src/compositor_wayland.cpp",
        .symbol = "WaylandCompositor::composite(RenderLoop *)",
        .note =
            "Preferred frame tap candidate after OutputFrame construction and before "
            "Backend::present(output, frame).",
        .field_sources = KfiKwinFrameBuilder::compositor_field_sources(),
        .required_fields = combine(
            KwinFrameInputField::FrameId,
            KwinFrameInputField::Timestamp,
            KwinFrameInputField::Width,
            KwinFrameInputField::Height,
            KwinFrameInputField::GpuHandle
        ),
    };
}

KwinFrameHookCandidatePlan KfiKwinHookCandidates::backend_present_handoff() noexcept {
    return KwinFrameHookCandidatePlan {
        .hook_point = KwinFrameHookPoint::BackendPresentHandoff,
        .source_file = "src/backends/*/*_output.cpp or drm_pipeline.cpp",
        .symbol = "BackendOutput::present(..., frame) handoff",
        .note =
            "Fallback frame tap candidate for backend-specific frame handles; MVP policy "
            "still treats this boundary as HookUnavailable until final composed semantics are confirmed.",
        .field_sources = KfiKwinFrameBuilder::backend_present_handoff_field_sources(),
        .required_fields = combine(
            KwinFrameInputField::FrameId,
            KwinFrameInputField::Timestamp,
            KwinFrameInputField::Width,
            KwinFrameInputField::Height,
            KwinFrameInputField::GpuHandle
        ),
    };
}

KwinPresentHookCandidatePlan KfiKwinHookCandidates::output_frame_presented() noexcept {
    return KwinPresentHookCandidatePlan {
        .hook_point = KwinPresentHookPoint::OutputFramePresented,
        .source_file = "src/core/renderbackend.cpp",
        .symbol = "OutputFrame::presented(...)",
        .note =
            "Preferred present feedback candidate when backend completion can still surface "
            "frame id, presentation timestamp, and refresh interval.",
        .field_sources = KfiKwinPresentBuilder::output_frame_presented_field_sources(),
        .required_fields = combine(
            KwinPresentInputField::FrameId,
            KwinPresentInputField::PresentedTimestamp,
            KwinPresentInputField::RefreshInterval
        ),
    };
}

KwinPresentHookCandidatePlan KfiKwinHookCandidates::render_loop_frame_presented() noexcept {
    return KwinPresentHookCandidatePlan {
        .hook_point = KwinPresentHookPoint::RenderLoopFramePresented,
        .source_file = "src/core/renderloop.cpp",
        .symbol = "RenderLoopPrivate::notifyFrameCompleted(...)",
        .note =
            "Fallback present feedback candidate after OutputFrame completion has been folded "
            "into RenderLoop scheduling.",
        .field_sources = KfiKwinPresentBuilder::render_loop_presented_field_sources(),
        .required_fields = combine(
            KwinPresentInputField::FrameId,
            KwinPresentInputField::PresentedTimestamp,
            KwinPresentInputField::RefreshInterval
        ),
    };
}

KwinFrameHookReadiness KfiKwinHookCandidates::assess(
    const KwinFrameHookCandidatePlan& plan,
    const KwinCompositorFrameInputs& inputs
) noexcept {
    const auto missing = KfiKwinFrameBuilder::missing_required_fields(inputs);
    return KwinFrameHookReadiness {
        .plan = plan,
        .missing_fields = static_cast<KwinFrameInputField>(
            raw(missing) & raw(plan.required_fields)
        ),
        .ready = (raw(missing) & raw(plan.required_fields)) == 0,
    };
}

KwinPresentHookReadiness KfiKwinHookCandidates::assess(
    const KwinPresentHookCandidatePlan& plan,
    const KwinPresentFeedbackInputs& inputs
) noexcept {
    const auto missing = KfiKwinPresentBuilder::missing_required_fields(inputs);
    return KwinPresentHookReadiness {
        .plan = plan,
        .missing_fields = static_cast<KwinPresentInputField>(
            raw(missing) & raw(plan.required_fields)
        ),
        .ready = (raw(missing) & raw(plan.required_fields)) == 0,
    };
}

}  // namespace fluxma
