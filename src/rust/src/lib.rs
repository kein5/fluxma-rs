use std::panic::{catch_unwind, AssertUnwindSafe};
use std::sync::atomic::{AtomicU64, Ordering};
use std::sync::Mutex;

const CADENCE_HISTORY_LEN: usize = 4;
const NOMINAL_CADENCE_MILLIHZ: [u32; 5] = [24_000, 25_000, 30_000, 50_000, 60_000];

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
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum FluxmaRustCadenceStatus {
    Unknown = 0,
    Unstable = 1,
    Stable = 2,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum FluxmaRustGovernorMode {
    Bypass = 0,
    QualityLow = 1,
    QualityMedium = 2,
    QualityHigh = 3,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum FluxmaRustSchedulerMode {
    PassthroughOnly = 0,
    WarmupHold = 1,
    Synthetic2x = 2,
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
    interpolation_armed: u8,
    reserved: [u8; 6],
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct FluxmaRustMetricsSnapshot {
    state: FluxmaRustOutputState,
    bypass_reason: FluxmaRustBypassReason,
    protected_content: u8,
    passthrough_only: u8,
    classifier_allows_interpolation: u8,
    reserved: [u8; 5],
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
    cadence_status: FluxmaRustCadenceStatus,
    governor_mode: FluxmaRustGovernorMode,
    scheduler_mode: FluxmaRustSchedulerMode,
    reserved_tail: [u8; 5],
    cadence_hz_millihz: u32,
    state_transition_count: u64,
}

impl FluxmaRustDecision {
    fn bypass(state: FluxmaRustOutputState, reason: FluxmaRustBypassReason) -> Self {
        Self {
            state,
            bypass_reason: reason,
            passthrough_only: 1,
            interpolation_armed: 0,
            reserved: [0; 6],
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
            classifier_allows_interpolation: 0,
            reserved: [0; 5],
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
            cadence_status: FluxmaRustCadenceStatus::Unknown,
            governor_mode: FluxmaRustGovernorMode::Bypass,
            scheduler_mode: FluxmaRustSchedulerMode::PassthroughOnly,
            reserved_tail: [0; 5],
            cadence_hz_millihz: 0,
            state_transition_count: 0,
        }
    }
}

#[derive(Clone, Copy, Debug)]
struct CadenceEstimator {
    last_frame_timestamp_ns: u64,
    deltas_ns: [u64; CADENCE_HISTORY_LEN],
    delta_count: usize,
    cadence_hz_millihz: u32,
    status: FluxmaRustCadenceStatus,
    stable_streak: u32,
}

impl Default for CadenceEstimator {
    fn default() -> Self {
        Self {
            last_frame_timestamp_ns: 0,
            deltas_ns: [0; CADENCE_HISTORY_LEN],
            delta_count: 0,
            cadence_hz_millihz: 0,
            status: FluxmaRustCadenceStatus::Unknown,
            stable_streak: 0,
        }
    }
}

impl CadenceEstimator {
    fn note_frame(&mut self, timestamp_ns: u64) {
        if self.last_frame_timestamp_ns == 0 || timestamp_ns <= self.last_frame_timestamp_ns {
            self.last_frame_timestamp_ns = timestamp_ns;
            return;
        }

        let delta_ns = timestamp_ns - self.last_frame_timestamp_ns;
        self.last_frame_timestamp_ns = timestamp_ns;
        if delta_ns == 0 {
            return;
        }

        if self.delta_count < self.deltas_ns.len() {
            self.deltas_ns[self.delta_count] = delta_ns;
            self.delta_count += 1;
        } else {
            self.deltas_ns.rotate_left(1);
            self.deltas_ns[self.deltas_ns.len() - 1] = delta_ns;
        }

        self.recompute();
    }

    fn recompute(&mut self) {
        if self.delta_count < 3 {
            self.cadence_hz_millihz = 0;
            self.status = FluxmaRustCadenceStatus::Unknown;
            self.stable_streak = 0;
            return;
        }

        let window = &self.deltas_ns[..self.delta_count];
        let sum: u64 = window.iter().copied().sum();
        let avg_delta_ns = sum / self.delta_count as u64;
        if avg_delta_ns == 0 {
            self.cadence_hz_millihz = 0;
            self.status = FluxmaRustCadenceStatus::Unknown;
            self.stable_streak = 0;
            return;
        }

        let estimated_millihz = (1_000_000_000_000u64 / avg_delta_ns) as u32;
        let snapped_millihz = snap_nominal_cadence_millihz(estimated_millihz);
        let max_delta_ns = window.iter().copied().max().unwrap_or(avg_delta_ns);
        let min_delta_ns = window.iter().copied().min().unwrap_or(avg_delta_ns);
        let jitter_ns = max_delta_ns.saturating_sub(min_delta_ns);
        let stable = snapped_millihz != 0 && jitter_ns <= avg_delta_ns / 12;

        self.cadence_hz_millihz = if snapped_millihz != 0 {
            snapped_millihz
        } else {
            estimated_millihz
        };
        self.status = if stable {
            FluxmaRustCadenceStatus::Stable
        } else {
            FluxmaRustCadenceStatus::Unstable
        };
        self.stable_streak = if stable {
            self.stable_streak.saturating_add(1)
        } else {
            0
        };
    }
}

#[derive(Clone, Copy, Debug)]
struct ClassifierState {
    allows_interpolation: bool,
}

impl Default for ClassifierState {
    fn default() -> Self {
        Self {
            allows_interpolation: false,
        }
    }
}

#[derive(Clone, Copy, Debug)]
struct GovernorState {
    mode: FluxmaRustGovernorMode,
}

impl Default for GovernorState {
    fn default() -> Self {
        Self {
            mode: FluxmaRustGovernorMode::Bypass,
        }
    }
}

#[derive(Clone, Copy, Debug)]
struct SchedulerState {
    mode: FluxmaRustSchedulerMode,
}

impl Default for SchedulerState {
    fn default() -> Self {
        Self {
            mode: FluxmaRustSchedulerMode::PassthroughOnly,
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
    cadence_status: FluxmaRustCadenceStatus,
    cadence_hz_millihz: u32,
    classifier_allows_interpolation: bool,
    governor_mode: FluxmaRustGovernorMode,
    scheduler_mode: FluxmaRustSchedulerMode,
    state_transition_count: u64,
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
            cadence_status: FluxmaRustCadenceStatus::Unknown,
            cadence_hz_millihz: 0,
            classifier_allows_interpolation: false,
            governor_mode: FluxmaRustGovernorMode::Bypass,
            scheduler_mode: FluxmaRustSchedulerMode::PassthroughOnly,
            state_transition_count: 0,
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
    cadence: Mutex<CadenceEstimator>,
    classifier: Mutex<ClassifierState>,
    governor: Mutex<GovernorState>,
    scheduler: Mutex<SchedulerState>,
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
            cadence: Mutex::new(CadenceEstimator::default()),
            classifier: Mutex::new(ClassifierState::default()),
            governor: Mutex::new(GovernorState::default()),
            scheduler: Mutex::new(SchedulerState::default()),
        }
    }

    fn evaluate_frame(&self, frame: FluxmaRustFrameDescriptor) -> FluxmaRustDecision {
        self.frames_seen.fetch_add(1, Ordering::Relaxed);

        let cadence_snapshot = {
            let mut cadence = match self.cadence.lock() {
                Ok(cadence) => cadence,
                Err(poisoned) => poisoned.into_inner(),
            };
            cadence.note_frame(frame.timestamp_ns);
            *cadence
        };

        let decision = if !self.enabled {
            self.reset_runtime_controls_for_bypass();
            FluxmaRustDecision::bypass(
                FluxmaRustOutputState::Disabled,
                FluxmaRustBypassReason::Disabled,
            )
        } else if frame.protected_content != 0 {
            self.reset_runtime_controls_for_bypass();
            FluxmaRustDecision::bypass(
                FluxmaRustOutputState::ProtectedBypass,
                FluxmaRustBypassReason::ProtectedContent,
            )
        } else if frame.width == 0 || frame.height == 0 || frame.gpu_handle.handle_id == 0 {
            self.reset_runtime_controls_for_bypass();
            FluxmaRustDecision::bypass(
                FluxmaRustOutputState::Bypass,
                FluxmaRustBypassReason::UnsupportedOutput,
            )
        } else {
            self.evaluate_runtime_state(frame, cadence_snapshot)
        };

        self.update_state_from_decision(
            decision,
            frame.protected_content != 0,
            frame,
            cadence_snapshot,
        );
        decision
    }

    fn evaluate_runtime_state(
        &self,
        frame: FluxmaRustFrameDescriptor,
        cadence: CadenceEstimator,
    ) -> FluxmaRustDecision {
        let allows_interpolation =
            cadence.status == FluxmaRustCadenceStatus::Stable &&
            matches!(frame.content_type, FluxmaRustContentType::Video) &&
            frame.damage_ratio >= 0.05 &&
            cursor_speed(frame) < 800.0;
        {
            let mut classifier = match self.classifier.lock() {
                Ok(classifier) => classifier,
                Err(poisoned) => poisoned.into_inner(),
            };
            classifier.allows_interpolation = allows_interpolation;
        }

        let deadline_misses = self.deadline_miss_count.load(Ordering::Relaxed);
        let dropped_synthetic = self.dropped_synthetic_count.load(Ordering::Relaxed);
        let governor_mode = if deadline_misses >= 3 || dropped_synthetic >= 2 {
            FluxmaRustGovernorMode::QualityLow
        } else if deadline_misses >= 1 {
            FluxmaRustGovernorMode::QualityMedium
        } else if allows_interpolation {
            FluxmaRustGovernorMode::QualityHigh
        } else {
            FluxmaRustGovernorMode::Bypass
        };
        {
            let mut governor = match self.governor.lock() {
                Ok(governor) => governor,
                Err(poisoned) => poisoned.into_inner(),
            };
            governor.mode = governor_mode;
        }

        let scheduler_mode = if !allows_interpolation {
            FluxmaRustSchedulerMode::PassthroughOnly
        } else if cadence.stable_streak < 2 {
            FluxmaRustSchedulerMode::WarmupHold
        } else {
            FluxmaRustSchedulerMode::Synthetic2x
        };
        {
            let mut scheduler = match self.scheduler.lock() {
                Ok(scheduler) => scheduler,
                Err(poisoned) => poisoned.into_inner(),
            };
            scheduler.mode = scheduler_mode;
        }

        let state = if !allows_interpolation {
            FluxmaRustOutputState::Bypass
        } else if governor_mode == FluxmaRustGovernorMode::QualityLow {
            FluxmaRustOutputState::Degraded
        } else if scheduler_mode == FluxmaRustSchedulerMode::WarmupHold {
            FluxmaRustOutputState::Warmup
        } else {
            FluxmaRustOutputState::Active2x
        };
        let bypass_reason = if state == FluxmaRustOutputState::Bypass {
            FluxmaRustBypassReason::GpuPathNotReady
        } else {
            FluxmaRustBypassReason::None
        };

        FluxmaRustDecision {
            state,
            bypass_reason,
            passthrough_only: 1,
            interpolation_armed: u8::from(
                scheduler_mode == FluxmaRustSchedulerMode::Synthetic2x &&
                    state != FluxmaRustOutputState::Bypass
            ),
            reserved: [0; 6],
        }
    }

    fn reset_runtime_controls_for_bypass(&self) {
        {
            let mut classifier = match self.classifier.lock() {
                Ok(classifier) => classifier,
                Err(poisoned) => poisoned.into_inner(),
            };
            classifier.allows_interpolation = false;
        }
        {
            let mut governor = match self.governor.lock() {
                Ok(governor) => governor,
                Err(poisoned) => poisoned.into_inner(),
            };
            governor.mode = FluxmaRustGovernorMode::Bypass;
        }
        {
            let mut scheduler = match self.scheduler.lock() {
                Ok(scheduler) => scheduler,
                Err(poisoned) => poisoned.into_inner(),
            };
            scheduler.mode = FluxmaRustSchedulerMode::PassthroughOnly;
        }
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
            classifier_allows_interpolation: u8::from(state.classifier_allows_interpolation),
            reserved: [0; 5],
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
            cadence_status: state.cadence_status,
            governor_mode: state.governor_mode,
            scheduler_mode: state.scheduler_mode,
            reserved_tail: [0; 5],
            cadence_hz_millihz: state.cadence_hz_millihz,
            state_transition_count: state.state_transition_count,
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
        cadence_snapshot: CadenceEstimator,
    ) {
        let classifier = match self.classifier.lock() {
            Ok(classifier) => classifier,
            Err(poisoned) => poisoned.into_inner(),
        };
        let governor = match self.governor.lock() {
            Ok(governor) => governor,
            Err(poisoned) => poisoned.into_inner(),
        };
        let scheduler = match self.scheduler.lock() {
            Ok(scheduler) => scheduler,
            Err(poisoned) => poisoned.into_inner(),
        };
        let mut state = match self.state.lock() {
            Ok(state) => state,
            Err(poisoned) => poisoned.into_inner(),
        };
        if state.last_state != decision.state || state.last_bypass_reason != decision.bypass_reason
        {
            state.state_transition_count = state.state_transition_count.saturating_add(1);
        }
        state.last_state = decision.state;
        state.last_bypass_reason = decision.bypass_reason;
        state.last_protected_content = protected_content;
        state.last_passthrough_only = decision.passthrough_only != 0;
        state.last_content_type = frame.content_type;
        state.last_target_presentation_timestamp_ns = frame.target_presentation_timestamp_ns;
        state.last_predicted_render_time_ns = frame.predicted_render_time_ns;
        state.cadence_status = cadence_snapshot.status;
        state.cadence_hz_millihz = cadence_snapshot.cadence_hz_millihz;
        state.classifier_allows_interpolation = classifier.allows_interpolation;
        state.governor_mode = governor.mode;
        state.scheduler_mode = scheduler.mode;
    }
}

fn snap_nominal_cadence_millihz(estimated_millihz: u32) -> u32 {
    let mut best = 0;
    let mut best_delta = u32::MAX;
    for nominal in NOMINAL_CADENCE_MILLIHZ {
        let delta = nominal.abs_diff(estimated_millihz);
        if delta < best_delta {
            best = nominal;
            best_delta = delta;
        }
    }

    if best == 0 || best_delta > best / 14 {
        0
    } else {
        best
    }
}

fn cursor_speed(frame: FluxmaRustFrameDescriptor) -> f64 {
    (frame.cursor_velocity_x * frame.cursor_velocity_x +
        frame.cursor_velocity_y * frame.cursor_velocity_y)
        .sqrt()
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
    use std::mem::{align_of, size_of, MaybeUninit};
    use std::ptr::addr_of;

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
        assert_eq!(decision.interpolation_armed, 0);
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
        assert_eq!(decision.interpolation_armed, 0);
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
        assert_eq!(decision.interpolation_armed, 0);
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
        assert_eq!(decision.interpolation_armed, 0);
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
        assert_eq!(snapshot.cadence_status, FluxmaRustCadenceStatus::Unknown);
        assert_eq!(snapshot.governor_mode, FluxmaRustGovernorMode::Bypass);
        assert_eq!(
            snapshot.scheduler_mode,
            FluxmaRustSchedulerMode::PassthroughOnly
        );
    }

    #[test]
    fn stable_video_cadence_reaches_active_2x_skeleton_state() {
        let core = RustOutputCore::new(FluxmaRustConfig {
            enabled: 1,
            reserved: [0; 7],
        });

        for index in 0..5u64 {
            let mut descriptor = frame(0);
            descriptor.frame_id = index + 1;
            descriptor.timestamp_ns = 33_333_333 * (index + 1);
            let decision = core.evaluate_frame(descriptor);
            if index < 3 {
                assert_eq!(decision.state, FluxmaRustOutputState::Bypass);
            } else if index == 3 {
                assert_eq!(decision.state, FluxmaRustOutputState::Warmup);
            } else {
                assert_eq!(decision.state, FluxmaRustOutputState::Active2x);
                assert_eq!(decision.bypass_reason, FluxmaRustBypassReason::None);
                assert_eq!(decision.interpolation_armed, 1);
            }
        }

        let snapshot = core.snapshot_metrics();
        assert_eq!(snapshot.state, FluxmaRustOutputState::Active2x);
        assert_eq!(snapshot.cadence_status, FluxmaRustCadenceStatus::Stable);
        assert_eq!(snapshot.cadence_hz_millihz, 30_000);
        assert_eq!(snapshot.classifier_allows_interpolation, 1);
        assert_eq!(snapshot.governor_mode, FluxmaRustGovernorMode::QualityHigh);
        assert_eq!(
            snapshot.scheduler_mode,
            FluxmaRustSchedulerMode::Synthetic2x
        );
        assert!(snapshot.state_transition_count >= 2);
    }

    #[test]
    fn deadline_pressure_moves_skeleton_state_to_degraded() {
        let core = RustOutputCore::new(FluxmaRustConfig {
            enabled: 1,
            reserved: [0; 7],
        });

        for index in 0..5u64 {
            let mut descriptor = frame(0);
            descriptor.frame_id = index + 1;
            descriptor.timestamp_ns = 33_333_333 * (index + 1);
            let _ = core.evaluate_frame(descriptor);
        }

        for index in 0..3u64 {
            core.note_present_feedback(FluxmaRustPresentFeedback {
                frame_id: index + 1,
                presented_timestamp_ns: 33_333_333 * (index + 1),
                refresh_interval_ns: 16_666_667,
                presentation_mode: FluxmaRustPresentationMode::VSync,
                present_success: 0,
                dropped_synthetic: 0,
                reserved: [0; 5],
            });
        }

        let mut descriptor = frame(0);
        descriptor.frame_id = 6;
        descriptor.timestamp_ns = 33_333_333 * 6;
        let decision = core.evaluate_frame(descriptor);
        let snapshot = core.snapshot_metrics();
        assert_eq!(decision.state, FluxmaRustOutputState::Degraded);
        assert_eq!(decision.bypass_reason, FluxmaRustBypassReason::None);
        assert_eq!(decision.interpolation_armed, 1);
        assert_eq!(snapshot.governor_mode, FluxmaRustGovernorMode::QualityLow);
        assert_eq!(
            snapshot.scheduler_mode,
            FluxmaRustSchedulerMode::Synthetic2x
        );
        assert_eq!(snapshot.deadline_miss_count, 3);
    }

    #[test]
    fn null_ffi_calls_return_fault_safe_values() {
        let decision = fluxma_rust_core_evaluate_frame(std::ptr::null_mut(), frame(0));
        assert_eq!(decision.state, FluxmaRustOutputState::Faulted);
        assert_eq!(decision.bypass_reason, FluxmaRustBypassReason::Fault);
        assert_eq!(decision.passthrough_only, 1);
        assert_eq!(decision.interpolation_armed, 0);

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

    #[test]
    fn cadence_estimator_snaps_24fps() {
        let core = RustOutputCore::new(FluxmaRustConfig {
            enabled: 1,
            reserved: [0; 7],
        });

        for index in 0..5u64 {
            let mut descriptor = frame(0);
            descriptor.frame_id = index + 1;
            descriptor.timestamp_ns = 41_666_667 * (index + 1);
            let _ = core.evaluate_frame(descriptor);
        }

        let snapshot = core.snapshot_metrics();
        assert_eq!(snapshot.cadence_status, FluxmaRustCadenceStatus::Stable);
        assert_eq!(snapshot.cadence_hz_millihz, 24_000);
    }

    #[test]
    fn out_of_order_timestamps_do_not_fake_stable_cadence() {
        let core = RustOutputCore::new(FluxmaRustConfig {
            enabled: 1,
            reserved: [0; 7],
        });

        let timestamps = [33_333_333u64, 66_666_666, 50_000_000, 100_000_000, 133_333_333];
        for (index, timestamp_ns) in timestamps.into_iter().enumerate() {
            let mut descriptor = frame(0);
            descriptor.frame_id = (index + 1) as u64;
            descriptor.timestamp_ns = timestamp_ns;
            let _ = core.evaluate_frame(descriptor);
        }

        let snapshot = core.snapshot_metrics();
        assert_ne!(snapshot.state, FluxmaRustOutputState::Active2x);
        assert_ne!(snapshot.cadence_status, FluxmaRustCadenceStatus::Stable);
    }

    #[test]
    fn protected_frame_clears_active_control_diagnostics() {
        let core = RustOutputCore::new(FluxmaRustConfig {
            enabled: 1,
            reserved: [0; 7],
        });

        for index in 0..5u64 {
            let mut descriptor = frame(0);
            descriptor.frame_id = index + 1;
            descriptor.timestamp_ns = 33_333_333 * (index + 1);
            let _ = core.evaluate_frame(descriptor);
        }

        let protected = core.evaluate_frame(frame(1));
        let snapshot = core.snapshot_metrics();
        assert_eq!(protected.state, FluxmaRustOutputState::ProtectedBypass);
        assert_eq!(protected.interpolation_armed, 0);
        assert_eq!(snapshot.cadence_status, FluxmaRustCadenceStatus::Stable);
        assert_eq!(snapshot.classifier_allows_interpolation, 0);
        assert_eq!(snapshot.governor_mode, FluxmaRustGovernorMode::Bypass);
        assert_eq!(
            snapshot.scheduler_mode,
            FluxmaRustSchedulerMode::PassthroughOnly
        );
    }

    #[test]
    fn ffi_layout_stays_stable() {
        assert_eq!(size_of::<FluxmaRustConfig>(), 8);
        assert_eq!(align_of::<FluxmaRustConfig>(), 1);
        assert_eq!(size_of::<FluxmaRustDecision>(), 16);
        assert_eq!(align_of::<FluxmaRustDecision>(), 4);
        assert_eq!(size_of::<FluxmaRustPresentFeedback>(), 40);
        assert_eq!(align_of::<FluxmaRustPresentFeedback>(), 8);
        assert_eq!(size_of::<FluxmaRustFrameDescriptor>(), 120);
        assert_eq!(align_of::<FluxmaRustFrameDescriptor>(), 8);
        assert_eq!(size_of::<FluxmaRustMetricsSnapshot>(), 128);
        assert_eq!(align_of::<FluxmaRustMetricsSnapshot>(), 8);

        let frame = MaybeUninit::<FluxmaRustFrameDescriptor>::uninit();
        let frame_ptr = frame.as_ptr();
        // Safety: addr_of does not dereference the MaybeUninit payload.
        assert_eq!(unsafe { addr_of!((*frame_ptr).gpu_handle) as usize - frame_ptr as usize }, 104);

        let feedback = MaybeUninit::<FluxmaRustPresentFeedback>::uninit();
        let feedback_ptr = feedback.as_ptr();
        // Safety: addr_of does not dereference the MaybeUninit payload.
        assert_eq!(
            unsafe { addr_of!((*feedback_ptr).present_success) as usize - feedback_ptr as usize },
            28
        );

        let metrics = MaybeUninit::<FluxmaRustMetricsSnapshot>::uninit();
        let metrics_ptr = metrics.as_ptr();
        // Safety: addr_of does not dereference the MaybeUninit payload.
        assert_eq!(
            unsafe { addr_of!((*metrics_ptr).frame_tap_count) as usize - metrics_ptr as usize },
            16
        );
        assert_eq!(
            unsafe { addr_of!((*metrics_ptr).cadence_hz_millihz) as usize - metrics_ptr as usize },
            116
        );
        assert_eq!(
            unsafe { addr_of!((*metrics_ptr).state_transition_count) as usize - metrics_ptr as usize },
            120
        );
    }
}
