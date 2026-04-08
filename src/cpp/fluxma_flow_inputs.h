#pragma once

#include <cstdint>

#include "fluxma_luma_pyramid.h"

namespace fluxma {

struct FlowFrameResources {
    std::uint64_t frame_id = 0;
    GpuTextureLease source_texture {};
    LumaPyramidBuildResult luma_pyramid {};
    bool valid = false;
    bool truncated = false;
    bool placeholder_only = true;

    [[nodiscard]] bool has_live_resources() const noexcept {
        return valid;
    }
};

struct FlowInputBundle {
    FlowFrameResources previous {};
    FlowFrameResources current {};
    bool valid = false;
    bool truncated = false;
    bool placeholder_only = true;

    [[nodiscard]] bool is_usable() const noexcept {
        return valid && !truncated;
    }

    [[nodiscard]] bool has_live_resources() const noexcept {
        return previous.has_live_resources() || current.has_live_resources();
    }
};

class KfiFlowInputsBuilder {
  public:
    explicit KfiFlowInputsBuilder(std::size_t texture_pool_capacity = 16) noexcept;

    [[nodiscard]] FlowInputBundle build(
        std::uint64_t previous_frame_id,
        const GpuTextureDescriptor& previous_descriptor,
        std::uint64_t current_frame_id,
        const GpuTextureDescriptor& current_descriptor
    ) noexcept;
    void release(const FlowInputBundle& bundle) noexcept;
    [[nodiscard]] std::size_t pooled_texture_count() const noexcept;

  private:
    [[nodiscard]] FlowFrameResources build_frame_resources(
        std::uint64_t frame_id,
        const GpuTextureDescriptor& descriptor
    ) noexcept;

    KfiTexturePool texture_pool_;
    KfiLumaPyramidBuilder luma_builder_ {};
};

}  // namespace fluxma
