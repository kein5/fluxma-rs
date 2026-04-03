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

    return EXIT_SUCCESS;
}
