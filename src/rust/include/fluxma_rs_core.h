#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FluxmaRustCore FluxmaRustCore;

typedef struct FluxmaRustConfig {
    uint8_t enabled;
    uint8_t reserved[7];
} FluxmaRustConfig;

typedef enum FluxmaRustOutputState {
    FLUXMA_RUST_OUTPUT_STATE_DISABLED = 0,
    FLUXMA_RUST_OUTPUT_STATE_BYPASS = 1,
    FLUXMA_RUST_OUTPUT_STATE_WARMUP = 2,
    FLUXMA_RUST_OUTPUT_STATE_ACTIVE_2X = 3,
    FLUXMA_RUST_OUTPUT_STATE_DEGRADED = 4,
    FLUXMA_RUST_OUTPUT_STATE_PROTECTED_BYPASS = 5,
    FLUXMA_RUST_OUTPUT_STATE_FAULTED = 6,
} FluxmaRustOutputState;

typedef enum FluxmaRustBypassReason {
    FLUXMA_RUST_BYPASS_REASON_NONE = 0,
    FLUXMA_RUST_BYPASS_REASON_DISABLED = 1,
    FLUXMA_RUST_BYPASS_REASON_PROTECTED_CONTENT = 2,
    FLUXMA_RUST_BYPASS_REASON_HOOK_UNAVAILABLE = 3,
    FLUXMA_RUST_BYPASS_REASON_UNSUPPORTED_OUTPUT = 4,
    FLUXMA_RUST_BYPASS_REASON_GPU_PATH_NOT_READY = 5,
    FLUXMA_RUST_BYPASS_REASON_FAULT = 6,
} FluxmaRustBypassReason;

typedef enum FluxmaRustPresentationMode {
    FLUXMA_RUST_PRESENTATION_MODE_VSYNC = 0,
    FLUXMA_RUST_PRESENTATION_MODE_ADAPTIVE_SYNC = 1,
    FLUXMA_RUST_PRESENTATION_MODE_ASYNC = 2,
    FLUXMA_RUST_PRESENTATION_MODE_ADAPTIVE_ASYNC = 3,
} FluxmaRustPresentationMode;

typedef enum FluxmaRustContentType {
    FLUXMA_RUST_CONTENT_TYPE_NONE = 0,
    FLUXMA_RUST_CONTENT_TYPE_PHOTO = 1,
    FLUXMA_RUST_CONTENT_TYPE_VIDEO = 2,
    FLUXMA_RUST_CONTENT_TYPE_GAME = 3,
} FluxmaRustContentType;

typedef enum FluxmaRustCadenceStatus {
    FLUXMA_RUST_CADENCE_STATUS_UNKNOWN = 0,
    FLUXMA_RUST_CADENCE_STATUS_UNSTABLE = 1,
    FLUXMA_RUST_CADENCE_STATUS_STABLE = 2,
} FluxmaRustCadenceStatus;

typedef enum FluxmaRustGovernorMode {
    FLUXMA_RUST_GOVERNOR_MODE_BYPASS = 0,
    FLUXMA_RUST_GOVERNOR_MODE_QUALITY_LOW = 1,
    FLUXMA_RUST_GOVERNOR_MODE_QUALITY_MEDIUM = 2,
    FLUXMA_RUST_GOVERNOR_MODE_QUALITY_HIGH = 3,
} FluxmaRustGovernorMode;

typedef enum FluxmaRustSchedulerMode {
    FLUXMA_RUST_SCHEDULER_MODE_PASSTHROUGH_ONLY = 0,
    FLUXMA_RUST_SCHEDULER_MODE_WARMUP_HOLD = 1,
    FLUXMA_RUST_SCHEDULER_MODE_SYNTHETIC_2X = 2,
} FluxmaRustSchedulerMode;

typedef struct FluxmaRustGpuFrameHandle {
    uint32_t backend_kind;
    uint64_t handle_id;
} FluxmaRustGpuFrameHandle;

typedef struct FluxmaRustFrameDescriptor {
    uint64_t frame_id;
    uint64_t timestamp_ns;
    uint64_t target_presentation_timestamp_ns;
    uint64_t predicted_render_time_ns;
    uint32_t width;
    uint32_t height;
    uint32_t pixel_format;
    uint32_t color_space;
    FluxmaRustContentType content_type;
    uint8_t protected_content;
    uint8_t reserved[2];
    double damage_ratio;
    uint8_t cursor_visible;
    uint8_t cursor_reserved[7];
    double cursor_x;
    double cursor_y;
    double cursor_velocity_x;
    double cursor_velocity_y;
    FluxmaRustGpuFrameHandle gpu_handle;
} FluxmaRustFrameDescriptor;

typedef struct FluxmaRustPresentFeedback {
    uint64_t frame_id;
    uint64_t presented_timestamp_ns;
    uint64_t refresh_interval_ns;
    FluxmaRustPresentationMode presentation_mode;
    uint8_t present_success;
    uint8_t dropped_synthetic;
    uint8_t reserved[5];
} FluxmaRustPresentFeedback;

typedef struct FluxmaRustDecision {
    FluxmaRustOutputState state;
    FluxmaRustBypassReason bypass_reason;
    uint8_t passthrough_only;
    uint8_t reserved[7];
} FluxmaRustDecision;

typedef struct FluxmaRustMetricsSnapshot {
    FluxmaRustOutputState state;
    FluxmaRustBypassReason bypass_reason;
    uint8_t protected_content;
    uint8_t passthrough_only;
    uint8_t classifier_allows_interpolation;
    uint8_t reserved[5];
    uint64_t frame_tap_count;
    uint64_t present_feedback_count;
    uint64_t deadline_miss_count;
    uint64_t dropped_synthetic_count;
    uint64_t last_presented_frame_id;
    uint64_t last_presented_timestamp_ns;
    uint64_t refresh_interval_ns;
    uint64_t last_target_presentation_timestamp_ns;
    uint64_t last_predicted_render_time_ns;
    FluxmaRustPresentationMode last_presentation_mode;
    FluxmaRustContentType last_content_type;
    FluxmaRustCadenceStatus cadence_status;
    FluxmaRustGovernorMode governor_mode;
    FluxmaRustSchedulerMode scheduler_mode;
    uint8_t reserved_tail[5];
    uint32_t cadence_hz_millihz;
    uint64_t state_transition_count;
} FluxmaRustMetricsSnapshot;

FluxmaRustCore* fluxma_rust_core_create(FluxmaRustConfig config);
void fluxma_rust_core_destroy(FluxmaRustCore* core);
FluxmaRustDecision fluxma_rust_core_evaluate_frame(
    FluxmaRustCore* core,
    FluxmaRustFrameDescriptor frame
);
void fluxma_rust_core_note_present_feedback(
    FluxmaRustCore* core,
    FluxmaRustPresentFeedback feedback
);
FluxmaRustMetricsSnapshot fluxma_rust_core_snapshot_metrics(FluxmaRustCore* core);
uint64_t fluxma_rust_core_ping(FluxmaRustCore* core, uint64_t value);

#ifdef __cplusplus
}
#endif
