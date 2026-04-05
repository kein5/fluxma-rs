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
    const auto frame_checklist = fluxma::KfiKwinHookAdapter::preferred_frame_checklist();
    if (frame_checklist[0] != "confirm final composed frame-id provenance" ||
        frame_checklist[4] != "confirm stable gpu handle ownership at present handoff") {
        std::cerr << "adapter frame checklist must stay aligned with candidate plan\n";
        return EXIT_FAILURE;
    }

    const auto present_candidate = fluxma::KfiKwinHookAdapter::preferred_present_candidate();
    if (present_candidate.hook_point != fluxma::KwinPresentHookPoint::OutputFramePresented) {
        std::cerr << "adapter must prefer output-frame present candidate\n";
        return EXIT_FAILURE;
    }
    const auto present_checklist = fluxma::KfiKwinHookAdapter::preferred_present_checklist();
    if (present_checklist[0] != "confirm presented frame-id still correlates with submitted frame" ||
        present_checklist[2] !=
            "confirm refresh interval is available without backend-specific fallback") {
        std::cerr << "adapter present checklist must stay aligned with candidate plan\n";
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
    const auto frame_summary = fluxma::KfiKwinHookAdapter::summarize_frame_candidate(
        fluxma::KwinCompositorFrameInputs {
            .output_id = 0,
            .frame_id = 1,
            .timestamp_ns = 2,
            .width = 1920,
            .height = 1080,
            .gpu_handle = fluxma::GpuFrameHandle {.backend_kind = 0, .handle_id = 7},
        }
    );
    if (frame_summary.find("hook=compositor-output-frame-ready") == std::string::npos ||
        frame_summary.find("checklist=confirm final composed frame-id provenance") ==
            std::string::npos) {
        std::cerr << "adapter frame summary must stay aligned with readiness summary\n";
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
    const auto present_summary = fluxma::KfiKwinHookAdapter::summarize_present_candidate(
        fluxma::KwinPresentFeedbackInputs {
            .output_id = 0,
            .frame_id = 1,
            .presented_timestamp_ns = 2,
            .refresh_interval_ns = 16'666'667,
        }
    );
    if (present_summary.find("hook=output-frame-presented") == std::string::npos ||
        present_summary.find(
            "checklist=confirm presented frame-id still correlates with submitted frame"
        ) == std::string::npos) {
        std::cerr << "adapter present summary must stay aligned with readiness summary\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
