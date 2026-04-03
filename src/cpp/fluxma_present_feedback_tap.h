#pragma once

#include "fluxma_types.h"

namespace fluxma {

class KfiPresentFeedbackTap {
  public:
    [[nodiscard]] PresentFeedback capture(PresentFeedback feedback) const noexcept;
};

}  // namespace fluxma
