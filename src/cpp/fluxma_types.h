#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include "fluxma_rs_core.h"

namespace fluxma {

enum class OutputState : std::uint8_t {
    Disabled = FLUXMA_RUST_OUTPUT_STATE_DISABLED,
    Bypass = FLUXMA_RUST_OUTPUT_STATE_BYPASS,
    Warmup = FLUXMA_RUST_OUTPUT_STATE_WARMUP,
    Active2x = FLUXMA_RUST_OUTPUT_STATE_ACTIVE_2X,
    Degraded = FLUXMA_RUST_OUTPUT_STATE_DEGRADED,
    ProtectedBypass = FLUXMA_RUST_OUTPUT_STATE_PROTECTED_BYPASS,
    Faulted = FLUXMA_RUST_OUTPUT_STATE_FAULTED,
};

enum class BypassReason : std::uint8_t {
    None = FLUXMA_RUST_BYPASS_REASON_NONE,
    Disabled = FLUXMA_RUST_BYPASS_REASON_DISABLED,
    ProtectedContent = FLUXMA_RUST_BYPASS_REASON_PROTECTED_CONTENT,
    HookUnavailable = FLUXMA_RUST_BYPASS_REASON_HOOK_UNAVAILABLE,
    UnsupportedOutput = FLUXMA_RUST_BYPASS_REASON_UNSUPPORTED_OUTPUT,
    GpuPathNotReady = FLUXMA_RUST_BYPASS_REASON_GPU_PATH_NOT_READY,
    Fault = FLUXMA_RUST_BYPASS_REASON_FAULT,
};

enum class PresentationMode : std::uint8_t {
    VSync = FLUXMA_RUST_PRESENTATION_MODE_VSYNC,
    AdaptiveSync = FLUXMA_RUST_PRESENTATION_MODE_ADAPTIVE_SYNC,
    Async = FLUXMA_RUST_PRESENTATION_MODE_ASYNC,
    AdaptiveAsync = FLUXMA_RUST_PRESENTATION_MODE_ADAPTIVE_ASYNC,
};

enum class ContentType : std::uint8_t {
    None = FLUXMA_RUST_CONTENT_TYPE_NONE,
    Photo = FLUXMA_RUST_CONTENT_TYPE_PHOTO,
    Video = FLUXMA_RUST_CONTENT_TYPE_VIDEO,
    Game = FLUXMA_RUST_CONTENT_TYPE_GAME,
};

enum class CadenceStatus : std::uint8_t {
    Unknown = FLUXMA_RUST_CADENCE_STATUS_UNKNOWN,
    Unstable = FLUXMA_RUST_CADENCE_STATUS_UNSTABLE,
    Stable = FLUXMA_RUST_CADENCE_STATUS_STABLE,
};

enum class GovernorMode : std::uint8_t {
    Bypass = FLUXMA_RUST_GOVERNOR_MODE_BYPASS,
    QualityLow = FLUXMA_RUST_GOVERNOR_MODE_QUALITY_LOW,
    QualityMedium = FLUXMA_RUST_GOVERNOR_MODE_QUALITY_MEDIUM,
    QualityHigh = FLUXMA_RUST_GOVERNOR_MODE_QUALITY_HIGH,
};

enum class SchedulerMode : std::uint8_t {
    PassthroughOnly = FLUXMA_RUST_SCHEDULER_MODE_PASSTHROUGH_ONLY,
    WarmupHold = FLUXMA_RUST_SCHEDULER_MODE_WARMUP_HOLD,
    Synthetic2x = FLUXMA_RUST_SCHEDULER_MODE_SYNTHETIC_2X,
};

struct GpuFrameHandle {
    std::uint32_t backend_kind = 0;
    std::uint64_t handle_id = 0;
};

struct FrameDescriptor {
    std::uint64_t frame_id = 0;
    std::uint64_t timestamp_ns = 0;
    std::uint64_t target_presentation_timestamp_ns = 0;
    std::uint64_t predicted_render_time_ns = 0;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::uint32_t pixel_format = 0;
    std::uint32_t color_space = 0;
    ContentType content_type = ContentType::None;
    bool protected_content = false;
    double damage_ratio = 0.0;
    bool cursor_visible = false;
    double cursor_x = 0.0;
    double cursor_y = 0.0;
    double cursor_velocity_x = 0.0;
    double cursor_velocity_y = 0.0;
    GpuFrameHandle gpu_handle {};

    [[nodiscard]] FluxmaRustFrameDescriptor to_ffi() const noexcept {
        return FluxmaRustFrameDescriptor {
            .frame_id = frame_id,
            .timestamp_ns = timestamp_ns,
            .target_presentation_timestamp_ns = target_presentation_timestamp_ns,
            .predicted_render_time_ns = predicted_render_time_ns,
            .width = width,
            .height = height,
            .pixel_format = pixel_format,
            .color_space = color_space,
            .content_type = static_cast<FluxmaRustContentType>(content_type),
            .protected_content = static_cast<std::uint8_t>(protected_content ? 1 : 0),
            .reserved = {0, 0},
            .damage_ratio = damage_ratio,
            .cursor_visible = static_cast<std::uint8_t>(cursor_visible ? 1 : 0),
            .cursor_reserved = {0, 0, 0, 0, 0, 0, 0},
            .cursor_x = cursor_x,
            .cursor_y = cursor_y,
            .cursor_velocity_x = cursor_velocity_x,
            .cursor_velocity_y = cursor_velocity_y,
            .gpu_handle =
                FluxmaRustGpuFrameHandle {
                    .backend_kind = gpu_handle.backend_kind,
                    .handle_id = gpu_handle.handle_id,
                },
        };
    }
};

struct PresentFeedback {
    std::uint64_t frame_id = 0;
    std::uint64_t presented_timestamp_ns = 0;
    std::uint64_t refresh_interval_ns = 0;
    PresentationMode presentation_mode = PresentationMode::VSync;
    bool present_success = false;
    bool dropped_synthetic = false;

    [[nodiscard]] FluxmaRustPresentFeedback to_ffi() const noexcept {
        return FluxmaRustPresentFeedback {
            .frame_id = frame_id,
            .presented_timestamp_ns = presented_timestamp_ns,
            .refresh_interval_ns = refresh_interval_ns,
            .presentation_mode = static_cast<FluxmaRustPresentationMode>(presentation_mode),
            .present_success = static_cast<std::uint8_t>(present_success ? 1 : 0),
            .dropped_synthetic = static_cast<std::uint8_t>(dropped_synthetic ? 1 : 0),
            .reserved = {0, 0, 0, 0, 0},
        };
    }
};

struct OutputDecision {
    OutputState state = OutputState::Bypass;
    BypassReason bypass_reason = BypassReason::None;
    bool passthrough_only = true;
    bool interpolation_armed = false;

    [[nodiscard]] static OutputDecision from_ffi(FluxmaRustDecision decision) noexcept {
        return OutputDecision {
            .state = static_cast<OutputState>(decision.state),
            .bypass_reason = static_cast<BypassReason>(decision.bypass_reason),
            .passthrough_only = decision.passthrough_only != 0,
            .interpolation_armed = decision.interpolation_armed != 0,
        };
    }
};

struct MetricsSnapshot {
    OutputState state = OutputState::Bypass;
    BypassReason bypass_reason = BypassReason::None;
    bool protected_content = false;
    bool passthrough_only = true;
    bool classifier_allows_interpolation = false;
    std::uint64_t frame_tap_count = 0;
    std::uint64_t present_feedback_count = 0;
    std::uint64_t deadline_miss_count = 0;
    std::uint64_t dropped_synthetic_count = 0;
    std::uint64_t last_presented_frame_id = 0;
    std::uint64_t last_presented_timestamp_ns = 0;
    std::uint64_t refresh_interval_ns = 0;
    std::uint64_t last_target_presentation_timestamp_ns = 0;
    std::uint64_t last_predicted_render_time_ns = 0;
    PresentationMode last_presentation_mode = PresentationMode::VSync;
    ContentType last_content_type = ContentType::None;
    CadenceStatus cadence_status = CadenceStatus::Unknown;
    GovernorMode governor_mode = GovernorMode::Bypass;
    SchedulerMode scheduler_mode = SchedulerMode::PassthroughOnly;
    std::uint32_t cadence_hz_millihz = 0;
    std::uint64_t state_transition_count = 0;

    [[nodiscard]] bool cadence_stable() const noexcept {
        return cadence_status == CadenceStatus::Stable;
    }

    [[nodiscard]] bool scheduler_can_synthesize() const noexcept {
        return scheduler_mode == SchedulerMode::Synthetic2x;
    }

    [[nodiscard]] bool governor_is_degraded() const noexcept {
        return governor_mode == GovernorMode::QualityLow ||
            governor_mode == GovernorMode::QualityMedium;
    }

    [[nodiscard]] static MetricsSnapshot from_ffi(
        FluxmaRustMetricsSnapshot snapshot
    ) noexcept {
        return MetricsSnapshot {
            .state = static_cast<OutputState>(snapshot.state),
            .bypass_reason = static_cast<BypassReason>(snapshot.bypass_reason),
            .protected_content = snapshot.protected_content != 0,
            .passthrough_only = snapshot.passthrough_only != 0,
            .classifier_allows_interpolation =
                snapshot.classifier_allows_interpolation != 0,
            .frame_tap_count = snapshot.frame_tap_count,
            .present_feedback_count = snapshot.present_feedback_count,
            .deadline_miss_count = snapshot.deadline_miss_count,
            .dropped_synthetic_count = snapshot.dropped_synthetic_count,
            .last_presented_frame_id = snapshot.last_presented_frame_id,
            .last_presented_timestamp_ns = snapshot.last_presented_timestamp_ns,
            .refresh_interval_ns = snapshot.refresh_interval_ns,
            .last_target_presentation_timestamp_ns =
                snapshot.last_target_presentation_timestamp_ns,
            .last_predicted_render_time_ns = snapshot.last_predicted_render_time_ns,
            .last_presentation_mode =
                static_cast<PresentationMode>(snapshot.last_presentation_mode),
            .last_content_type = static_cast<ContentType>(snapshot.last_content_type),
            .cadence_status = static_cast<CadenceStatus>(snapshot.cadence_status),
            .governor_mode = static_cast<GovernorMode>(snapshot.governor_mode),
            .scheduler_mode = static_cast<SchedulerMode>(snapshot.scheduler_mode),
            .cadence_hz_millihz = snapshot.cadence_hz_millihz,
            .state_transition_count = snapshot.state_transition_count,
        };
    }
};

struct PassthroughSubmission {
    std::uint32_t output_id = 0;
    std::uint64_t frame_id = 0;
    GpuFrameHandle source_frame {};
    bool accepted = false;
    bool interpolation_armed = false;
    bool protected_content = false;
    BypassReason bypass_reason = BypassReason::None;
};

struct SyntheticFramePlan {
    std::uint32_t output_id = 0;
    std::uint64_t source_frame_id = 0;
    std::uint64_t synthetic_frame_id = 0;
    std::uint64_t target_present_timestamp_ns = 0;
    std::uint64_t deadline_timestamp_ns = 0;
    bool armed = false;
    bool should_drop = false;
};

[[nodiscard]] inline std::string_view to_string(OutputState state) noexcept {
    switch (state) {
    case OutputState::Disabled:
        return "disabled";
    case OutputState::Bypass:
        return "bypass";
    case OutputState::Warmup:
        return "warmup";
    case OutputState::Active2x:
        return "active-2x";
    case OutputState::Degraded:
        return "degraded";
    case OutputState::ProtectedBypass:
        return "protected-bypass";
    case OutputState::Faulted:
        return "faulted";
    }

    return "unknown";
}

[[nodiscard]] inline std::string_view to_string(BypassReason reason) noexcept {
    switch (reason) {
    case BypassReason::None:
        return "none";
    case BypassReason::Disabled:
        return "disabled";
    case BypassReason::ProtectedContent:
        return "protected-content";
    case BypassReason::HookUnavailable:
        return "hook-unavailable";
    case BypassReason::UnsupportedOutput:
        return "unsupported-output";
    case BypassReason::GpuPathNotReady:
        return "gpu-path-not-ready";
    case BypassReason::Fault:
        return "fault";
    }

    return "unknown";
}

[[nodiscard]] inline std::string_view to_string(PresentationMode mode) noexcept {
    switch (mode) {
    case PresentationMode::VSync:
        return "vsync";
    case PresentationMode::AdaptiveSync:
        return "adaptive-sync";
    case PresentationMode::Async:
        return "async";
    case PresentationMode::AdaptiveAsync:
        return "adaptive-async";
    }

    return "unknown";
}

[[nodiscard]] inline std::string_view to_string(ContentType content_type) noexcept {
    switch (content_type) {
    case ContentType::None:
        return "none";
    case ContentType::Photo:
        return "photo";
    case ContentType::Video:
        return "video";
    case ContentType::Game:
        return "game";
    }

    return "unknown";
}

[[nodiscard]] inline std::string_view to_string(CadenceStatus cadence_status) noexcept {
    switch (cadence_status) {
    case CadenceStatus::Unknown:
        return "unknown";
    case CadenceStatus::Unstable:
        return "unstable";
    case CadenceStatus::Stable:
        return "stable";
    }

    return "unknown";
}

[[nodiscard]] inline std::string_view to_string(GovernorMode governor_mode) noexcept {
    switch (governor_mode) {
    case GovernorMode::Bypass:
        return "bypass";
    case GovernorMode::QualityLow:
        return "quality-low";
    case GovernorMode::QualityMedium:
        return "quality-medium";
    case GovernorMode::QualityHigh:
        return "quality-high";
    }

    return "unknown";
}

[[nodiscard]] inline std::string_view to_string(SchedulerMode scheduler_mode) noexcept {
    switch (scheduler_mode) {
    case SchedulerMode::PassthroughOnly:
        return "passthrough-only";
    case SchedulerMode::WarmupHold:
        return "warmup-hold";
    case SchedulerMode::Synthetic2x:
        return "synthetic-2x";
    }

    return "unknown";
}

[[nodiscard]] inline std::string to_bool_string(bool value) {
    return value ? "yes" : "no";
}

}  // namespace fluxma
