#include <cstdlib>
#include <iostream>

#include "fluxma_kwin_hook_builders.h"

int main() {
    const auto compositor_sources = fluxma::KfiKwinFrameBuilder::compositor_field_sources();
    if (compositor_sources.frame_id != fluxma::KwinFrameFieldSource::CompositorHook ||
        compositor_sources.target_presentation_timestamp_ns !=
            fluxma::KwinFrameFieldSource::OutputFrame ||
        compositor_sources.cursor_state != fluxma::KwinFrameFieldSource::CursorSceneState ||
        compositor_sources.gpu_handle != fluxma::KwinFrameFieldSource::BackendPresentPath) {
        std::cerr << "compositor field source plan must stay explicit\n";
        return EXIT_FAILURE;
    }

    const auto backend_sources =
        fluxma::KfiKwinFrameBuilder::backend_present_handoff_field_sources();
    if (backend_sources.width != fluxma::KwinFrameFieldSource::BackendPresentPath ||
        backend_sources.color_space != fluxma::KwinFrameFieldSource::BackendPresentPath) {
        std::cerr << "backend handoff field source plan must stay explicit\n";
        return EXIT_FAILURE;
    }

    const auto output_frame_sources =
        fluxma::KfiKwinPresentBuilder::output_frame_presented_field_sources();
    if (output_frame_sources.frame_id != fluxma::KwinPresentFieldSource::OutputFramePresented ||
        output_frame_sources.refresh_interval_ns !=
            fluxma::KwinPresentFieldSource::OutputFramePresented) {
        std::cerr << "output frame present source plan must stay explicit\n";
        return EXIT_FAILURE;
    }

    const auto render_loop_sources =
        fluxma::KfiKwinPresentBuilder::render_loop_presented_field_sources();
    if (render_loop_sources.presented_timestamp_ns !=
            fluxma::KwinPresentFieldSource::RenderLoopPresented ||
        render_loop_sources.presentation_mode !=
            fluxma::KwinPresentFieldSource::RenderLoopPresented) {
        std::cerr << "render loop present source plan must stay explicit\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
