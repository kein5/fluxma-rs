#include "fluxma_frame_tap.h"

namespace fluxma {

FrameDescriptor KfiFrameTap::capture(FrameDescriptor frame) const noexcept {
    // TODO: Wire this to the confirmed KWin final per-output post-composition hook.
    return frame;
}

}  // namespace fluxma
