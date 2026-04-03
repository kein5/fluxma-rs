#include "fluxma_present_feedback_tap.h"

namespace fluxma {

PresentFeedback KfiPresentFeedbackTap::capture(PresentFeedback feedback) const noexcept {
    // TODO: Wire this to the confirmed KWin present feedback boundary.
    // Candidate handoff points are OutputFrame::presented(...) and RenderLoop::framePresented(...).
    return feedback;
}

}  // namespace fluxma
