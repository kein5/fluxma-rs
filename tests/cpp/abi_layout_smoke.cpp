#include <cstddef>
#include <cstdlib>

#include "fluxma_rs_core.h"

static_assert(sizeof(FluxmaRustConfig) == 8);
static_assert(alignof(FluxmaRustConfig) == 1);
static_assert(sizeof(FluxmaRustDecision) == 16);
static_assert(alignof(FluxmaRustDecision) == 4);
static_assert(sizeof(FluxmaRustPresentFeedback) == 40);
static_assert(alignof(FluxmaRustPresentFeedback) == 8);
static_assert(offsetof(FluxmaRustPresentFeedback, present_success) == 28);
static_assert(sizeof(FluxmaRustFrameDescriptor) == 120);
static_assert(alignof(FluxmaRustFrameDescriptor) == 8);
static_assert(offsetof(FluxmaRustFrameDescriptor, gpu_handle) == 104);
static_assert(sizeof(FluxmaRustMetricsSnapshot) == 128);
static_assert(alignof(FluxmaRustMetricsSnapshot) == 8);
static_assert(offsetof(FluxmaRustMetricsSnapshot, frame_tap_count) == 16);
static_assert(offsetof(FluxmaRustMetricsSnapshot, cadence_hz_millihz) == 116);
static_assert(offsetof(FluxmaRustMetricsSnapshot, state_transition_count) == 120);

int main() {
    return EXIT_SUCCESS;
}
