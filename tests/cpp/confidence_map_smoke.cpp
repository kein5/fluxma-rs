#include <cstdlib>
#include <iostream>

#include "fluxma_confidence_map.h"

int main() {
    fluxma::KfiFlowInputsBuilder flow_builder(16);
    const auto bundle = flow_builder.build(
        300,
        fluxma::GpuTextureDescriptor {.width = 1920, .height = 1080, .pixel_format = 1},
        301,
        fluxma::GpuTextureDescriptor {.width = 1920, .height = 1080, .pixel_format = 1}
    );

    fluxma::KfiConfidenceMapBuilder confidence_builder;
    const auto confidence = confidence_builder.build(bundle);
    if (!confidence.valid || !confidence.placeholder_only || confidence.previous_frame_id != 300 ||
        confidence.current_frame_id != 301 || confidence.built_levels == 0 ||
        confidence.levels[0].width != 1920 || confidence.levels[0].height != 1080 ||
        confidence.levels[0].confidence_bias != 255 ||
        confidence.levels[1].confidence_bias != 192) {
        std::cerr << "confidence map build mismatch\n";
        return EXIT_FAILURE;
    }

    flow_builder.release(bundle);

    const auto invalid = confidence_builder.build(fluxma::FlowInputBundle {});
    if (invalid.valid || invalid.built_levels != 0) {
        std::cerr << "confidence map invalid bundle mismatch\n";
        return EXIT_FAILURE;
    }

    fluxma::FlowInputBundle truncated_bundle {};
    truncated_bundle.previous.frame_id = 500;
    truncated_bundle.current.frame_id = 501;
    truncated_bundle.truncated = true;
    const auto truncated_confidence = confidence_builder.build(truncated_bundle);
    if (truncated_confidence.valid || truncated_confidence.built_levels != 0) {
        std::cerr << "confidence map truncated bundle mismatch\n";
        return EXIT_FAILURE;
    }

    fluxma::FlowInputBundle mismatched_bundle {};
    mismatched_bundle.valid = true;
    mismatched_bundle.previous.valid = true;
    mismatched_bundle.current.valid = true;
    mismatched_bundle.previous.frame_id = 600;
    mismatched_bundle.current.frame_id = 601;
    mismatched_bundle.previous.luma_pyramid.valid = true;
    mismatched_bundle.current.luma_pyramid.valid = true;
    mismatched_bundle.previous.luma_pyramid.built_levels = 3;
    mismatched_bundle.current.luma_pyramid.built_levels = 1;
    mismatched_bundle.previous.luma_pyramid.levels[0] =
        fluxma::LumaPyramidLevel {.level_index = 0, .width = 640, .height = 360, .acquired = true};
    mismatched_bundle.current.luma_pyramid.levels[0] =
        fluxma::LumaPyramidLevel {.level_index = 0, .width = 640, .height = 360, .acquired = true};
    const auto mismatched_confidence = confidence_builder.build(mismatched_bundle);
    if (!mismatched_confidence.valid || mismatched_confidence.built_levels != 1 ||
        mismatched_confidence.levels[0].width != 640 ||
        mismatched_confidence.levels[0].height != 360) {
        std::cerr << "confidence map level-count mismatch\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
