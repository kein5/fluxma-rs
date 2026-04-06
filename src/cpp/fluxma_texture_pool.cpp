#include "fluxma_texture_pool.h"

namespace fluxma {

KfiTexturePool::KfiTexturePool(std::size_t capacity) noexcept
    : capacity_(capacity < kMaxSlots ? capacity : kMaxSlots) {
    if (capacity_ == 0) {
        capacity_ = 1;
    }
}

GpuTextureLease KfiTexturePool::acquire(const GpuTextureDescriptor& descriptor) noexcept {
    if (!descriptor.is_valid()) {
        return {};
    }

    for (std::size_t index = 0; index < capacity_; ++index) {
        auto& slot = slots_[index];
        if (slot.in_use) {
            continue;
        }

        slot.descriptor = descriptor;
        slot.in_use = true;
        return GpuTextureLease {
            .slot_id = static_cast<std::uint32_t>(index + 1),
            .descriptor = descriptor,
            .acquired = true,
            .placeholder_only = true,
        };
    }

    return {};
}

bool KfiTexturePool::release(std::uint32_t slot_id) noexcept {
    if (slot_id == 0) {
        return false;
    }

    const auto index = static_cast<std::size_t>(slot_id - 1);
    if (index >= capacity_ || !slots_[index].in_use) {
        return false;
    }

    slots_[index].in_use = false;
    return true;
}

std::size_t KfiTexturePool::capacity() const noexcept {
    return capacity_;
}

std::size_t KfiTexturePool::in_use_count() const noexcept {
    std::size_t count = 0;
    for (std::size_t index = 0; index < capacity_; ++index) {
        if (slots_[index].in_use) {
            ++count;
        }
    }
    return count;
}

}  // namespace fluxma
