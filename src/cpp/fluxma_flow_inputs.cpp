#include "fluxma_flow_inputs.h"

namespace fluxma {

KfiFlowInputsBuilder::KfiFlowInputsBuilder(std::size_t texture_pool_capacity) noexcept
    : texture_pool_(texture_pool_capacity) {}

FlowInputBundle KfiFlowInputsBuilder::build(
    std::uint64_t previous_frame_id,
    const GpuTextureDescriptor& previous_descriptor,
    std::uint64_t current_frame_id,
    const GpuTextureDescriptor& current_descriptor
) noexcept {
    auto previous = build_frame_resources(previous_frame_id, previous_descriptor);
    auto current = build_frame_resources(current_frame_id, current_descriptor);

    if (!previous.valid || !current.valid) {
        if (previous.valid) {
            luma_builder_.release(texture_pool_, previous.luma_pyramid);
            static_cast<void>(texture_pool_.release(previous.source_texture.slot_id));
        }
        if (current.valid) {
            luma_builder_.release(texture_pool_, current.luma_pyramid);
            static_cast<void>(texture_pool_.release(current.source_texture.slot_id));
        }
        return FlowInputBundle {
            .previous = previous,
            .current = current,
            .valid = false,
            .truncated = previous.truncated || current.truncated,
            .placeholder_only = true,
        };
    }

    return FlowInputBundle {
        .previous = previous,
        .current = current,
        .valid = true,
        .truncated = false,
        .placeholder_only = true,
    };
}

void KfiFlowInputsBuilder::release(const FlowInputBundle& bundle) noexcept {
    if (bundle.previous.valid) {
        luma_builder_.release(texture_pool_, bundle.previous.luma_pyramid);
        static_cast<void>(texture_pool_.release(bundle.previous.source_texture.slot_id));
    }
    if (bundle.current.valid) {
        luma_builder_.release(texture_pool_, bundle.current.luma_pyramid);
        static_cast<void>(texture_pool_.release(bundle.current.source_texture.slot_id));
    }
}

std::size_t KfiFlowInputsBuilder::pooled_texture_count() const noexcept {
    return texture_pool_.in_use_count();
}

FlowFrameResources KfiFlowInputsBuilder::build_frame_resources(
    std::uint64_t frame_id,
    const GpuTextureDescriptor& descriptor
) noexcept {
    if (!descriptor.is_valid()) {
        return FlowFrameResources {.frame_id = frame_id};
    }

    const auto source_texture = texture_pool_.acquire(descriptor);
    if (!source_texture.acquired) {
        return FlowFrameResources {.frame_id = frame_id, .truncated = true};
    }

    const auto luma_pyramid = luma_builder_.build(frame_id, descriptor, texture_pool_);
    if (!luma_pyramid.valid || luma_pyramid.truncated) {
        luma_builder_.release(texture_pool_, luma_pyramid);
        static_cast<void>(texture_pool_.release(source_texture.slot_id));
        return FlowFrameResources {
            .frame_id = frame_id,
            .truncated = luma_pyramid.truncated,
        };
    }

    return FlowFrameResources {
        .frame_id = frame_id,
        .source_texture = source_texture,
        .luma_pyramid = luma_pyramid,
        .valid = true,
        .truncated = false,
        .placeholder_only = true,
    };
}

}  // namespace fluxma
