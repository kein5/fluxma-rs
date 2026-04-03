#pragma once

#include "fluxma_types.h"

namespace fluxma {

struct FinalComposedFrameEvent;

class KfiOutputPolicy {
  public:
    explicit KfiOutputPolicy(std::uint32_t target_output_id);

    [[nodiscard]] bool accepts_output(std::uint32_t output_id) const noexcept;
    [[nodiscard]] bool supports_frame_event(
        const FinalComposedFrameEvent& event
    ) const noexcept;
    [[nodiscard]] OutputDecision classify_frame_event(
        const FinalComposedFrameEvent& event
    ) const noexcept;

  private:
    std::uint32_t target_output_id_ = 0;
};

}  // namespace fluxma
