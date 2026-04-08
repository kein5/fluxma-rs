#include <cstdlib>
#include <iostream>

#include "fluxma_logger.h"

int main() {
    fluxma::KfiRateLimitedLogger logger(4, 2);
    logger.note_present_feedback_issue(0, 3, 1, false, true);
    logger.note_present_feedback_issue(0, 4, 5, false, true);
    logger.note_present_feedback_issue(0, 5, 6, false, true);

    const auto logs = logger.snapshot_messages();
    if (logs.size() != 2) {
        std::cerr << "logger capacity mismatch\n";
        return EXIT_FAILURE;
    }
    if (logs.front().find("present-feedback-count=1") == std::string::npos) {
        std::cerr << "logger rate limit mismatch\n";
        return EXIT_FAILURE;
    }
    if (logs.back().find("present-feedback-count=5") == std::string::npos) {
        std::cerr << "logger eviction order mismatch\n";
        return EXIT_FAILURE;
    }
    if (logs.back().find("dropped-synthetic=yes") == std::string::npos) {
        std::cerr << "logger latest entry mismatch\n";
        return EXIT_FAILURE;
    }
    if (logs.back().find("present-feedback-count=6") != std::string::npos) {
        std::cerr << "logger should have rate-limited the last event\n";
        return EXIT_FAILURE;
    }

    fluxma::KfiRateLimitedLogger synth_logger(4, 4);
    synth_logger.note_synthetic_artifact(
        0,
        1,
        fluxma::SyntheticFrameArtifact {
            .output_id = 0,
            .source_frame_id = 5,
            .synthetic_frame_id = 11,
            .target_present_timestamp_ns = 183333332,
            .generated = true,
            .dropped = false,
            .placeholder_only = true,
        }
    );
    synth_logger.note_synthetic_artifact(
        0,
        2,
        fluxma::SyntheticFrameArtifact {
            .output_id = 0,
            .source_frame_id = 5,
            .synthetic_frame_id = 11,
            .target_present_timestamp_ns = 183333332,
            .generated = true,
            .dropped = false,
            .placeholder_only = true,
        }
    );
    synth_logger.note_synthetic_artifact(
        0,
        3,
        fluxma::SyntheticFrameArtifact {
            .output_id = 0,
            .source_frame_id = 5,
            .synthetic_frame_id = 11,
            .target_present_timestamp_ns = 183333332,
            .generated = false,
            .dropped = true,
            .placeholder_only = true,
        }
    );
    const auto synth_logs = synth_logger.snapshot_messages();
    if (synth_logs.size() != 2 ||
        synth_logs.front().find("synthetic-generated=yes") == std::string::npos ||
        synth_logs.back().find("synthetic-drop=yes") == std::string::npos ||
        synth_logs.back().find("synthetic-placeholder=yes") == std::string::npos) {
        std::cerr << "synthetic artifact logger mismatch\n";
        return EXIT_FAILURE;
    }

    synth_logger.note_synthetic_submission(
        0,
        4,
        fluxma::SyntheticPresentSubmission {
            .output_id = 0,
            .source_frame_id = 5,
            .synthetic_frame_id = 11,
            .target_present_timestamp_ns = 183333332,
            .queued = true,
            .dropped = false,
            .placeholder_only = true,
        }
    );
    const auto submission_logs = synth_logger.snapshot_messages();
    if (submission_logs.size() != 3 ||
        submission_logs.back().find("synthetic-queued=yes") == std::string::npos ||
        submission_logs.back().find("synthetic-drop=no") == std::string::npos ||
        submission_logs.back().find("synthetic-placeholder=yes") == std::string::npos ||
        submission_logs.back().find("synthetic-cursor-passthrough=no") == std::string::npos ||
        submission_logs.back().find("synthetic-overlay-passthrough=no") == std::string::npos ||
        submission_logs.back().find("synthetic-subtitle-current=no") == std::string::npos ||
        submission_logs.back().find("synthetic-frame-id=11") == std::string::npos) {
        std::cerr << "synthetic submission logger mismatch\n";
        return EXIT_FAILURE;
    }
    synth_logger.note_synthetic_submission(
        0,
        5,
        fluxma::SyntheticPresentSubmission {
            .output_id = 0,
            .source_frame_id = 5,
            .synthetic_frame_id = 12,
            .target_present_timestamp_ns = 199999999,
            .queued = false,
            .dropped = true,
            .protection_plan =
                fluxma::ProtectionPlan {
                    .cursor_passthrough = true,
                    .cursor_recomposite = true,
                    .subtitle_band_active = true,
                    .transient_overlay_passthrough = true,
                    .subtitle_band_top = 820,
                    .subtitle_band_bottom = 1080,
                    .placeholder_only = true,
                },
            .prefer_current_in_subtitle_band = true,
            .placeholder_only = true,
        }
    );
    const auto dropped_submission_logs = synth_logger.snapshot_messages();
    if (dropped_submission_logs.size() != 4 ||
        dropped_submission_logs.back().find("synthetic-queued=no") == std::string::npos ||
        dropped_submission_logs.back().find("synthetic-drop=yes") == std::string::npos ||
        dropped_submission_logs.back().find("synthetic-placeholder=yes") == std::string::npos ||
        dropped_submission_logs.back().find("synthetic-cursor-passthrough=yes") ==
            std::string::npos ||
        dropped_submission_logs.back().find("synthetic-overlay-passthrough=yes") ==
            std::string::npos ||
        dropped_submission_logs.back().find("synthetic-subtitle-current=yes") ==
            std::string::npos ||
        dropped_submission_logs.back().find("synthetic-frame-id=12") == std::string::npos) {
        std::cerr << "dropped synthetic submission logger mismatch\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
