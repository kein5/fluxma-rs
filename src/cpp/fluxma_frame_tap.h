#pragma once

#include "fluxma_types.h"

namespace fluxma {

class KfiFrameTap {
  public:
    [[nodiscard]] FrameDescriptor capture(FrameDescriptor frame) const noexcept;
};

}  // namespace fluxma
