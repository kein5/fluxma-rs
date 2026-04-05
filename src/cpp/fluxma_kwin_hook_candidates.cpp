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

constexpr KwinFrameInputField required_missing(
    KwinFrameInputField required_fields,
    KwinFrameInputField missing_fields
) noexcept {
    return static_cast<KwinFrameInputField>(raw(required_fields) & raw(missing_fields));
}

constexpr KwinPresentInputField required_missing(
    KwinPresentInputField required_fields,
    KwinPresentInputField missing_fields
) noexcept {
    return static_cast<KwinPresentInputField>(raw(required_fields) & raw(missing_fields));
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
        .unresolved_fields = combine(
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
        .unresolved_fields = combine(
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
        .unresolved_fields = combine(
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
        .unresolved_fields = combine(
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
    const auto input_missing = KfiKwinFrameBuilder::missing_required_fields(inputs);
    const auto plan_missing = required_missing(plan.required_fields, input_missing);
    return KwinFrameHookReadiness {
        .plan = plan,
        .missing_fields = plan_missing,
        .unresolved_fields = plan.unresolved_fields,
        .ready = plan_missing == KwinFrameInputField::None &&
            plan.unresolved_fields == KwinFrameInputField::None,
    };
}

KwinPresentHookReadiness KfiKwinHookCandidates::assess(
    const KwinPresentHookCandidatePlan& plan,
    const KwinPresentFeedbackInputs& inputs
) noexcept {
    const auto input_missing = KfiKwinPresentBuilder::missing_required_fields(inputs);
    const auto plan_missing = required_missing(plan.required_fields, input_missing);
    return KwinPresentHookReadiness {
        .plan = plan,
        .missing_fields = plan_missing,
        .unresolved_fields = plan.unresolved_fields,
        .ready = plan_missing == KwinPresentInputField::None &&
            plan.unresolved_fields == KwinPresentInputField::None,
    };
}

std::string_view to_string(KwinFrameHookPoint hook_point) noexcept {
    switch (hook_point) {
    case KwinFrameHookPoint::Unknown:
        return "unknown";
    case KwinFrameHookPoint::CompositorOutputFrameReady:
        return "compositor-output-frame-ready";
    case KwinFrameHookPoint::BackendPresentHandoff:
        return "backend-present-handoff";
    }

    return "unknown";
}

std::string_view to_string(KwinPresentHookPoint hook_point) noexcept {
    switch (hook_point) {
    case KwinPresentHookPoint::Unknown:
        return "unknown";
    case KwinPresentHookPoint::OutputFramePresented:
        return "output-frame-presented";
    case KwinPresentHookPoint::RenderLoopFramePresented:
        return "render-loop-frame-presented";
    }

    return "unknown";
}

std::array<std::string_view, 5> describe_required(KwinFrameHookCandidatePlan plan) noexcept {
    return describe(plan.required_fields);
}

std::array<std::string_view, 3> describe_required(KwinPresentHookCandidatePlan plan) noexcept {
    return describe(plan.required_fields);
}

std::array<std::string_view, 5> describe_unresolved(KwinFrameHookCandidatePlan plan) noexcept {
    return describe(plan.unresolved_fields);
}

std::array<std::string_view, 3> describe_unresolved(KwinPresentHookCandidatePlan plan) noexcept {
    return describe(plan.unresolved_fields);
}

}  // namespace fluxma
