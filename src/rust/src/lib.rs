use std::panic::{catch_unwind, AssertUnwindSafe};
use std::sync::atomic::{AtomicU64, Ordering};
use std::sync::Mutex;

#[repr(C)]
pub struct FluxmaRustConfig {
    enabled: u8,
    reserved: [u8; 7],
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum FluxmaRustOutputState {
    Disabled = 0,
    Bypass = 1,
    Warmup = 2,
    Active2x = 3,
    Degraded = 4,
    ProtectedBypass = 5,
    Faulted = 6,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum FluxmaRustBypassReason {
    None = 0,
    Disabled = 1,
    ProtectedContent = 2,
    HookUnavailable = 3,
    UnsupportedOutput = 4,
    GpuPathNotReady = 5,
    Fault = 6,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum FluxmaRustPresentationMode {
    VSync = 0,
    AdaptiveSync = 1,
    Async = 2,
    AdaptiveAsync = 3,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum FluxmaRustContentType {
    None = 0,
    Photo = 1,
    Video = 2,
    Game = 3,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct FluxmaRustGpuFrameHandle {
    backend_kind: u32,
    handle_id: u64,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct FluxmaRustFrameDescriptor {
    frame_id: u64,
    timestamp_ns: u64,
    target_presentation_timestamp_ns: u64,
    predicted_render_time_ns: u64,
    width: u32,
    height: u32,
    pixel_format: u32,
    color_space: u32,
    content_type: FluxmaRustContentType,
    protected_content: u8,
    reserved: [u8; 2],
    damage_ratio: f64,
    cursor_visible: u8,
    cursor_reserved: [u8; 7],
    cursor_x: f64,
    cursor_y: f64,
    cursor_velocity_x: f64,
    cursor_velocity_y: f64,
    gpu_handle: FluxmaRustGpuFrameHandle,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct FluxmaRustPresentFeedback {
    frame_id: u64,
    presented_timestamp_ns: u64,
    refresh_interval_ns: u64,
    presentation_mode: FluxmaRustPresentationMode,
    present_success: u8,
    dropped_synthetic: u8,
    reserved: [u8; 5],
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct FluxmaRustDecision {
    state: FluxmaRustOutputState,
    bypass_reason: FluxmaRustBypassReason,
    passthrough_only: u8,
    reserved: [u8; 7],
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct FluxmaRustMetricsSnapshot {
    state: FluxmaRustOutputState,
    bypass_reason: FluxmaRustBypassReason,
    protected_content: u8,
    passthrough_only: u8,
    reserved: [u8; 6],
    frame_tap_count: u64,
    present_feedback_count: u64,
    deadline_miss_count: u64,
    dropped_synthetic_count: u64,
    last_presented_frame_id: u64,
    last_presented_timestamp_ns: u64,
    refresh_interval_ns: u64,
    last_target_presentation_timestamp_ns: u64,
    last_predicted_render_time_ns: u64,
    last_presentation_mode: FluxmaRustPresentationMode,
    last_content_type: FluxmaRustContentType,
    reserved_tail: [u8; 6],
}

impl FluxmaRustDecision {
    fn bypass(state: FluxmaRustOutputState, reason: FluxmaRustBypassReason) -> Self {
        Self {
            state,
            bypass_reason: reason,
            passthrough_only: 1,
            reserved: [0; 7],
        }
    }
}

impl FluxmaRustMetricsSnapshot {
    fn faulted() -> Self {
        Self {
            state: FluxmaRustOutputState::Faulted,
            bypass_reason: FluxmaRustBypassReason::Fault,
            protected_content: 0,
            passthrough_only: 1,
            reserved: [0; 6],
            frame_tap_count: 0,
            present_feedback_count: 0,
            deadline_miss_count: 0,
            dropped_synthetic_count: 0,
            last_presented_frame_id: 0,
            last_presented_timestamp_ns: 0,
            refresh_interval_ns: 0,
            last_target_presentation_timestamp_ns: 0,
            last_predicted_render_time_ns: 0,
            last_presentation_mode: FluxmaRustPresentationMode::VSync,
            last_content_type: FluxmaRustContentType::None,
            reserved_tail: [0; 6],
        }
    }
}

#[derive(Clone, Copy, Debug)]
struct CoreState {
    last_state: FluxmaRustOutputState,
    last_bypass_reason: FluxmaRustBypassReason,
    last_protected_content: bool,
    last_passthrough_only: bool,
    last_presented_frame_id: u64,
    last_presented_timestamp_ns: u64,
    refresh_interval_ns: u64,
    last_target_presentation_timestamp_ns: u64,
    last_predicted_render_time_ns: u64,
    last_presentation_mode: FluxmaRustPresentationMode,
    last_content_type: FluxmaRustContentType,
}

impl Default for CoreState {
    fn default() -> Self {
        Self {
            last_state: FluxmaRustOutputState::Bypass,
            last_bypass_reason: FluxmaRustBypassReason::None,
            last_protected_content: false,
            last_passthrough_only: true,
            last_presented_frame_id: 0,
            last_presented_timestamp_ns: 0,
            refresh_interval_ns: 0,
            last_target_presentation_timestamp_ns: 0,
            last_predicted_render_time_ns: 0,
            last_presentation_mode: FluxmaRustPresentationMode::VSync,
            last_content_type: FluxmaRustContentType::None,
        }
    }
}

pub struct RustOutputCore {
    enabled: bool,
    frames_seen: AtomicU64,
    present_feedback_seen: AtomicU64,
    deadline_miss_count: AtomicU64,
    dropped_synthetic_count: AtomicU64,
    state: Mutex<CoreState>,
}

impl RustOutputCore {
    fn new(config: FluxmaRustConfig) -> Self {
        Self {
            enabled: config.enabled != 0,
            frames_seen: AtomicU64::new(0),
            present_feedback_seen: AtomicU64::new(0),
            deadline_miss_count: AtomicU64::new(0),
            dropped_synthetic_count: AtomicU64::new(0),
            state: Mutex::new(CoreState::default()),
        }
    }

    fn evaluate_frame(&self, frame: FluxmaRustFrameDescriptor) -> FluxmaRustDecision {
        self.frames_seen.fetch_add(1, Ordering::Relaxed);

        let decision = if !self.enabled {
            FluxmaRustDecision::bypass(
                FluxmaRustOutputState::Disabled,
                FluxmaRustBypassReason::Disabled,
            )
        } else if frame.protected_content != 0 {
            FluxmaRustDecision::bypass(
                FluxmaRustOutputState::ProtectedBypass,
                FluxmaRustBypassReason::ProtectedContent,
            )
        } else if frame.width == 0 || frame.height == 0 || frame.gpu_handle.handle_id == 0 {
            FluxmaRustDecision::bypass(
                FluxmaRustOutputState::Bypass,
                FluxmaRustBypassReason::UnsupportedOutput,
            )
        } else {
            FluxmaRustDecision::bypass(
                FluxmaRustOutputState::Bypass,
                FluxmaRustBypassReason::GpuPathNotReady,
            )
        };

        self.update_state_from_decision(decision, frame.protected_content != 0, frame);
        decision
    }

    fn note_present_feedback(&self, feedback: FluxmaRustPresentFeedback) {
        self.present_feedback_seen.fetch_add(1, Ordering::Relaxed);

        if feedback.present_success == 0 {
            self.deadline_miss_count.fetch_add(1, Ordering::Relaxed);
        }

        if feedback.dropped_synthetic != 0 {
            self.dropped_synthetic_count.fetch_add(1, Ordering::Relaxed);
        }

        let mut state = match self.state.lock() {
            Ok(state) => state,
            Err(poisoned) => poisoned.into_inner(),
        };
        state.last_presented_frame_id = feedback.frame_id;
        state.last_presented_timestamp_ns = feedback.presented_timestamp_ns;
        state.refresh_interval_ns = feedback.refresh_interval_ns;
        state.last_presentation_mode = feedback.presentation_mode;
    }

    fn snapshot_metrics(&self) -> FluxmaRustMetricsSnapshot {
        let state = match self.state.lock() {
            Ok(state) => state,
            Err(poisoned) => poisoned.into_inner(),
        };

        FluxmaRustMetricsSnapshot {
            state: state.last_state,
            bypass_reason: state.last_bypass_reason,
            protected_content: u8::from(state.last_protected_content),
            passthrough_only: u8::from(state.last_passthrough_only),
            reserved: [0; 6],
            frame_tap_count: self.frames_seen.load(Ordering::Relaxed),
            present_feedback_count: self.present_feedback_seen.load(Ordering::Relaxed),
            deadline_miss_count: self.deadline_miss_count.load(Ordering::Relaxed),
            dropped_synthetic_count: self.dropped_synthetic_count.load(Ordering::Relaxed),
            last_presented_frame_id: state.last_presented_frame_id,
            last_presented_timestamp_ns: state.last_presented_timestamp_ns,
            refresh_interval_ns: state.refresh_interval_ns,
            last_target_presentation_timestamp_ns: state.last_target_presentation_timestamp_ns,
            last_predicted_render_time_ns: state.last_predicted_render_time_ns,
            last_presentation_mode: state.last_presentation_mode,
            last_content_type: state.last_content_type,
            reserved_tail: [0; 6],
        }
    }

    fn ping(&self, value: u64) -> u64 {
        self.frames_seen.load(Ordering::Relaxed) ^ value
    }

    fn update_state_from_decision(
        &self,
        decision: FluxmaRustDecision,
        protected_content: bool,
        frame: FluxmaRustFrameDescriptor,
    ) {
        let mut state = match self.state.lock() {
            Ok(state) => state,
            Err(poisoned) => poisoned.into_inner(),
        };
        state.last_state = decision.state;
        state.last_bypass_reason = decision.bypass_reason;
        state.last_protected_content = protected_content;
        state.last_passthrough_only = decision.passthrough_only != 0;
        state.last_content_type = frame.content_type;
        state.last_target_presentation_timestamp_ns = frame.target_presentation_timestamp_ns;
        state.last_predicted_render_time_ns = frame.predicted_render_time_ns;
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn fluxma_rust_core_create(config: FluxmaRustConfig) -> *mut RustOutputCore {
    catch_unwind(AssertUnwindSafe(|| Box::into_raw(Box::new(RustOutputCore::new(config)))))
        .unwrap_or(std::ptr::null_mut())
}

#[unsafe(no_mangle)]
pub extern "C" fn fluxma_rust_core_destroy(core: *mut RustOutputCore) {
    let _ = catch_unwind(AssertUnwindSafe(|| {
        if core.is_null() {
            return;
        }

        // Safety: the pointer is allocated by fluxma_rust_core_create and owned by C++.
        unsafe {
            drop(Box::from_raw(core));
        }
    }));
}

#[unsafe(no_mangle)]
pub extern "C" fn fluxma_rust_core_evaluate_frame(
    core: *mut RustOutputCore,
    frame: FluxmaRustFrameDescriptor,
) -> FluxmaRustDecision {
    catch_unwind(AssertUnwindSafe(|| {
        if core.is_null() {
            return FluxmaRustDecision::bypass(
                FluxmaRustOutputState::Faulted,
                FluxmaRustBypassReason::Fault,
            );
        }

        // Safety: the pointer is owned by C++ and must remain valid during the call.
        unsafe { (&*core).evaluate_frame(frame) }
    }))
    .unwrap_or(FluxmaRustDecision::bypass(
        FluxmaRustOutputState::Faulted,
        FluxmaRustBypassReason::Fault,
    ))
}

#[unsafe(no_mangle)]
pub extern "C" fn fluxma_rust_core_ping(core: *mut RustOutputCore, value: u64) -> u64 {
    catch_unwind(AssertUnwindSafe(|| {
        if core.is_null() {
            return value;
        }

        // Safety: the pointer is owned by C++ and must remain valid during the call.
        unsafe { (&*core).ping(value) }
    }))
    .unwrap_or(value)
}

#[unsafe(no_mangle)]
pub extern "C" fn fluxma_rust_core_note_present_feedback(
    core: *mut RustOutputCore,
    feedback: FluxmaRustPresentFeedback,
) {
    let _ = catch_unwind(AssertUnwindSafe(|| {
        if core.is_null() {
            return;
        }

        // Safety: the pointer is owned by C++ and must remain valid during the call.
        unsafe {
            (&*core).note_present_feedback(feedback);
        }
    }));
}

#[unsafe(no_mangle)]
pub extern "C" fn fluxma_rust_core_snapshot_metrics(
    core: *mut RustOutputCore,
) -> FluxmaRustMetricsSnapshot {
    catch_unwind(AssertUnwindSafe(|| {
        if core.is_null() {
            return FluxmaRustMetricsSnapshot::faulted();
        }

        // Safety: the pointer is owned by C++ and must remain valid during the call.
        unsafe { (&*core).snapshot_metrics() }
    }))
    .unwrap_or(FluxmaRustMetricsSnapshot::faulted())
}

#[cfg(test)]
mod tests {
    use super::*;

    fn frame(protected_content: u8) -> FluxmaRustFrameDescriptor {
        FluxmaRustFrameDescriptor {
            frame_id: 1,
            timestamp_ns: 123,
            target_presentation_timestamp_ns: 16_666_667,
            predicted_render_time_ns: 2_000_000,
            width: 1920,
            height: 1080,
            pixel_format: 0,
            color_space: 0,
            content_type: FluxmaRustContentType::Video,
            protected_content,
            reserved: [0; 2],
            damage_ratio: 0.5,
            cursor_visible: 0,
            cursor_reserved: [0; 7],
            cursor_x: 0.0,
            cursor_y: 0.0,
            cursor_velocity_x: 0.0,
            cursor_velocity_y: 0.0,
            gpu_handle: FluxmaRustGpuFrameHandle {
                backend_kind: 0,
                handle_id: 42,
            },
        }
    }

    #[test]
    fn protected_content_forces_passthrough() {
        let core = RustOutputCore::new(FluxmaRustConfig {
            enabled: 1,
            reserved: [0; 7],
        });
        let decision = core.evaluate_frame(frame(1));
        assert_eq!(decision.state, FluxmaRustOutputState::ProtectedBypass);
        assert_eq!(
            decision.bypass_reason,
            FluxmaRustBypassReason::ProtectedContent
        );
        assert_eq!(decision.passthrough_only, 1);
    }

    #[test]
    fn enabled_but_unimplemented_core_stays_in_bypass() {
        let core = RustOutputCore::new(FluxmaRustConfig {
            enabled: 1,
            reserved: [0; 7],
        });
        let decision = core.evaluate_frame(frame(0));
        assert_eq!(decision.state, FluxmaRustOutputState::Bypass);
        assert_eq!(
            decision.bypass_reason,
            FluxmaRustBypassReason::GpuPathNotReady
        );
    }

    #[test]
    fn unsupported_frame_stays_in_bypass() {
        let core = RustOutputCore::new(FluxmaRustConfig {
            enabled: 1,
            reserved: [0; 7],
        });
        let mut descriptor = frame(0);
        descriptor.width = 0;
        descriptor.gpu_handle.handle_id = 0;
        let decision = core.evaluate_frame(descriptor);
        assert_eq!(decision.state, FluxmaRustOutputState::Bypass);
        assert_eq!(
            decision.bypass_reason,
            FluxmaRustBypassReason::UnsupportedOutput
        );
        assert_eq!(decision.passthrough_only, 1);
    }

    #[test]
    fn disabled_core_stays_passthrough_only() {
        let core = RustOutputCore::new(FluxmaRustConfig {
            enabled: 0,
            reserved: [0; 7],
        });
        let decision = core.evaluate_frame(frame(0));
        assert_eq!(decision.state, FluxmaRustOutputState::Disabled);
        assert_eq!(decision.bypass_reason, FluxmaRustBypassReason::Disabled);
        assert_eq!(decision.passthrough_only, 1);
    }

    #[test]
    fn present_feedback_updates_metrics() {
        let core = RustOutputCore::new(FluxmaRustConfig {
            enabled: 1,
            reserved: [0; 7],
        });
        let _ = core.evaluate_frame(frame(0));
        core.note_present_feedback(FluxmaRustPresentFeedback {
            frame_id: 1,
            presented_timestamp_ns: 999,
            refresh_interval_ns: 16_666_667,
            presentation_mode: FluxmaRustPresentationMode::VSync,
            present_success: 1,
            dropped_synthetic: 0,
            reserved: [0; 5],
        });
        let snapshot = core.snapshot_metrics();
        assert_eq!(snapshot.frame_tap_count, 1);
        assert_eq!(snapshot.present_feedback_count, 1);
        assert_eq!(snapshot.last_presented_frame_id, 1);
        assert_eq!(snapshot.refresh_interval_ns, 16_666_667);
        assert_eq!(snapshot.last_target_presentation_timestamp_ns, 16_666_667);
        assert_eq!(snapshot.last_predicted_render_time_ns, 2_000_000);
        assert_eq!(snapshot.last_presentation_mode, FluxmaRustPresentationMode::VSync);
        assert_eq!(snapshot.last_content_type, FluxmaRustContentType::Video);
        assert_eq!(snapshot.state, FluxmaRustOutputState::Bypass);
    }

    #[test]
    fn null_ffi_calls_return_fault_safe_values() {
        let decision = fluxma_rust_core_evaluate_frame(std::ptr::null_mut(), frame(0));
        assert_eq!(decision.state, FluxmaRustOutputState::Faulted);
        assert_eq!(decision.bypass_reason, FluxmaRustBypassReason::Fault);
        assert_eq!(decision.passthrough_only, 1);

        let snapshot = fluxma_rust_core_snapshot_metrics(std::ptr::null_mut());
        assert_eq!(snapshot.state, FluxmaRustOutputState::Faulted);
        assert_eq!(snapshot.bypass_reason, FluxmaRustBypassReason::Fault);
        assert_eq!(snapshot.passthrough_only, 1);
        assert_eq!(snapshot.last_presented_frame_id, 0);

        assert_eq!(fluxma_rust_core_ping(std::ptr::null_mut(), 55), 55);
        fluxma_rust_core_note_present_feedback(
            std::ptr::null_mut(),
            FluxmaRustPresentFeedback {
                frame_id: 1,
                presented_timestamp_ns: 0,
                refresh_interval_ns: 0,
                presentation_mode: FluxmaRustPresentationMode::VSync,
                present_success: 0,
                dropped_synthetic: 0,
                reserved: [0; 5],
            },
        );
    }
}
