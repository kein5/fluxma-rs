#include <cstdlib>
#include <iostream>

#include "fluxma_kwin_hook_candidates.h"

int main() {
    const auto compositor = fluxma::KfiKwinHookCandidates::compositor_output_frame_ready();
    if (compositor.hook_point != fluxma::KwinFrameHookPoint::CompositorOutputFrameReady ||
        compositor.source_file != "src/compositor_wayland.cpp" ||
        compositor.symbol != "WaylandCompositor::composite(RenderLoop *)" ||
        fluxma::to_string(compositor.hook_point) != "compositor-output-frame-ready") {
        std::cerr << "compositor hook candidate must stay pinned to current source plan\n";
        return EXIT_FAILURE;
    }
    const auto compositor_required_names = fluxma::describe_required(compositor);
    if (compositor_required_names[0] != "frame-id" || compositor_required_names[4] != "gpu-handle") {
        std::cerr << "compositor required fields must stay named\n";
        return EXIT_FAILURE;
    }
    const auto compositor_unresolved_names = fluxma::describe_unresolved(compositor);
    if (compositor_unresolved_names[0] != "frame-id" ||
        compositor_unresolved_names[4] != "gpu-handle") {
        std::cerr << "compositor unresolved fields must stay named\n";
        return EXIT_FAILURE;
    }

    const auto backend = fluxma::KfiKwinHookCandidates::backend_present_handoff();
    if (backend.hook_point != fluxma::KwinFrameHookPoint::BackendPresentHandoff ||
        backend.field_sources.gpu_handle != fluxma::KwinFrameFieldSource::BackendPresentPath) {
        std::cerr << "backend handoff candidate must preserve gpu-handle source plan\n";
        return EXIT_FAILURE;
    }

    const auto output_frame = fluxma::KfiKwinHookCandidates::output_frame_presented();
    if (output_frame.hook_point != fluxma::KwinPresentHookPoint::OutputFramePresented ||
        output_frame.source_file != "src/core/renderbackend.cpp" ||
        output_frame.symbol != "OutputFrame::presented(...)" ||
        fluxma::to_string(output_frame.hook_point) != "output-frame-presented") {
        std::cerr << "output frame present candidate must stay pinned to current source plan\n";
        return EXIT_FAILURE;
    }
    const auto output_frame_required_names = fluxma::describe_required(output_frame);
    if (output_frame_required_names[0] != "frame-id" ||
        output_frame_required_names[2] != "refresh-interval-ns") {
        std::cerr << "present required fields must stay named\n";
        return EXIT_FAILURE;
    }

    const auto render_loop = fluxma::KfiKwinHookCandidates::render_loop_frame_presented();
    if (render_loop.hook_point != fluxma::KwinPresentHookPoint::RenderLoopFramePresented ||
        render_loop.source_file != "src/core/renderloop.cpp" ||
        render_loop.symbol != "RenderLoopPrivate::notifyFrameCompleted(...)" ||
        fluxma::to_string(render_loop.hook_point) != "render-loop-frame-presented") {
        std::cerr << "render loop present candidate must stay pinned to current source plan\n";
        return EXIT_FAILURE;
    }

    const auto ready_frame = fluxma::KfiKwinHookCandidates::assess(
        compositor,
        fluxma::KwinCompositorFrameInputs {
            .output_id = 0,
            .frame_id = 1,
            .timestamp_ns = 2,
            .width = 1920,
            .height = 1080,
            .gpu_handle = fluxma::GpuFrameHandle {.backend_kind = 0, .handle_id = 9},
        }
    );
    if (ready_frame.ready || ready_frame.missing_fields != fluxma::KwinFrameInputField::None ||
        ready_frame.unresolved_fields != compositor.unresolved_fields) {
        std::cerr << "compositor candidate must remain unresolved even with complete inputs\n";
        return EXIT_FAILURE;
    }

    const auto missing_frame = fluxma::KfiKwinHookCandidates::assess(
        compositor,
        fluxma::KwinCompositorFrameInputs {
            .output_id = 0,
            .frame_id = 1,
            .timestamp_ns = 2,
            .width = 1920,
            .height = 1080,
        }
    );
    if (missing_frame.ready ||
        !fluxma::has_flag(missing_frame.missing_fields, fluxma::KwinFrameInputField::GpuHandle)) {
        std::cerr << "candidate readiness must surface missing gpu handle\n";
        return EXIT_FAILURE;
    }

    const auto missing_present = fluxma::KfiKwinHookCandidates::assess(
        output_frame,
        fluxma::KwinPresentFeedbackInputs {
            .output_id = 0,
            .frame_id = 7,
            .presented_timestamp_ns = 8,
            .refresh_interval_ns = 0,
        }
    );
    if (missing_present.ready ||
        !fluxma::has_flag(
            missing_present.missing_fields,
            fluxma::KwinPresentInputField::RefreshInterval
        ) ||
        missing_present.unresolved_fields != output_frame.unresolved_fields) {
        std::cerr << "present candidate readiness must surface missing refresh interval\n";
        return EXIT_FAILURE;
    }

    const auto backend_missing = fluxma::KfiKwinHookCandidates::assess(
        backend,
        fluxma::KwinCompositorFrameInputs {
            .output_id = 0,
            .frame_id = 5,
            .timestamp_ns = 6,
            .width = 1280,
            .height = 720,
        }
    );
    if (backend_missing.ready ||
        !fluxma::has_flag(backend_missing.missing_fields, fluxma::KwinFrameInputField::GpuHandle) ||
        backend_missing.unresolved_fields != backend.unresolved_fields) {
        std::cerr << "backend candidate readiness must surface missing gpu handle\n";
        return EXIT_FAILURE;
    }

    const auto render_loop_missing = fluxma::KfiKwinHookCandidates::assess(
        render_loop,
        fluxma::KwinPresentFeedbackInputs {
            .output_id = 0,
            .frame_id = 11,
            .presented_timestamp_ns = 0,
            .refresh_interval_ns = 16'666'667,
        }
    );
    if (render_loop_missing.ready ||
        !fluxma::has_flag(
            render_loop_missing.missing_fields,
            fluxma::KwinPresentInputField::PresentedTimestamp
        ) ||
        render_loop_missing.unresolved_fields != render_loop.unresolved_fields) {
        std::cerr << "render loop candidate readiness must surface missing timestamp\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
