#include "fluxma_plugin_root.h"

namespace fluxma {

KfiPluginRoot::KfiPluginRoot(ModuleConfig config)
    : config_(config),
      primary_output_(0, config_),
      primary_output_hook_adapter_(0, primary_output_) {
    // TODO: This is a native-module skeleton only. Do not add QML effect entry points here.
}

KfiPluginRoot::KfiPluginRoot(ModuleConfig config, bool force_rust_core_unavailable_for_tests)
    : config_(config),
      primary_output_(0, config_, force_rust_core_unavailable_for_tests),
      primary_output_hook_adapter_(0, primary_output_) {
    // TODO: This constructor is a local test seam for bypass fallback only.
}

KfiOutputController& KfiPluginRoot::primary_output() noexcept {
    return primary_output_;
}

const KfiOutputController& KfiPluginRoot::primary_output() const noexcept {
    return primary_output_;
}

KfiKwinHookAdapter& KfiPluginRoot::primary_output_hook_adapter() noexcept {
    return primary_output_hook_adapter_;
}

const KfiKwinHookAdapter& KfiPluginRoot::primary_output_hook_adapter() const noexcept {
    return primary_output_hook_adapter_;
}

}  // namespace fluxma
