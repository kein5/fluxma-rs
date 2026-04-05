#include <cstdlib>
#include <iostream>

#include "fluxma_kwin_hook_adapter.h"
#include "fluxma_kwin_hook_candidates.h"
#include "fluxma_kwin_hook_builders.h"

int main() {
    const auto frame_candidate = fluxma::KfiKwinHookAdapter::preferred_frame_candidate();
    if (frame_candidate.hook_point != fluxma::KwinFrameHookPoint::CompositorOutputFrameReady) {
        std::cerr << "adapter must prefer compositor frame candidate\n";
        return EXIT_FAILURE;
    }

    const auto present_candidate = fluxma::KfiKwinHookAdapter::preferred_present_candidate();
    if (present_candidate.hook_point != fluxma::KwinPresentHookPoint::OutputFramePresented) {
        std::cerr << "adapter must prefer output-frame present candidate\n";
        return EXIT_FAILURE;
    }

    const auto frame_readiness = fluxma::KfiKwinHookAdapter::assess_frame_candidate(
        fluxma::KwinCompositorFrameInputs {
            .output_id = 0,
            .frame_id = 1,
            .timestamp_ns = 2,
            .width = 1920,
            .height = 1080,
            .gpu_handle = fluxma::GpuFrameHandle {.backend_kind = 0, .handle_id = 7},
        }
    );
    if (frame_readiness.ready || frame_readiness.missing_fields != fluxma::KwinFrameInputField::None ||
        frame_readiness.unresolved_fields == fluxma::KwinFrameInputField::None) {
        std::cerr << "adapter frame candidate must stay unresolved before real hook landing\n";
        return EXIT_FAILURE;
    }

    const auto present_readiness = fluxma::KfiKwinHookAdapter::assess_present_candidate(
        fluxma::KwinPresentFeedbackInputs {
            .output_id = 0,
            .frame_id = 1,
            .presented_timestamp_ns = 2,
            .refresh_interval_ns = 16'666'667,
        }
    );
    if (present_readiness.ready ||
        present_readiness.missing_fields != fluxma::KwinPresentInputField::None ||
        present_readiness.unresolved_fields == fluxma::KwinPresentInputField::None) {
        std::cerr << "adapter present candidate must stay unresolved before real hook landing\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
