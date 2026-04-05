#pragma once

#include "fluxma_config.h"
#include "fluxma_kwin_hook_adapter.h"
#include "fluxma_kwin_native_bridge.h"
#include "fluxma_output_controller.h"

namespace fluxma {

class KfiPluginRoot {
  public:
    explicit KfiPluginRoot(ModuleConfig config = {});
    KfiPluginRoot(ModuleConfig config, bool force_rust_core_unavailable_for_tests);

    [[nodiscard]] KfiOutputController& primary_output() noexcept;
    [[nodiscard]] const KfiOutputController& primary_output() const noexcept;
    [[nodiscard]] KfiKwinHookAdapter& primary_output_hook_adapter() noexcept;
    [[nodiscard]] const KfiKwinHookAdapter& primary_output_hook_adapter() const noexcept;
    [[nodiscard]] KfiKwinNativeBridge& native_bridge() noexcept;
    [[nodiscard]] const KfiKwinNativeBridge& native_bridge() const noexcept;

  private:
    ModuleConfig config_ {};
    KfiOutputController primary_output_;
    KfiKwinHookAdapter primary_output_hook_adapter_;
    KfiKwinNativeBridge native_bridge_;
};

}  // namespace fluxma
