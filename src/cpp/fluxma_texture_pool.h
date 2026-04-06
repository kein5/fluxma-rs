#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace fluxma {

struct GpuTextureDescriptor {
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::uint32_t pixel_format = 0;

    [[nodiscard]] bool is_valid() const noexcept {
        return width != 0 && height != 0;
    }
};

struct GpuTextureLease {
    std::uint32_t slot_id = 0;
    std::uint32_t generation = 0;
    GpuTextureDescriptor descriptor {};
    bool acquired = false;
    bool placeholder_only = true;
};

class KfiTexturePool {
  public:
    static constexpr std::size_t kMaxSlots = 16;

    explicit KfiTexturePool(std::size_t capacity = 4) noexcept;

    [[nodiscard]] GpuTextureLease acquire(const GpuTextureDescriptor& descriptor) noexcept;
    [[nodiscard]] bool release(const GpuTextureLease& lease) noexcept;
    [[nodiscard]] std::size_t capacity() const noexcept;
    [[nodiscard]] std::size_t in_use_count() const noexcept;

  private:
    struct Slot {
        std::uint32_t generation = 0;
        GpuTextureDescriptor descriptor {};
        bool in_use = false;
    };

    std::array<Slot, kMaxSlots> slots_ {};
    std::size_t capacity_ = 0;
};

}  // namespace fluxma
