#include <cstdlib>
#include <iostream>

#include "fluxma_kcm_bridge.h"

namespace {

fluxma::FrameDescriptor frame(std::uint64_t frame_id, bool protected_content = false) {
    return fluxma::FrameDescriptor {
        .frame_id = frame_id,
        .timestamp_ns = 33'333'333 * frame_id,
        .target_presentation_timestamp_ns = 33'333'333 * frame_id,
        .predicted_render_time_ns = 2'000'000,
        .width = 1920,
        .height = 1080,
        .pixel_format = 0,
        .color_space = 0,
        .content_type = fluxma::ContentType::Video,
        .protected_content = protected_content,
        .damage_ratio = 0.5,
        .cursor_visible = frame_id == 5,
        .cursor_x = 960.0,
        .cursor_y = 980.0,
        .cursor_velocity_x = 20.0,
        .cursor_velocity_y = 16.0,
        .overlay_promoted = frame_id == 5,
        .gpu_handle = fluxma::GpuFrameHandle {.backend_kind = 1, .handle_id = frame_id + 100},
    };
}

}  // namespace

int main() {
    const fluxma::ModuleConfig config {
        .enabled = true,
        .mode = fluxma::ModuleMode::Synthetic2x,
        .show_hud = true,
        .subtitle_protection = true,
        .cursor_protection = true,
        .log_interval_frames = 7,
        .max_log_messages = 9,
    };
    fluxma::KfiPluginRoot plugin_root(config);
    const fluxma::KfiKcmBridge kcm_bridge(plugin_root);

    const auto settings = kcm_bridge.settings();
    if (!settings.enabled || settings.mode != fluxma::ModuleMode::Synthetic2x ||
        !settings.show_hud || !settings.subtitle_protection ||
        !settings.cursor_protection || settings.log_interval_frames != 7 ||
        settings.summary().find("mode=synthetic-2x") == std::string::npos ||
        settings.summary().find("show-hud=yes") == std::string::npos ||
        settings.max_log_messages != 9) {
        std::cerr << "kcm settings snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    const auto direct_settings = fluxma::KcmSettingsSnapshot::from_plugin_root(plugin_root);
    if (!direct_settings.enabled || direct_settings.mode != fluxma::ModuleMode::Synthetic2x ||
        !direct_settings.show_hud || !direct_settings.subtitle_protection ||
        !direct_settings.cursor_protection || direct_settings.log_interval_frames != 7 ||
        direct_settings.max_log_messages != 9 ||
        direct_settings.summary().find("mode=synthetic-2x") == std::string::npos ||
        direct_settings.summary().find("cursor-protection=yes") == std::string::npos) {
        std::cerr << "kcm direct settings helper snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    const auto initial_overview = kcm_bridge.overview(
        0,
        fluxma::KwinNativeInstallContext {
            .kwin_version_supported = true,
            .backend_supported = false,
            .kwin_version = "6.3.93",
            .backend_name = "wayland",
        }
    );
    if (initial_overview.atomic ||
        !initial_overview.assembled_from_split_reads ||
        initial_overview.runtime_observed_at_ns != 0 ||
        initial_overview.provenance_summary != "settings+runtime+native-install" ||
        initial_overview.settings_provenance != "settings()" ||
        initial_overview.runtime_provenance != "runtime(now-ns)" ||
        initial_overview.native_install_provenance !=
            "native_bridge_install(install-context)" ||
        !initial_overview.settings.enabled ||
        initial_overview.settings.mode != fluxma::ModuleMode::Synthetic2x ||
        initial_overview.runtime.state != fluxma::OutputState::Bypass ||
        initial_overview.runtime.bypass_reason != fluxma::BypassReason::None ||
        initial_overview.native_install.state != fluxma::KwinNativeBridgeState::PlaceholderOnly ||
        initial_overview.native_install.frame_deferred_reason !=
            fluxma::KwinNativeDeferredReason::BackendGate ||
        initial_overview.native_install.present_deferred_reason !=
            fluxma::KwinNativeDeferredReason::BackendGate ||
        initial_overview.summary().find("atomic=no") == std::string::npos ||
        initial_overview.summary().find("split-reads=yes") == std::string::npos ||
        initial_overview.summary().find("runtime-observed-at-ns=0") ==
            std::string::npos ||
        initial_overview.summary().find("provenance=settings+runtime+native-install") ==
            std::string::npos ||
        initial_overview.summary().find("settings-source=settings()") ==
            std::string::npos ||
        initial_overview.summary().find("runtime-source=runtime(now-ns)") ==
            std::string::npos ||
        initial_overview.summary().find(
            "native-install-source=native_bridge_install(install-context)"
        ) == std::string::npos ||
        initial_overview.summary().find("settings{enabled=yes") == std::string::npos ||
        initial_overview.summary().find("runtime{state=bypass") == std::string::npos ||
        initial_overview.summary().find("native-install{state=placeholder-only") ==
            std::string::npos) {
        std::cerr << "kcm initial overview snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    const auto complete_bringup = kcm_bridge.native_bridge_bringup(
        fluxma::KwinCompositorFrameInputs {
            .output_id = 0,
            .frame_id = 11,
            .timestamp_ns = 1'000'000,
            .target_presentation_timestamp_ns = 16'666'667,
            .predicted_render_time_ns = 2'000'000,
            .content_type = fluxma::ContentType::Video,
            .width = 1920,
            .height = 1080,
            .gpu_handle = fluxma::GpuFrameHandle {.backend_kind = 1, .handle_id = 211},
        },
        fluxma::KwinPresentFeedbackInputs {
            .output_id = 0,
            .frame_id = 11,
            .presented_timestamp_ns = 2'000'000,
            .refresh_interval_ns = 16'666'667,
            .presentation_mode = fluxma::PresentationMode::VSync,
            .present_success = true,
            .dropped_synthetic = false,
        }
    );
    if (complete_bringup.state != fluxma::KwinNativeBridgeState::PlaceholderOnly ||
        !complete_bringup.frame_complete || !complete_bringup.present_complete ||
        !complete_bringup.frame_has_unresolved || !complete_bringup.present_has_unresolved ||
        complete_bringup.summary().find("frame-complete=yes") == std::string::npos ||
        complete_bringup.bringup_summary.find("frame{hook=compositor-output-frame-ready") ==
            std::string::npos) {
        std::cerr << "kcm native bridge bringup snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    const auto incomplete_bringup = kcm_bridge.native_bridge_bringup(
        fluxma::KwinCompositorFrameInputs {
            .output_id = 0,
            .timestamp_ns = 1'000'000,
            .width = 1920,
            .height = 1080,
        },
        fluxma::KwinPresentFeedbackInputs {
            .output_id = 0,
            .presented_timestamp_ns = 2'000'000,
        }
    );
    if (incomplete_bringup.state != fluxma::KwinNativeBridgeState::PlaceholderOnly ||
        incomplete_bringup.frame_complete || incomplete_bringup.present_complete ||
        !incomplete_bringup.frame_has_unresolved || !incomplete_bringup.present_has_unresolved ||
        incomplete_bringup.summary().find("present-complete=no") == std::string::npos ||
        incomplete_bringup.bringup_summary.find("missing=frame-id,gpu-handle") ==
            std::string::npos ||
        incomplete_bringup.bringup_summary.find("missing=frame-id,refresh-interval") ==
            std::string::npos) {
        std::cerr << "kcm native bridge incomplete bringup snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    const auto frame_only_incomplete_bringup = fluxma::KcmNativeBringupSnapshot::from_report(
        fluxma::KwinNativeBringupReport {
            .state = fluxma::KwinNativeBridgeState::PlaceholderOnly,
            .frame = fluxma::KwinFrameHookReadiness {
                .plan = fluxma::KfiKwinHookCandidates::compositor_output_frame_ready(),
                .missing_fields = fluxma::KwinFrameInputField::GpuHandle,
                .unresolved_fields = fluxma::KwinFrameInputField::FrameId,
                .ready = false,
            },
            .present = fluxma::KwinPresentHookReadiness {
                .plan = fluxma::KfiKwinHookCandidates::output_frame_presented(),
                .ready = true,
            },
        }
    );
    if (frame_only_incomplete_bringup.state != fluxma::KwinNativeBridgeState::PlaceholderOnly ||
        frame_only_incomplete_bringup.frame_complete ||
        !frame_only_incomplete_bringup.present_complete ||
        !frame_only_incomplete_bringup.frame_has_unresolved ||
        frame_only_incomplete_bringup.present_has_unresolved ||
        frame_only_incomplete_bringup.summary().find("frame-complete=no") ==
            std::string::npos ||
        frame_only_incomplete_bringup.summary().find("present-complete=yes") ==
            std::string::npos ||
        frame_only_incomplete_bringup.summary().find("frame-unresolved=yes") ==
            std::string::npos ||
        frame_only_incomplete_bringup.summary().find("present-unresolved=no") ==
            std::string::npos ||
        frame_only_incomplete_bringup.bringup_summary.find("present{hook=output-frame-presented") ==
            std::string::npos ||
        frame_only_incomplete_bringup.bringup_summary.find("missing=gpu-handle") ==
            std::string::npos) {
        std::cerr << "kcm frame-only incomplete bringup snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    const auto present_only_incomplete_bringup = fluxma::KcmNativeBringupSnapshot::from_report(
        fluxma::KwinNativeBringupReport {
            .state = fluxma::KwinNativeBridgeState::PlaceholderOnly,
            .frame = fluxma::KwinFrameHookReadiness {
                .plan = fluxma::KfiKwinHookCandidates::compositor_output_frame_ready(),
                .ready = true,
            },
            .present = fluxma::KwinPresentHookReadiness {
                .plan = fluxma::KfiKwinHookCandidates::output_frame_presented(),
                .missing_fields = fluxma::KwinPresentInputField::RefreshInterval,
                .unresolved_fields = fluxma::KwinPresentInputField::FrameId,
                .ready = false,
            },
        }
    );
    if (present_only_incomplete_bringup.state != fluxma::KwinNativeBridgeState::PlaceholderOnly ||
        !present_only_incomplete_bringup.frame_complete ||
        present_only_incomplete_bringup.present_complete ||
        present_only_incomplete_bringup.frame_has_unresolved ||
        !present_only_incomplete_bringup.present_has_unresolved ||
        present_only_incomplete_bringup.summary().find("frame-complete=yes") ==
            std::string::npos ||
        present_only_incomplete_bringup.summary().find("present-complete=no") ==
            std::string::npos ||
        present_only_incomplete_bringup.summary().find("frame-unresolved=no") ==
            std::string::npos ||
        present_only_incomplete_bringup.summary().find("present-unresolved=yes") ==
            std::string::npos ||
        present_only_incomplete_bringup.bringup_summary.find(
            "frame{hook=compositor-output-frame-ready"
        ) == std::string::npos ||
        present_only_incomplete_bringup.bringup_summary.find("missing=refresh-interval") ==
            std::string::npos) {
        std::cerr << "kcm present-only incomplete bringup snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    const auto placeholder_native_diagnostics = kcm_bridge.native_bridge_diagnostics(
        fluxma::KwinCompositorFrameInputs {
            .output_id = 0,
            .frame_id = 12,
            .timestamp_ns = 2'000'000,
            .target_presentation_timestamp_ns = 18'000'000,
            .predicted_render_time_ns = 2'000'000,
            .content_type = fluxma::ContentType::Video,
            .width = 1920,
            .height = 1080,
            .gpu_handle = fluxma::GpuFrameHandle {.backend_kind = 1, .handle_id = 212},
        },
        fluxma::KwinPresentFeedbackInputs {
            .output_id = 0,
            .frame_id = 12,
            .presented_timestamp_ns = 3'000'000,
            .refresh_interval_ns = 16'666'667,
            .presentation_mode = fluxma::PresentationMode::VSync,
            .present_success = true,
            .dropped_synthetic = false,
        },
        fluxma::KwinNativeInstallContext {}
    );
    if (placeholder_native_diagnostics.state != fluxma::KwinNativeBridgeState::PlaceholderOnly ||
        !placeholder_native_diagnostics.bringup_complete ||
        !placeholder_native_diagnostics.has_unresolved_candidates ||
        !placeholder_native_diagnostics.all_gates_match ||
        !placeholder_native_diagnostics.all_installs_deferred ||
        !placeholder_native_diagnostics.kwin_version_supported ||
        !placeholder_native_diagnostics.backend_supported ||
        placeholder_native_diagnostics.install_context_summary !=
            "frame=kwin=unspecified backend=unspecified version_supported=true backend_supported=true" ||
        placeholder_native_diagnostics.frame_install_context_summary !=
            "kwin=unspecified backend=unspecified version_supported=true backend_supported=true" ||
        placeholder_native_diagnostics.present_install_context_summary !=
            "kwin=unspecified backend=unspecified version_supported=true backend_supported=true" ||
        !placeholder_native_diagnostics.frame_gate_matches ||
        !placeholder_native_diagnostics.present_gate_matches ||
        placeholder_native_diagnostics.frame_has_any_blocker ||
        placeholder_native_diagnostics.present_has_any_blocker ||
        !placeholder_native_diagnostics.frame_install_deferred ||
        !placeholder_native_diagnostics.present_install_deferred ||
        placeholder_native_diagnostics.frame_version_blocked ||
        placeholder_native_diagnostics.present_version_blocked ||
        placeholder_native_diagnostics.frame_backend_blocked ||
        placeholder_native_diagnostics.present_backend_blocked ||
        placeholder_native_diagnostics.frame_deferred_reason !=
            fluxma::KwinNativeDeferredReason::PlaceholderOnly ||
        placeholder_native_diagnostics.present_deferred_reason !=
            fluxma::KwinNativeDeferredReason::PlaceholderOnly ||
        placeholder_native_diagnostics.summary().find("frame-blocked=no") ==
            std::string::npos ||
        placeholder_native_diagnostics.summary().find("all-gates-match=yes") ==
            std::string::npos ||
        placeholder_native_diagnostics.summary().find("frame-install-deferred=yes") ==
            std::string::npos ||
        placeholder_native_diagnostics.summary().find("kwin-version-supported=yes") ==
            std::string::npos ||
        placeholder_native_diagnostics.summary().find("backend-supported=yes") ==
            std::string::npos ||
        placeholder_native_diagnostics.summary().find(
            "install-context=frame=kwin=unspecified backend=unspecified version_supported=true backend_supported=true"
        ) == std::string::npos ||
        placeholder_native_diagnostics.summary().find(
            "frame-install-context=kwin=unspecified backend=unspecified version_supported=true backend_supported=true"
        ) == std::string::npos ||
        placeholder_native_diagnostics.summary().find(
            "present-install-context=kwin=unspecified backend=unspecified version_supported=true backend_supported=true"
        ) == std::string::npos ||
        placeholder_native_diagnostics.summary().find("frame-version-blocked=no") ==
            std::string::npos ||
        placeholder_native_diagnostics.diagnostics_summary.find(
            "deferred_reason=placeholder-only"
        ) == std::string::npos) {
        std::cerr << "kcm placeholder native diagnostics snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    const auto backend_native_diagnostics = kcm_bridge.native_bridge_diagnostics(
        fluxma::KwinCompositorFrameInputs {
            .output_id = 0,
            .frame_id = 13,
            .timestamp_ns = 2'500'000,
            .target_presentation_timestamp_ns = 18'500'000,
            .predicted_render_time_ns = 2'000'000,
            .content_type = fluxma::ContentType::Video,
            .width = 1920,
            .height = 1080,
            .gpu_handle = fluxma::GpuFrameHandle {.backend_kind = 1, .handle_id = 213},
        },
        fluxma::KwinPresentFeedbackInputs {
            .output_id = 0,
            .frame_id = 13,
            .presented_timestamp_ns = 3'500'000,
            .refresh_interval_ns = 16'666'667,
            .presentation_mode = fluxma::PresentationMode::VSync,
            .present_success = true,
            .dropped_synthetic = false,
        },
        fluxma::KwinNativeInstallContext {
            .kwin_version_supported = true,
            .backend_supported = false,
            .kwin_version = "6.3.93",
            .backend_name = "wayland",
        }
    );
    if (backend_native_diagnostics.state != fluxma::KwinNativeBridgeState::PlaceholderOnly ||
        !backend_native_diagnostics.bringup_complete ||
        !backend_native_diagnostics.has_unresolved_candidates ||
        !backend_native_diagnostics.all_gates_match ||
        !backend_native_diagnostics.all_installs_deferred ||
        !backend_native_diagnostics.kwin_version_supported ||
        backend_native_diagnostics.backend_supported ||
        backend_native_diagnostics.install_context_summary !=
            "frame=kwin=6.3.93 backend=wayland version_supported=true backend_supported=false" ||
        backend_native_diagnostics.frame_install_context_summary !=
            "kwin=6.3.93 backend=wayland version_supported=true backend_supported=false" ||
        backend_native_diagnostics.present_install_context_summary !=
            "kwin=6.3.93 backend=wayland version_supported=true backend_supported=false" ||
        !backend_native_diagnostics.frame_gate_matches ||
        !backend_native_diagnostics.present_gate_matches ||
        !backend_native_diagnostics.frame_has_any_blocker ||
        !backend_native_diagnostics.present_has_any_blocker ||
        !backend_native_diagnostics.frame_install_deferred ||
        !backend_native_diagnostics.present_install_deferred ||
        backend_native_diagnostics.frame_version_blocked ||
        backend_native_diagnostics.present_version_blocked ||
        !backend_native_diagnostics.frame_backend_blocked ||
        !backend_native_diagnostics.present_backend_blocked ||
        backend_native_diagnostics.frame_deferred_reason !=
            fluxma::KwinNativeDeferredReason::BackendGate ||
        backend_native_diagnostics.present_deferred_reason !=
            fluxma::KwinNativeDeferredReason::BackendGate ||
        backend_native_diagnostics.summary().find("present-blocked=yes") ==
            std::string::npos ||
        backend_native_diagnostics.summary().find("present-install-deferred=yes") ==
            std::string::npos ||
        backend_native_diagnostics.summary().find("kwin-version-supported=yes") ==
            std::string::npos ||
        backend_native_diagnostics.summary().find("backend-supported=no") ==
            std::string::npos ||
        backend_native_diagnostics.summary().find(
            "install-context=frame=kwin=6.3.93 backend=wayland version_supported=true backend_supported=false"
        ) == std::string::npos ||
        backend_native_diagnostics.summary().find(
            "frame-install-context=kwin=6.3.93 backend=wayland version_supported=true backend_supported=false"
        ) == std::string::npos ||
        backend_native_diagnostics.summary().find(
            "present-install-context=kwin=6.3.93 backend=wayland version_supported=true backend_supported=false"
        ) == std::string::npos ||
        backend_native_diagnostics.summary().find("present-backend-blocked=yes") ==
            std::string::npos ||
        backend_native_diagnostics.diagnostics_summary.find(
            "backend gate blocked install for wayland"
        ) == std::string::npos) {
        std::cerr << "kcm backend native diagnostics snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    const auto native_diagnostics = kcm_bridge.native_bridge_diagnostics(
        fluxma::KwinCompositorFrameInputs {
            .output_id = 0,
            .frame_id = 12,
            .timestamp_ns = 2'000'000,
            .target_presentation_timestamp_ns = 18'000'000,
            .predicted_render_time_ns = 2'000'000,
            .content_type = fluxma::ContentType::Video,
            .width = 1920,
            .height = 1080,
            .gpu_handle = fluxma::GpuFrameHandle {.backend_kind = 1, .handle_id = 212},
        },
        fluxma::KwinPresentFeedbackInputs {
            .output_id = 0,
            .frame_id = 12,
            .presented_timestamp_ns = 3'000'000,
            .refresh_interval_ns = 16'666'667,
            .presentation_mode = fluxma::PresentationMode::VSync,
            .present_success = true,
            .dropped_synthetic = false,
        },
        fluxma::KwinNativeInstallContext {
            .kwin_version_supported = false,
            .backend_supported = true,
            .kwin_version = "6.3.92",
            .backend_name = "drm",
        }
    );
    if (native_diagnostics.state != fluxma::KwinNativeBridgeState::PlaceholderOnly ||
        !native_diagnostics.bringup_complete ||
        !native_diagnostics.has_unresolved_candidates ||
        !native_diagnostics.all_gates_match ||
        !native_diagnostics.all_installs_deferred ||
        native_diagnostics.kwin_version_supported ||
        !native_diagnostics.backend_supported ||
        native_diagnostics.install_context_summary !=
            "frame=kwin=6.3.92 backend=drm version_supported=false backend_supported=true" ||
        native_diagnostics.frame_install_context_summary !=
            "kwin=6.3.92 backend=drm version_supported=false backend_supported=true" ||
        native_diagnostics.present_install_context_summary !=
            "kwin=6.3.92 backend=drm version_supported=false backend_supported=true" ||
        !native_diagnostics.frame_gate_matches ||
        !native_diagnostics.present_gate_matches ||
        !native_diagnostics.frame_has_any_blocker ||
        !native_diagnostics.present_has_any_blocker ||
        !native_diagnostics.frame_install_deferred ||
        !native_diagnostics.present_install_deferred ||
        !native_diagnostics.frame_version_blocked ||
        !native_diagnostics.present_version_blocked ||
        native_diagnostics.frame_backend_blocked ||
        native_diagnostics.present_backend_blocked ||
        native_diagnostics.frame_deferred_reason !=
            fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        native_diagnostics.present_deferred_reason !=
            fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        native_diagnostics.summary().find("bringup-complete=yes") == std::string::npos ||
        native_diagnostics.summary().find("frame-install-deferred=yes") ==
            std::string::npos ||
        native_diagnostics.summary().find("kwin-version-supported=no") ==
            std::string::npos ||
        native_diagnostics.summary().find("backend-supported=yes") ==
            std::string::npos ||
        native_diagnostics.summary().find(
            "install-context=frame=kwin=6.3.92 backend=drm version_supported=false backend_supported=true"
        ) == std::string::npos ||
        native_diagnostics.summary().find(
            "frame-install-context=kwin=6.3.92 backend=drm version_supported=false backend_supported=true"
        ) == std::string::npos ||
        native_diagnostics.summary().find(
            "present-install-context=kwin=6.3.92 backend=drm version_supported=false backend_supported=true"
        ) == std::string::npos ||
        native_diagnostics.summary().find("frame-version-blocked=yes") ==
            std::string::npos ||
        native_diagnostics.diagnostics_summary.find("preflight{") == std::string::npos ||
        native_diagnostics.diagnostics_summary.find(
            "reason=kwin version gate blocked install for 6.3.92"
        ) == std::string::npos) {
        std::cerr << "kcm native diagnostics snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    const auto asymmetric_diagnostics = fluxma::KcmNativeDiagnosticsSnapshot::from_observation(
        fluxma::KwinNativeBridgeObservationReport {
            .state = fluxma::KwinNativeBridgeState::PlaceholderOnly,
            .bringup = fluxma::KwinNativeBringupReport {
                .frame = fluxma::KwinFrameHookReadiness {
                    .plan = fluxma::KfiKwinHookCandidates::compositor_output_frame_ready(),
                    .ready = true,
                },
                .present = fluxma::KwinPresentHookReadiness {
                    .plan = fluxma::KfiKwinHookCandidates::output_frame_presented(),
                    .ready = true,
                },
            },
            .preflight = fluxma::KwinNativeCombinedPreflightReport {
                .frame = fluxma::KwinNativeInstallPreflightReport {
                    .gate = fluxma::KwinNativeInstallGateAssessment {
                        .deferred_reason = fluxma::KwinNativeDeferredReason::PlaceholderOnly,
                        .reason = "frame placeholder path",
                        .context_summary =
                            "kwin=6.3.93 backend=wayland version_supported=true backend_supported=false",
                        .version_blocked = false,
                        .backend_blocked = true,
                    },
                },
                .present = fluxma::KwinNativeInstallPreflightReport {
                    .gate = fluxma::KwinNativeInstallGateAssessment {
                        .deferred_reason = fluxma::KwinNativeDeferredReason::KwinVersionGate,
                        .reason = "present version gate",
                        .context_summary =
                            "kwin=6.3.92 backend=drm version_supported=false backend_supported=true",
                        .version_blocked = true,
                        .backend_blocked = false,
                    },
                },
            },
            .install = fluxma::KwinNativeCombinedInstallReport {
                .frame = fluxma::KwinNativeInstallReport {
                    .result = fluxma::KwinNativeInstallResult::Deferred,
                    .deferred_reason = fluxma::KwinNativeDeferredReason::PlaceholderOnly,
                },
                .present = fluxma::KwinNativeInstallReport {
                    .result = fluxma::KwinNativeInstallResult::Installed,
                    .deferred_reason = fluxma::KwinNativeDeferredReason::PlaceholderOnly,
                },
            },
        }
    );
    if (asymmetric_diagnostics.kwin_version_supported ||
        asymmetric_diagnostics.backend_supported ||
        asymmetric_diagnostics.install_context_summary !=
            "frame=kwin=6.3.93 backend=wayland version_supported=true backend_supported=false present=kwin=6.3.92 backend=drm version_supported=false backend_supported=true" ||
        asymmetric_diagnostics.frame_install_context_summary !=
            "kwin=6.3.93 backend=wayland version_supported=true backend_supported=false" ||
        asymmetric_diagnostics.present_install_context_summary !=
            "kwin=6.3.92 backend=drm version_supported=false backend_supported=true" ||
        asymmetric_diagnostics.frame_backend_blocked != true ||
        asymmetric_diagnostics.present_backend_blocked != false ||
        asymmetric_diagnostics.frame_version_blocked != false ||
        asymmetric_diagnostics.present_version_blocked != true ||
        asymmetric_diagnostics.all_gates_match ||
        asymmetric_diagnostics.all_installs_deferred ||
        asymmetric_diagnostics.frame_deferred_reason !=
            fluxma::KwinNativeDeferredReason::PlaceholderOnly ||
        asymmetric_diagnostics.present_deferred_reason !=
            fluxma::KwinNativeDeferredReason::PlaceholderOnly ||
        !asymmetric_diagnostics.frame_install_deferred ||
        asymmetric_diagnostics.present_install_deferred ||
        asymmetric_diagnostics.summary().find("all-gates-match=no") ==
            std::string::npos ||
        asymmetric_diagnostics.summary().find("all-installs-deferred=no") ==
            std::string::npos ||
        asymmetric_diagnostics.summary().find("frame-backend-blocked=yes") ==
            std::string::npos ||
        asymmetric_diagnostics.summary().find("present-backend-blocked=no") ==
            std::string::npos ||
        asymmetric_diagnostics.summary().find("frame-version-blocked=no") ==
            std::string::npos ||
        asymmetric_diagnostics.summary().find("present-version-blocked=yes") ==
            std::string::npos ||
        asymmetric_diagnostics.summary().find("kwin-version-supported=no") ==
            std::string::npos ||
        asymmetric_diagnostics.summary().find("backend-supported=no") ==
            std::string::npos ||
        asymmetric_diagnostics.summary().find(
            "install-context=frame=kwin=6.3.93 backend=wayland version_supported=true backend_supported=false present=kwin=6.3.92 backend=drm version_supported=false backend_supported=true"
        ) ==
            std::string::npos ||
        asymmetric_diagnostics.summary().find(
            "frame-install-context=kwin=6.3.93 backend=wayland version_supported=true backend_supported=false"
        ) ==
            std::string::npos ||
        asymmetric_diagnostics.summary().find(
            "present-install-context=kwin=6.3.92 backend=drm version_supported=false backend_supported=true"
        ) ==
            std::string::npos) {
        std::cerr << "kcm asymmetric native diagnostics snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    const auto native_overview = kcm_bridge.native_overview(
        fluxma::KwinCompositorFrameInputs {
            .output_id = 0,
            .frame_id = 12,
            .timestamp_ns = 2'000'000,
            .target_presentation_timestamp_ns = 18'000'000,
            .predicted_render_time_ns = 2'000'000,
            .content_type = fluxma::ContentType::Video,
            .width = 1920,
            .height = 1080,
            .gpu_handle = fluxma::GpuFrameHandle {.backend_kind = 1, .handle_id = 212},
        },
        fluxma::KwinPresentFeedbackInputs {
            .output_id = 0,
            .frame_id = 12,
            .presented_timestamp_ns = 3'000'000,
            .refresh_interval_ns = 16'666'667,
            .presentation_mode = fluxma::PresentationMode::VSync,
            .present_success = true,
            .dropped_synthetic = false,
        },
        fluxma::KwinNativeInstallContext {
            .kwin_version_supported = false,
            .backend_supported = true,
            .kwin_version = "6.3.92",
            .backend_name = "drm",
        }
    );
    if (native_overview.atomic ||
        !native_overview.assembled_from_split_reads ||
        native_overview.kwin_version_supported ||
        !native_overview.backend_supported ||
        native_overview.install_context_summary != "6.3.92/drm" ||
        native_overview.provenance_summary != "bringup+diagnostics+install" ||
        native_overview.bringup_provenance !=
            "observe_native_bridge_bringup(frame,present)" ||
        native_overview.diagnostics_provenance !=
            "observe_native_bridge(frame,present,install-context)" ||
        native_overview.install_provenance !=
            "observe_native_bridge_install(install-context)" ||
        native_overview.bringup.state != fluxma::KwinNativeBridgeState::PlaceholderOnly ||
        !native_overview.bringup.frame_complete ||
        !native_overview.bringup.present_complete ||
        !native_overview.diagnostics.bringup_complete ||
        !native_overview.diagnostics.all_gates_match ||
        !native_overview.install.all_gates_match ||
        native_overview.diagnostics.frame_deferred_reason !=
            fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        native_overview.install.frame_deferred_reason !=
            fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        native_overview.summary().find("atomic=no") == std::string::npos ||
        native_overview.summary().find("split-reads=yes") == std::string::npos ||
        native_overview.summary().find("kwin-version-supported=no") ==
            std::string::npos ||
        native_overview.summary().find("backend-supported=yes") ==
            std::string::npos ||
        native_overview.summary().find("install-context=6.3.92/drm") ==
            std::string::npos ||
        native_overview.summary().find("provenance=bringup+diagnostics+install") ==
            std::string::npos ||
        native_overview.summary().find(
            "bringup-source=observe_native_bridge_bringup(frame,present)"
        ) == std::string::npos ||
        native_overview.summary().find(
            "diagnostics-source=observe_native_bridge(frame,present,install-context)"
        ) == std::string::npos ||
        native_overview.summary().find(
            "install-source=observe_native_bridge_install(install-context)"
        ) == std::string::npos ||
        native_overview.summary().find("bringup{state=placeholder-only") ==
            std::string::npos ||
        native_overview.summary().find("frame-complete=yes") == std::string::npos ||
        native_overview.summary().find("all-gates-match=yes") == std::string::npos ||
        native_overview.summary().find("diagnostics{state=placeholder-only") ==
            std::string::npos ||
        native_overview.summary().find("frame-version-blocked=yes") ==
            std::string::npos ||
        native_overview.summary().find("install{state=placeholder-only") ==
            std::string::npos) {
        std::cerr << "kcm native overview snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    const auto placeholder_native_overview = kcm_bridge.native_overview(
        fluxma::KwinCompositorFrameInputs {
            .output_id = 0,
            .frame_id = 12,
            .timestamp_ns = 2'000'000,
            .target_presentation_timestamp_ns = 18'000'000,
            .predicted_render_time_ns = 2'000'000,
            .content_type = fluxma::ContentType::Video,
            .width = 1920,
            .height = 1080,
            .gpu_handle = fluxma::GpuFrameHandle {.backend_kind = 1, .handle_id = 212},
        },
        fluxma::KwinPresentFeedbackInputs {
            .output_id = 0,
            .frame_id = 12,
            .presented_timestamp_ns = 3'000'000,
            .refresh_interval_ns = 16'666'667,
            .presentation_mode = fluxma::PresentationMode::VSync,
            .present_success = true,
            .dropped_synthetic = false,
        },
        fluxma::KwinNativeInstallContext {}
    );
    if (placeholder_native_overview.atomic ||
        !placeholder_native_overview.assembled_from_split_reads ||
        !placeholder_native_overview.kwin_version_supported ||
        !placeholder_native_overview.backend_supported ||
        placeholder_native_overview.install_context_summary != "unspecified/unspecified" ||
        placeholder_native_overview.provenance_summary != "bringup+diagnostics+install" ||
        placeholder_native_overview.bringup_provenance !=
            "observe_native_bridge_bringup(frame,present)" ||
        placeholder_native_overview.diagnostics_provenance !=
            "observe_native_bridge(frame,present,install-context)" ||
        placeholder_native_overview.install_provenance !=
            "observe_native_bridge_install(install-context)" ||
        !placeholder_native_overview.diagnostics.all_gates_match ||
        !placeholder_native_overview.install.all_gates_match ||
        placeholder_native_overview.install.frame_deferred_reason !=
            fluxma::KwinNativeDeferredReason::PlaceholderOnly ||
        placeholder_native_overview.install.present_deferred_reason !=
            fluxma::KwinNativeDeferredReason::PlaceholderOnly ||
        placeholder_native_overview.summary().find("split-reads=yes") ==
            std::string::npos ||
        placeholder_native_overview.summary().find("kwin-version-supported=yes") ==
            std::string::npos ||
        placeholder_native_overview.summary().find("backend-supported=yes") ==
            std::string::npos ||
        placeholder_native_overview.summary().find("install-context=unspecified/unspecified") ==
            std::string::npos ||
        placeholder_native_overview.summary().find("provenance=bringup+diagnostics+install") ==
            std::string::npos ||
        placeholder_native_overview.summary().find(
            "bringup-source=observe_native_bridge_bringup(frame,present)"
        ) == std::string::npos ||
        placeholder_native_overview.summary().find(
            "diagnostics-source=observe_native_bridge(frame,present,install-context)"
        ) == std::string::npos ||
        placeholder_native_overview.summary().find(
            "install-source=observe_native_bridge_install(install-context)"
        ) == std::string::npos ||
        placeholder_native_overview.summary().find("diagnostics{state=placeholder-only") ==
            std::string::npos ||
        placeholder_native_overview.summary().find("frame-deferred=placeholder-only") ==
            std::string::npos) {
        std::cerr << "kcm placeholder native overview snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    const auto backend_native_overview = kcm_bridge.native_overview(
        fluxma::KwinCompositorFrameInputs {
            .output_id = 0,
            .frame_id = 13,
            .timestamp_ns = 2'500'000,
            .target_presentation_timestamp_ns = 18'500'000,
            .predicted_render_time_ns = 2'000'000,
            .content_type = fluxma::ContentType::Video,
            .width = 1920,
            .height = 1080,
            .gpu_handle = fluxma::GpuFrameHandle {.backend_kind = 1, .handle_id = 213},
        },
        fluxma::KwinPresentFeedbackInputs {
            .output_id = 0,
            .frame_id = 13,
            .presented_timestamp_ns = 3'500'000,
            .refresh_interval_ns = 16'666'667,
            .presentation_mode = fluxma::PresentationMode::VSync,
            .present_success = true,
            .dropped_synthetic = false,
        },
        fluxma::KwinNativeInstallContext {
            .kwin_version_supported = true,
            .backend_supported = false,
            .kwin_version = "6.3.93",
            .backend_name = "wayland",
        }
    );
    if (backend_native_overview.atomic ||
        !backend_native_overview.assembled_from_split_reads ||
        !backend_native_overview.kwin_version_supported ||
        backend_native_overview.backend_supported ||
        backend_native_overview.install_context_summary != "6.3.93/wayland" ||
        backend_native_overview.provenance_summary != "bringup+diagnostics+install" ||
        backend_native_overview.bringup_provenance !=
            "observe_native_bridge_bringup(frame,present)" ||
        backend_native_overview.diagnostics_provenance !=
            "observe_native_bridge(frame,present,install-context)" ||
        backend_native_overview.install_provenance !=
            "observe_native_bridge_install(install-context)" ||
        !backend_native_overview.diagnostics.all_gates_match ||
        !backend_native_overview.install.all_gates_match ||
        backend_native_overview.install.frame_deferred_reason !=
            fluxma::KwinNativeDeferredReason::BackendGate ||
        backend_native_overview.install.present_deferred_reason !=
            fluxma::KwinNativeDeferredReason::BackendGate ||
        backend_native_overview.summary().find("split-reads=yes") ==
            std::string::npos ||
        backend_native_overview.summary().find("kwin-version-supported=yes") ==
            std::string::npos ||
        backend_native_overview.summary().find("backend-supported=no") ==
            std::string::npos ||
        backend_native_overview.summary().find("install-context=6.3.93/wayland") ==
            std::string::npos ||
        backend_native_overview.summary().find("provenance=bringup+diagnostics+install") ==
            std::string::npos ||
        backend_native_overview.summary().find(
            "bringup-source=observe_native_bridge_bringup(frame,present)"
        ) == std::string::npos ||
        backend_native_overview.summary().find(
            "diagnostics-source=observe_native_bridge(frame,present,install-context)"
        ) == std::string::npos ||
        backend_native_overview.summary().find(
            "install-source=observe_native_bridge_install(install-context)"
        ) == std::string::npos ||
        backend_native_overview.summary().find("diagnostics{state=placeholder-only") ==
            std::string::npos ||
        backend_native_overview.summary().find("frame-deferred=backend-gate") ==
            std::string::npos) {
        std::cerr << "kcm backend native overview snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    const auto direct_runtime = fluxma::KcmRuntimeSnapshot::from_observation(
        fluxma::OutputRuntimeObservationReport {
            .snapshot = fluxma::MetricsSnapshot {
                .state = fluxma::OutputState::ProtectedBypass,
                .bypass_reason = fluxma::BypassReason::ProtectedContent,
                .protected_content = true,
                .passthrough_only = true,
                .classifier_allows_interpolation = false,
                .frame_tap_count = 9,
                .present_feedback_count = 2,
                .deadline_miss_count = 1,
                .dropped_synthetic_count = 1,
                .last_presented_frame_id = 17,
                .last_presented_timestamp_ns = 123'000'000,
                .refresh_interval_ns = 16'666'667,
                .last_target_presentation_timestamp_ns = 133'000'000,
                .last_predicted_render_time_ns = 3'000'000,
                .last_presentation_mode = fluxma::PresentationMode::AdaptiveSync,
                .last_content_type = fluxma::ContentType::Game,
                .cadence_status = fluxma::CadenceStatus::Unstable,
                .governor_mode = fluxma::GovernorMode::Bypass,
                .scheduler_mode = fluxma::SchedulerMode::PassthroughOnly,
                .cadence_hz_millihz = 59'940,
                .state_transition_count = 4,
            },
            .synthetic_plan = fluxma::SyntheticFramePlan {
                .armed = false,
                .should_drop = false,
            },
            .synthetic_artifact = fluxma::SyntheticFrameArtifact {
                .generated = false,
                .dropped = false,
                .placeholder_only = true,
            },
            .synthetic_submission = fluxma::SyntheticPresentSubmission {
                .queued = false,
                .dropped = false,
                .protected_content = true,
                .protection_plan =
                    fluxma::ProtectionPlan {
                        .cursor_passthrough = false,
                        .cursor_recomposite = true,
                        .subtitle_band_active = false,
                        .transient_overlay_passthrough = true,
                        .placeholder_only = false,
                    },
                .prefer_current_in_subtitle_band = false,
                .suppressed_by_protection = true,
                .placeholder_only = true,
            },
            .protection_plan =
                fluxma::ProtectionPlan {
                    .cursor_passthrough = false,
                    .cursor_recomposite = true,
                    .subtitle_band_active = false,
                    .transient_overlay_passthrough = true,
                    .placeholder_only = false,
                },
            .hud_text = "hud state=protected-bypass scheduler=passthrough-only",
        }
    );
    if (direct_runtime.state != fluxma::OutputState::ProtectedBypass ||
        direct_runtime.bypass_reason != fluxma::BypassReason::ProtectedContent ||
        direct_runtime.governor_mode != fluxma::GovernorMode::Bypass ||
        direct_runtime.scheduler_mode != fluxma::SchedulerMode::PassthroughOnly ||
        direct_runtime.classifier_allows_interpolation ||
        !direct_runtime.protected_content || !direct_runtime.passthrough_only ||
        direct_runtime.synthetic_armed || direct_runtime.synthetic_queued ||
        !direct_runtime.synthetic_suppressed_by_protection ||
        direct_runtime.cursor_passthrough || !direct_runtime.cursor_recomposite ||
        direct_runtime.subtitle_band_active || !direct_runtime.overlay_passthrough ||
        direct_runtime.protection_placeholder_only ||
        direct_runtime.state_transition_count != 4 ||
        direct_runtime.frame_tap_count != 9 ||
        direct_runtime.present_feedback_count != 2 ||
        direct_runtime.deadline_miss_count != 1 ||
        direct_runtime.dropped_synthetic_count != 1 ||
        direct_runtime.last_presented_frame_id != 17 ||
        direct_runtime.last_presented_timestamp_ns != 123'000'000 ||
        direct_runtime.refresh_interval_ns != 16'666'667 ||
        direct_runtime.last_target_presentation_timestamp_ns != 133'000'000 ||
        direct_runtime.last_predicted_render_time_ns != 3'000'000 ||
        direct_runtime.last_presentation_mode != fluxma::PresentationMode::AdaptiveSync ||
        direct_runtime.last_content_type != fluxma::ContentType::Game ||
        direct_runtime.cadence_hz_millihz != 59'940 ||
        direct_runtime.hud_text.find("state=protected-bypass") == std::string::npos ||
        direct_runtime.summary().find("classifier=no") == std::string::npos ||
        direct_runtime.summary().find("cursor-passthrough=no") == std::string::npos ||
        direct_runtime.summary().find("cursor-recomposite=yes") == std::string::npos ||
        direct_runtime.summary().find("overlay-passthrough=yes") == std::string::npos ||
        direct_runtime.summary().find("protection-placeholder=no") == std::string::npos ||
        direct_runtime.summary().find("present-mode=adaptive-sync") == std::string::npos ||
        direct_runtime.summary().find("content-type=game") == std::string::npos) {
        std::cerr << "kcm direct runtime helper snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    const auto direct_disabled_runtime = fluxma::KcmRuntimeSnapshot::from_observation(
        fluxma::OutputRuntimeObservationReport {
            .snapshot = fluxma::MetricsSnapshot {
                .state = fluxma::OutputState::Disabled,
                .bypass_reason = fluxma::BypassReason::Disabled,
                .protected_content = false,
                .passthrough_only = true,
                .classifier_allows_interpolation = false,
                .frame_tap_count = 1,
                .present_feedback_count = 0,
                .deadline_miss_count = 0,
                .dropped_synthetic_count = 0,
                .last_presented_frame_id = 0,
                .last_presented_timestamp_ns = 0,
                .refresh_interval_ns = 0,
                .last_target_presentation_timestamp_ns = 33'333'333,
                .last_predicted_render_time_ns = 2'000'000,
                .last_presentation_mode = fluxma::PresentationMode::VSync,
                .last_content_type = fluxma::ContentType::Video,
                .cadence_status = fluxma::CadenceStatus::Unknown,
                .governor_mode = fluxma::GovernorMode::Bypass,
                .scheduler_mode = fluxma::SchedulerMode::PassthroughOnly,
                .cadence_hz_millihz = 0,
                .state_transition_count = 1,
            },
            .synthetic_plan = fluxma::SyntheticFramePlan {
                .armed = false,
                .should_drop = false,
            },
            .synthetic_artifact = fluxma::SyntheticFrameArtifact {
                .generated = false,
                .dropped = false,
                .placeholder_only = true,
            },
            .synthetic_submission = fluxma::SyntheticPresentSubmission {
                .queued = false,
                .dropped = false,
                .protected_content = false,
                .protection_plan = fluxma::ProtectionPlan {},
                .prefer_current_in_subtitle_band = false,
                .suppressed_by_protection = false,
                .placeholder_only = true,
            },
            .protection_plan = fluxma::ProtectionPlan {},
            .hud_text = "hud state=disabled scheduler=passthrough-only",
        }
    );
    if (direct_disabled_runtime.state != fluxma::OutputState::Disabled ||
        direct_disabled_runtime.bypass_reason != fluxma::BypassReason::Disabled ||
        direct_disabled_runtime.governor_mode != fluxma::GovernorMode::Bypass ||
        direct_disabled_runtime.scheduler_mode != fluxma::SchedulerMode::PassthroughOnly ||
        direct_disabled_runtime.classifier_allows_interpolation ||
        direct_disabled_runtime.protected_content || !direct_disabled_runtime.passthrough_only ||
        direct_disabled_runtime.synthetic_armed || direct_disabled_runtime.synthetic_queued ||
        direct_disabled_runtime.synthetic_suppressed_by_protection ||
        direct_disabled_runtime.cursor_passthrough || direct_disabled_runtime.cursor_recomposite ||
        direct_disabled_runtime.subtitle_band_active || direct_disabled_runtime.overlay_passthrough ||
        !direct_disabled_runtime.protection_placeholder_only ||
        direct_disabled_runtime.state_transition_count != 1 ||
        direct_disabled_runtime.frame_tap_count != 1 ||
        direct_disabled_runtime.present_feedback_count != 0 ||
        direct_disabled_runtime.deadline_miss_count != 0 ||
        direct_disabled_runtime.dropped_synthetic_count != 0 ||
        direct_disabled_runtime.last_presented_frame_id != 0 ||
        direct_disabled_runtime.last_presented_timestamp_ns != 0 ||
        direct_disabled_runtime.refresh_interval_ns != 0 ||
        direct_disabled_runtime.last_target_presentation_timestamp_ns != 33'333'333 ||
        direct_disabled_runtime.last_predicted_render_time_ns != 2'000'000 ||
        direct_disabled_runtime.last_presentation_mode != fluxma::PresentationMode::VSync ||
        direct_disabled_runtime.last_content_type != fluxma::ContentType::Video ||
        direct_disabled_runtime.cadence_hz_millihz != 0 ||
        direct_disabled_runtime.hud_text.find("state=disabled") == std::string::npos ||
        direct_disabled_runtime.summary().find("classifier=no") == std::string::npos ||
        direct_disabled_runtime.summary().find("cursor-passthrough=no") ==
            std::string::npos ||
        direct_disabled_runtime.summary().find("overlay-passthrough=no") ==
            std::string::npos ||
        direct_disabled_runtime.summary().find("protection-placeholder=yes") ==
            std::string::npos ||
        direct_disabled_runtime.summary().find("present-mode=vsync") ==
            std::string::npos ||
        direct_disabled_runtime.summary().find("content-type=video") ==
            std::string::npos) {
        std::cerr << "kcm direct disabled runtime helper snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    const auto direct_active_runtime = fluxma::KcmRuntimeSnapshot::from_observation(
        fluxma::OutputRuntimeObservationReport {
            .snapshot = fluxma::MetricsSnapshot {
                .state = fluxma::OutputState::Active2x,
                .bypass_reason = fluxma::BypassReason::None,
                .protected_content = false,
                .passthrough_only = true,
                .classifier_allows_interpolation = true,
                .frame_tap_count = 5,
                .present_feedback_count = 1,
                .deadline_miss_count = 0,
                .dropped_synthetic_count = 0,
                .last_presented_frame_id = 5,
                .last_presented_timestamp_ns = 166'666'665,
                .refresh_interval_ns = 16'666'667,
                .last_target_presentation_timestamp_ns = 166'666'665,
                .last_predicted_render_time_ns = 2'000'000,
                .last_presentation_mode = fluxma::PresentationMode::VSync,
                .last_content_type = fluxma::ContentType::Video,
                .cadence_status = fluxma::CadenceStatus::Stable,
                .governor_mode = fluxma::GovernorMode::QualityHigh,
                .scheduler_mode = fluxma::SchedulerMode::Synthetic2x,
                .cadence_hz_millihz = 30'000,
                .state_transition_count = 3,
            },
            .synthetic_plan = fluxma::SyntheticFramePlan {
                .armed = true,
                .should_drop = false,
            },
            .synthetic_artifact = fluxma::SyntheticFrameArtifact {
                .generated = true,
                .dropped = false,
                .placeholder_only = true,
            },
            .synthetic_submission = fluxma::SyntheticPresentSubmission {
                .queued = true,
                .dropped = false,
                .protected_content = false,
                .protection_plan =
                    fluxma::ProtectionPlan {
                        .cursor_passthrough = true,
                        .cursor_recomposite = true,
                        .subtitle_band_active = true,
                        .transient_overlay_passthrough = true,
                        .placeholder_only = true,
                    },
                .prefer_current_in_subtitle_band = true,
                .suppressed_by_protection = false,
                .placeholder_only = true,
            },
            .protection_plan =
                fluxma::ProtectionPlan {
                    .cursor_passthrough = true,
                    .cursor_recomposite = true,
                    .subtitle_band_active = true,
                    .transient_overlay_passthrough = true,
                    .placeholder_only = true,
                },
            .hud_text = "hud state=active-2x scheduler=synthetic-2x",
        }
    );
    if (direct_active_runtime.state != fluxma::OutputState::Active2x ||
        direct_active_runtime.bypass_reason != fluxma::BypassReason::None ||
        direct_active_runtime.governor_mode != fluxma::GovernorMode::QualityHigh ||
        direct_active_runtime.scheduler_mode != fluxma::SchedulerMode::Synthetic2x ||
        !direct_active_runtime.classifier_allows_interpolation ||
        direct_active_runtime.protected_content || !direct_active_runtime.passthrough_only ||
        !direct_active_runtime.synthetic_armed || !direct_active_runtime.synthetic_queued ||
        direct_active_runtime.synthetic_suppressed_by_protection ||
        !direct_active_runtime.cursor_passthrough || !direct_active_runtime.cursor_recomposite ||
        !direct_active_runtime.subtitle_band_active || !direct_active_runtime.overlay_passthrough ||
        !direct_active_runtime.protection_placeholder_only ||
        direct_active_runtime.state_transition_count != 3 ||
        direct_active_runtime.frame_tap_count != 5 ||
        direct_active_runtime.present_feedback_count != 1 ||
        direct_active_runtime.deadline_miss_count != 0 ||
        direct_active_runtime.dropped_synthetic_count != 0 ||
        direct_active_runtime.last_presented_frame_id != 5 ||
        direct_active_runtime.last_presented_timestamp_ns != 166'666'665 ||
        direct_active_runtime.refresh_interval_ns != 16'666'667 ||
        direct_active_runtime.last_target_presentation_timestamp_ns != 166'666'665 ||
        direct_active_runtime.last_predicted_render_time_ns != 2'000'000 ||
        direct_active_runtime.last_presentation_mode != fluxma::PresentationMode::VSync ||
        direct_active_runtime.last_content_type != fluxma::ContentType::Video ||
        direct_active_runtime.cadence_hz_millihz != 30'000 ||
        direct_active_runtime.hud_text.find("state=active-2x") == std::string::npos ||
        direct_active_runtime.summary().find("classifier=yes") == std::string::npos ||
        direct_active_runtime.summary().find("cursor-passthrough=yes") ==
            std::string::npos ||
        direct_active_runtime.summary().find("subtitle-band=yes") == std::string::npos ||
        direct_active_runtime.summary().find("overlay-passthrough=yes") ==
            std::string::npos ||
        direct_active_runtime.summary().find("protection-placeholder=yes") ==
            std::string::npos) {
        std::cerr << "kcm direct active runtime helper snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    const auto direct_degraded_runtime = fluxma::KcmRuntimeSnapshot::from_observation(
        fluxma::OutputRuntimeObservationReport {
            .snapshot = fluxma::MetricsSnapshot {
                .state = fluxma::OutputState::Degraded,
                .bypass_reason = fluxma::BypassReason::GpuPathNotReady,
                .protected_content = false,
                .passthrough_only = true,
                .classifier_allows_interpolation = true,
                .frame_tap_count = 7,
                .present_feedback_count = 2,
                .deadline_miss_count = 2,
                .dropped_synthetic_count = 1,
                .last_presented_frame_id = 7,
                .last_presented_timestamp_ns = 233'333'331,
                .refresh_interval_ns = 16'666'667,
                .last_target_presentation_timestamp_ns = 233'333'331,
                .last_predicted_render_time_ns = 2'500'000,
                .last_presentation_mode = fluxma::PresentationMode::AdaptiveSync,
                .last_content_type = fluxma::ContentType::Video,
                .cadence_status = fluxma::CadenceStatus::Unstable,
                .governor_mode = fluxma::GovernorMode::QualityLow,
                .scheduler_mode = fluxma::SchedulerMode::Synthetic2x,
                .cadence_hz_millihz = 24'000,
                .state_transition_count = 5,
            },
            .synthetic_plan = fluxma::SyntheticFramePlan {
                .armed = true,
                .should_drop = true,
            },
            .synthetic_artifact = fluxma::SyntheticFrameArtifact {
                .generated = false,
                .dropped = true,
                .placeholder_only = true,
            },
            .synthetic_submission = fluxma::SyntheticPresentSubmission {
                .queued = false,
                .dropped = true,
                .protected_content = false,
                .protection_plan =
                    fluxma::ProtectionPlan {
                        .cursor_passthrough = true,
                        .cursor_recomposite = true,
                        .subtitle_band_active = false,
                        .transient_overlay_passthrough = true,
                        .placeholder_only = true,
                    },
                .prefer_current_in_subtitle_band = false,
                .suppressed_by_protection = false,
                .placeholder_only = true,
            },
            .protection_plan =
                fluxma::ProtectionPlan {
                    .cursor_passthrough = true,
                    .cursor_recomposite = true,
                    .subtitle_band_active = false,
                    .transient_overlay_passthrough = true,
                    .placeholder_only = true,
                },
            .hud_text = "hud state=degraded scheduler=synthetic-2x",
        }
    );
    if (direct_degraded_runtime.state != fluxma::OutputState::Degraded ||
        direct_degraded_runtime.bypass_reason != fluxma::BypassReason::GpuPathNotReady ||
        direct_degraded_runtime.governor_mode != fluxma::GovernorMode::QualityLow ||
        direct_degraded_runtime.scheduler_mode != fluxma::SchedulerMode::Synthetic2x ||
        !direct_degraded_runtime.classifier_allows_interpolation ||
        direct_degraded_runtime.protected_content || !direct_degraded_runtime.passthrough_only ||
        !direct_degraded_runtime.synthetic_armed || direct_degraded_runtime.synthetic_queued ||
        direct_degraded_runtime.synthetic_suppressed_by_protection ||
        !direct_degraded_runtime.cursor_passthrough ||
        !direct_degraded_runtime.cursor_recomposite ||
        direct_degraded_runtime.subtitle_band_active ||
        !direct_degraded_runtime.overlay_passthrough ||
        !direct_degraded_runtime.protection_placeholder_only ||
        direct_degraded_runtime.state_transition_count != 5 ||
        direct_degraded_runtime.frame_tap_count != 7 ||
        direct_degraded_runtime.present_feedback_count != 2 ||
        direct_degraded_runtime.deadline_miss_count != 2 ||
        direct_degraded_runtime.dropped_synthetic_count != 1 ||
        direct_degraded_runtime.last_presented_frame_id != 7 ||
        direct_degraded_runtime.last_presented_timestamp_ns != 233'333'331 ||
        direct_degraded_runtime.refresh_interval_ns != 16'666'667 ||
        direct_degraded_runtime.last_target_presentation_timestamp_ns != 233'333'331 ||
        direct_degraded_runtime.last_predicted_render_time_ns != 2'500'000 ||
        direct_degraded_runtime.last_presentation_mode !=
            fluxma::PresentationMode::AdaptiveSync ||
        direct_degraded_runtime.last_content_type != fluxma::ContentType::Video ||
        direct_degraded_runtime.cadence_hz_millihz != 24'000 ||
        direct_degraded_runtime.hud_text.find("state=degraded") == std::string::npos ||
        direct_degraded_runtime.summary().find("synthetic-queued=no") ==
            std::string::npos ||
        direct_degraded_runtime.summary().find("deadline-miss=2") == std::string::npos ||
        direct_degraded_runtime.summary().find("synthetic-dropped=1") ==
            std::string::npos ||
        direct_degraded_runtime.summary().find("governor=quality-low") ==
            std::string::npos) {
        std::cerr << "kcm direct degraded runtime helper snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    for (std::uint64_t frame_id = 1; frame_id <= 5; ++frame_id) {
        const auto _ = plugin_root.primary_output().on_frame_tapped(frame(frame_id));
    }
    plugin_root.primary_output().on_present_feedback(
        fluxma::PresentFeedback {
            .frame_id = 5,
            .presented_timestamp_ns = 166'666'665,
            .refresh_interval_ns = 16'666'667,
            .presentation_mode = fluxma::PresentationMode::VSync,
            .present_success = true,
            .dropped_synthetic = false,
        }
    );

    const auto runtime = kcm_bridge.runtime(47'999'999);
    if (runtime.state != fluxma::OutputState::Active2x ||
        runtime.bypass_reason != fluxma::BypassReason::None ||
        runtime.governor_mode != fluxma::GovernorMode::QualityHigh ||
        runtime.scheduler_mode != fluxma::SchedulerMode::Synthetic2x ||
        !runtime.classifier_allows_interpolation ||
        runtime.protected_content || !runtime.passthrough_only ||
        !runtime.synthetic_armed || !runtime.synthetic_queued ||
        runtime.synthetic_suppressed_by_protection ||
        !runtime.cursor_passthrough || !runtime.cursor_recomposite ||
        !runtime.subtitle_band_active || !runtime.overlay_passthrough ||
        !runtime.protection_placeholder_only ||
        runtime.state_transition_count != 3 ||
        runtime.frame_tap_count != 5 || runtime.present_feedback_count != 1 ||
        runtime.deadline_miss_count != 0 || runtime.dropped_synthetic_count != 0 ||
        runtime.last_presented_frame_id != 5 ||
        runtime.last_presented_timestamp_ns != 166'666'665 ||
        runtime.refresh_interval_ns != 16'666'667 ||
        runtime.last_target_presentation_timestamp_ns != 166'666'665 ||
        runtime.last_predicted_render_time_ns != 2'000'000 ||
        runtime.last_presentation_mode != fluxma::PresentationMode::VSync ||
        runtime.last_content_type != fluxma::ContentType::Video ||
        runtime.cadence_hz_millihz != 30000 ||
        runtime.hud_text.find("scheduler=synthetic-2x") == std::string::npos ||
        runtime.summary().find("synthetic-armed=yes") == std::string::npos ||
        runtime.summary().find("classifier=yes") == std::string::npos ||
        runtime.summary().find("cursor-passthrough=yes") == std::string::npos ||
        runtime.summary().find("subtitle-band=yes") == std::string::npos ||
        runtime.summary().find("overlay-passthrough=yes") == std::string::npos ||
        runtime.summary().find("protection-placeholder=yes") == std::string::npos ||
        runtime.summary().find("state-transitions=") == std::string::npos ||
        runtime.summary().find("last-presented-frame=5") == std::string::npos ||
        runtime.summary().find("last-presented-ts=166666665") == std::string::npos ||
        runtime.summary().find("target-present-ns=166666665") == std::string::npos ||
        runtime.summary().find("predicted-render-ns=2000000") == std::string::npos ||
        runtime.summary().find("present-mode=vsync") == std::string::npos ||
        runtime.summary().find("content-type=video") == std::string::npos ||
        runtime.summary().find("governor=quality-high") == std::string::npos ||
        runtime.summary().find("scheduler=synthetic-2x") == std::string::npos ||
        runtime.summary().find("frame-taps=5") == std::string::npos ||
        runtime.summary().find("present-feedback=1") == std::string::npos ||
        runtime.summary().find("synthetic-suppressed-by-protection=no") ==
            std::string::npos) {
        std::cerr << "kcm runtime snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    fluxma::KfiPluginRoot protected_root(config);
    const fluxma::KfiKcmBridge protected_bridge(protected_root);
    for (std::uint64_t frame_id = 1; frame_id <= 5; ++frame_id) {
        const auto _ =
            protected_root.primary_output().on_frame_tapped(frame(frame_id, frame_id == 5));
    }
    const auto protected_runtime = protected_bridge.runtime(200'000'000);
    if (protected_runtime.state != fluxma::OutputState::ProtectedBypass ||
        protected_runtime.bypass_reason != fluxma::BypassReason::ProtectedContent ||
        protected_runtime.governor_mode != fluxma::GovernorMode::Bypass ||
        protected_runtime.scheduler_mode != fluxma::SchedulerMode::PassthroughOnly ||
        protected_runtime.classifier_allows_interpolation ||
        !protected_runtime.protected_content || !protected_runtime.passthrough_only ||
        protected_runtime.synthetic_armed || protected_runtime.synthetic_queued ||
        !protected_runtime.synthetic_suppressed_by_protection ||
        !protected_runtime.cursor_passthrough || !protected_runtime.cursor_recomposite ||
        protected_runtime.subtitle_band_active || !protected_runtime.overlay_passthrough ||
        !protected_runtime.protection_placeholder_only ||
        protected_runtime.state_transition_count != 3 ||
        protected_runtime.frame_tap_count != 5 ||
        protected_runtime.present_feedback_count != 0 ||
        protected_runtime.deadline_miss_count != 0 ||
        protected_runtime.dropped_synthetic_count != 0 ||
        protected_runtime.last_presented_frame_id != 0 ||
        protected_runtime.last_presented_timestamp_ns != 0 ||
        protected_runtime.refresh_interval_ns != 0 ||
        protected_runtime.last_target_presentation_timestamp_ns != 166'666'665 ||
        protected_runtime.last_predicted_render_time_ns != 2'000'000 ||
        protected_runtime.last_presentation_mode != fluxma::PresentationMode::VSync ||
        protected_runtime.last_content_type != fluxma::ContentType::Video ||
        protected_runtime.summary().find("synthetic-suppressed-by-protection=yes") ==
            std::string::npos ||
        protected_runtime.summary().find("classifier=no") == std::string::npos ||
        protected_runtime.summary().find("cursor-passthrough=yes") ==
            std::string::npos ||
        protected_runtime.summary().find("subtitle-band=no") == std::string::npos ||
        protected_runtime.summary().find("overlay-passthrough=yes") ==
            std::string::npos ||
        protected_runtime.summary().find("protection-placeholder=yes") ==
            std::string::npos ||
        protected_runtime.summary().find("state-transitions=") == std::string::npos ||
        protected_runtime.summary().find("last-presented-frame=0") == std::string::npos ||
        protected_runtime.summary().find("last-presented-ts=0") == std::string::npos ||
        protected_runtime.summary().find("target-present-ns=166666665") ==
            std::string::npos ||
        protected_runtime.summary().find("predicted-render-ns=2000000") ==
            std::string::npos ||
        protected_runtime.summary().find("present-mode=vsync") == std::string::npos ||
        protected_runtime.summary().find("content-type=video") == std::string::npos ||
        protected_runtime.summary().find("governor=bypass") == std::string::npos ||
        protected_runtime.summary().find("scheduler=passthrough-only") ==
            std::string::npos ||
        protected_runtime.summary().find("frame-taps=5") == std::string::npos ||
        protected_runtime.summary().find("present-feedback=0") == std::string::npos ||
        protected_runtime.hud_text.find("protected=yes") == std::string::npos) {
        std::cerr << "kcm protected runtime snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    const fluxma::ModuleConfig disabled_config {
        .enabled = false,
        .mode = fluxma::ModuleMode::PassthroughOnly,
        .show_hud = false,
        .subtitle_protection = false,
        .cursor_protection = false,
        .log_interval_frames = 3,
        .max_log_messages = 5,
    };
    fluxma::KfiPluginRoot disabled_root(disabled_config);
    const fluxma::KfiKcmBridge disabled_bridge(disabled_root);
    const auto direct_disabled_settings =
        fluxma::KcmSettingsSnapshot::from_plugin_root(disabled_root);
    if (direct_disabled_settings.enabled ||
        direct_disabled_settings.mode != fluxma::ModuleMode::PassthroughOnly ||
        direct_disabled_settings.show_hud ||
        direct_disabled_settings.subtitle_protection ||
        direct_disabled_settings.cursor_protection ||
        direct_disabled_settings.log_interval_frames != 3 ||
        direct_disabled_settings.max_log_messages != 5 ||
        direct_disabled_settings.summary().find("mode=passthrough-only") ==
            std::string::npos ||
        direct_disabled_settings.summary().find("show-hud=no") == std::string::npos) {
        std::cerr << "kcm direct disabled settings helper mismatch\n";
        return EXIT_FAILURE;
    }
    const auto disabled_settings = disabled_bridge.settings();
    if (disabled_settings.enabled ||
        disabled_settings.mode != fluxma::ModuleMode::PassthroughOnly ||
        disabled_settings.show_hud ||
        disabled_settings.subtitle_protection || disabled_settings.cursor_protection ||
        disabled_settings.log_interval_frames != 3 ||
        disabled_settings.summary().find("mode=passthrough-only") ==
            std::string::npos ||
        disabled_settings.summary().find("show-hud=no") == std::string::npos ||
        disabled_settings.max_log_messages != 5) {
        std::cerr << "kcm disabled settings snapshot mismatch\n";
        return EXIT_FAILURE;
    }
    const auto _ = disabled_root.primary_output().on_frame_tapped(frame(1));
    const auto disabled_runtime = disabled_bridge.runtime(10'000'000);
    if (disabled_runtime.state != fluxma::OutputState::Disabled ||
        disabled_runtime.bypass_reason != fluxma::BypassReason::Disabled ||
        disabled_runtime.governor_mode != fluxma::GovernorMode::Bypass ||
        disabled_runtime.scheduler_mode != fluxma::SchedulerMode::PassthroughOnly ||
        disabled_runtime.classifier_allows_interpolation ||
        disabled_runtime.state_transition_count != 1 ||
        !disabled_runtime.passthrough_only ||
        disabled_runtime.cursor_passthrough || disabled_runtime.cursor_recomposite ||
        disabled_runtime.subtitle_band_active || disabled_runtime.overlay_passthrough ||
        !disabled_runtime.protection_placeholder_only ||
        disabled_runtime.last_presentation_mode != fluxma::PresentationMode::VSync ||
        disabled_runtime.last_presented_frame_id != 0 ||
        disabled_runtime.last_presented_timestamp_ns != 0 ||
        disabled_runtime.refresh_interval_ns != 0 ||
        disabled_runtime.last_target_presentation_timestamp_ns != 33'333'333 ||
        disabled_runtime.last_predicted_render_time_ns != 2'000'000 ||
        disabled_runtime.last_content_type != fluxma::ContentType::Video ||
        disabled_runtime.summary().find("classifier=no") == std::string::npos ||
        disabled_runtime.summary().find("cursor-passthrough=no") ==
            std::string::npos ||
        disabled_runtime.summary().find("subtitle-band=no") == std::string::npos ||
        disabled_runtime.summary().find("overlay-passthrough=no") ==
            std::string::npos ||
        disabled_runtime.summary().find("protection-placeholder=yes") ==
            std::string::npos ||
        disabled_runtime.summary().find("state-transitions=") == std::string::npos ||
        disabled_runtime.summary().find("last-presented-frame=0") == std::string::npos ||
        disabled_runtime.summary().find("last-presented-ts=0") == std::string::npos ||
        disabled_runtime.summary().find("target-present-ns=33333333") ==
            std::string::npos ||
        disabled_runtime.summary().find("predicted-render-ns=2000000") ==
            std::string::npos ||
        disabled_runtime.summary().find("present-mode=vsync") == std::string::npos ||
        disabled_runtime.summary().find("content-type=video") == std::string::npos ||
        disabled_runtime.summary().find("governor=bypass") == std::string::npos ||
        disabled_runtime.summary().find("scheduler=passthrough-only") ==
            std::string::npos) {
        std::cerr << "kcm disabled runtime snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    const auto disabled_overview = disabled_bridge.overview(
        10'000'000,
        fluxma::KwinNativeInstallContext {}
    );
    if (disabled_overview.atomic ||
        !disabled_overview.assembled_from_split_reads ||
        disabled_overview.runtime_observed_at_ns != 10'000'000 ||
        disabled_overview.provenance_summary != "settings+runtime+native-install" ||
        disabled_overview.settings_provenance != "settings()" ||
        disabled_overview.runtime_provenance != "runtime(now-ns)" ||
        disabled_overview.native_install_provenance !=
            "native_bridge_install(install-context)" ||
        disabled_overview.settings.enabled ||
        disabled_overview.settings.mode != fluxma::ModuleMode::PassthroughOnly ||
        disabled_overview.runtime.state != fluxma::OutputState::Disabled ||
        disabled_overview.runtime.bypass_reason != fluxma::BypassReason::Disabled ||
        disabled_overview.native_install.state != fluxma::KwinNativeBridgeState::PlaceholderOnly ||
        disabled_overview.native_install.frame_deferred_reason !=
            fluxma::KwinNativeDeferredReason::PlaceholderOnly ||
        disabled_overview.native_install.present_deferred_reason !=
            fluxma::KwinNativeDeferredReason::PlaceholderOnly ||
        !disabled_overview.native_install.all_gates_match ||
        !disabled_overview.native_install.all_installs_deferred ||
        disabled_overview.summary().find("atomic=no") == std::string::npos ||
        disabled_overview.summary().find("split-reads=yes") == std::string::npos ||
        disabled_overview.summary().find("runtime-observed-at-ns=10000000") ==
            std::string::npos ||
        disabled_overview.summary().find("provenance=settings+runtime+native-install") ==
            std::string::npos ||
        disabled_overview.summary().find("settings-source=settings()") ==
            std::string::npos ||
        disabled_overview.summary().find("runtime-source=runtime(now-ns)") ==
            std::string::npos ||
        disabled_overview.summary().find(
            "native-install-source=native_bridge_install(install-context)"
        ) == std::string::npos ||
        disabled_overview.summary().find("settings{enabled=no") == std::string::npos ||
        disabled_overview.summary().find("runtime{state=disabled") == std::string::npos ||
        disabled_overview.summary().find("native-install{state=placeholder-only") ==
            std::string::npos) {
        std::cerr << "kcm disabled overview snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    const auto placeholder_native_bridge =
        kcm_bridge.native_bridge_install(fluxma::KwinNativeInstallContext {});
    if (placeholder_native_bridge.state != fluxma::KwinNativeBridgeState::PlaceholderOnly ||
        !placeholder_native_bridge.all_gates_match ||
        !placeholder_native_bridge.all_installs_deferred ||
        !placeholder_native_bridge.kwin_version_supported ||
        !placeholder_native_bridge.backend_supported ||
        placeholder_native_bridge.install_context_summary !=
            "frame=kwin=unspecified backend=unspecified version_supported=true backend_supported=true" ||
        placeholder_native_bridge.frame_install_context_summary !=
            "kwin=unspecified backend=unspecified version_supported=true backend_supported=true" ||
        placeholder_native_bridge.present_install_context_summary !=
            "kwin=unspecified backend=unspecified version_supported=true backend_supported=true" ||
        placeholder_native_bridge.frame_deferred_reason !=
            fluxma::KwinNativeDeferredReason::PlaceholderOnly ||
        placeholder_native_bridge.present_deferred_reason !=
            fluxma::KwinNativeDeferredReason::PlaceholderOnly ||
        placeholder_native_bridge.frame_version_blocked ||
        placeholder_native_bridge.present_version_blocked ||
        placeholder_native_bridge.frame_backend_blocked ||
        placeholder_native_bridge.present_backend_blocked ||
        placeholder_native_bridge.summary().find("all-gates-match=yes") ==
            std::string::npos ||
        placeholder_native_bridge.summary().find("kwin-version-supported=yes") ==
            std::string::npos ||
        placeholder_native_bridge.summary().find("backend-supported=yes") ==
            std::string::npos ||
        placeholder_native_bridge.summary().find(
            "install-context=frame=kwin=unspecified backend=unspecified version_supported=true backend_supported=true"
        ) == std::string::npos ||
        placeholder_native_bridge.summary().find(
            "frame-install-context=kwin=unspecified backend=unspecified version_supported=true backend_supported=true"
        ) == std::string::npos ||
        placeholder_native_bridge.summary().find(
            "present-install-context=kwin=unspecified backend=unspecified version_supported=true backend_supported=true"
        ) == std::string::npos ||
        placeholder_native_bridge.summary().find("frame-deferred=placeholder-only") ==
            std::string::npos ||
        placeholder_native_bridge.install_summary.find("install{") == std::string::npos) {
        std::cerr << "kcm native bridge placeholder snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    const auto backend_blocked_native_bridge = kcm_bridge.native_bridge_install(
        fluxma::KwinNativeInstallContext {
            .kwin_version_supported = true,
            .backend_supported = false,
            .kwin_version = "6.3.90",
            .backend_name = "wayland",
        }
    );
    if (backend_blocked_native_bridge.frame_deferred_reason !=
            fluxma::KwinNativeDeferredReason::BackendGate ||
        backend_blocked_native_bridge.present_deferred_reason !=
            fluxma::KwinNativeDeferredReason::BackendGate ||
        backend_blocked_native_bridge.state !=
            fluxma::KwinNativeBridgeState::PlaceholderOnly ||
        !backend_blocked_native_bridge.all_gates_match ||
        !backend_blocked_native_bridge.all_installs_deferred ||
        !backend_blocked_native_bridge.kwin_version_supported ||
        backend_blocked_native_bridge.backend_supported ||
        backend_blocked_native_bridge.install_context_summary !=
            "frame=kwin=6.3.90 backend=wayland version_supported=true backend_supported=false" ||
        backend_blocked_native_bridge.frame_install_context_summary !=
            "kwin=6.3.90 backend=wayland version_supported=true backend_supported=false" ||
        backend_blocked_native_bridge.present_install_context_summary !=
            "kwin=6.3.90 backend=wayland version_supported=true backend_supported=false" ||
        backend_blocked_native_bridge.frame_version_blocked ||
        backend_blocked_native_bridge.present_version_blocked ||
        !backend_blocked_native_bridge.frame_backend_blocked ||
        !backend_blocked_native_bridge.present_backend_blocked ||
        backend_blocked_native_bridge.summary().find("kwin-version-supported=yes") ==
            std::string::npos ||
        backend_blocked_native_bridge.summary().find("backend-supported=no") ==
            std::string::npos ||
        backend_blocked_native_bridge.summary().find(
            "install-context=frame=kwin=6.3.90 backend=wayland version_supported=true backend_supported=false"
        ) == std::string::npos ||
        backend_blocked_native_bridge.summary().find(
            "frame-install-context=kwin=6.3.90 backend=wayland version_supported=true backend_supported=false"
        ) == std::string::npos ||
        backend_blocked_native_bridge.summary().find(
            "present-install-context=kwin=6.3.90 backend=wayland version_supported=true backend_supported=false"
        ) == std::string::npos ||
        backend_blocked_native_bridge.summary().find("frame-backend-blocked=yes") ==
            std::string::npos ||
        backend_blocked_native_bridge.install_summary.find("backend gate blocked install for wayland")
            == std::string::npos) {
        std::cerr << "kcm native bridge backend gate snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    const auto asymmetric_native_bridge = fluxma::KcmNativeBridgeSnapshot::from_observation(
        fluxma::KwinNativeBridgeInstallObservationReport {
            .state = fluxma::KwinNativeBridgeState::PlaceholderOnly,
            .preflight = fluxma::KwinNativeCombinedPreflightReport {
                .frame = fluxma::KwinNativeInstallPreflightReport {
                    .gate = fluxma::KwinNativeInstallGateAssessment {
                        .deferred_reason = fluxma::KwinNativeDeferredReason::PlaceholderOnly,
                        .reason = "frame placeholder path",
                        .context_summary =
                            "kwin=6.3.93 backend=wayland version_supported=true backend_supported=false",
                        .version_blocked = false,
                        .backend_blocked = true,
                    },
                },
                .present = fluxma::KwinNativeInstallPreflightReport {
                    .gate = fluxma::KwinNativeInstallGateAssessment {
                        .deferred_reason = fluxma::KwinNativeDeferredReason::KwinVersionGate,
                        .reason = "present version gate",
                        .context_summary =
                            "kwin=6.3.92 backend=drm version_supported=false backend_supported=true",
                        .version_blocked = true,
                        .backend_blocked = false,
                    },
                },
            },
            .install = fluxma::KwinNativeCombinedInstallReport {
                .frame = fluxma::KwinNativeInstallReport {
                    .result = fluxma::KwinNativeInstallResult::Deferred,
                    .deferred_reason = fluxma::KwinNativeDeferredReason::PlaceholderOnly,
                },
                .present = fluxma::KwinNativeInstallReport {
                    .result = fluxma::KwinNativeInstallResult::Installed,
                    .deferred_reason = fluxma::KwinNativeDeferredReason::PlaceholderOnly,
                },
            },
        }
    );
    if (asymmetric_native_bridge.state != fluxma::KwinNativeBridgeState::PlaceholderOnly ||
        asymmetric_native_bridge.all_gates_match ||
        asymmetric_native_bridge.all_installs_deferred ||
        asymmetric_native_bridge.kwin_version_supported ||
        asymmetric_native_bridge.backend_supported ||
        asymmetric_native_bridge.install_context_summary !=
            "frame=kwin=6.3.93 backend=wayland version_supported=true backend_supported=false present=kwin=6.3.92 backend=drm version_supported=false backend_supported=true" ||
        asymmetric_native_bridge.frame_install_context_summary !=
            "kwin=6.3.93 backend=wayland version_supported=true backend_supported=false" ||
        asymmetric_native_bridge.present_install_context_summary !=
            "kwin=6.3.92 backend=drm version_supported=false backend_supported=true" ||
        asymmetric_native_bridge.frame_deferred_reason !=
            fluxma::KwinNativeDeferredReason::PlaceholderOnly ||
        asymmetric_native_bridge.present_deferred_reason !=
            fluxma::KwinNativeDeferredReason::PlaceholderOnly ||
        asymmetric_native_bridge.frame_version_blocked ||
        !asymmetric_native_bridge.present_version_blocked ||
        !asymmetric_native_bridge.frame_backend_blocked ||
        asymmetric_native_bridge.present_backend_blocked ||
        asymmetric_native_bridge.summary().find("kwin-version-supported=no") ==
            std::string::npos ||
        asymmetric_native_bridge.summary().find("backend-supported=no") ==
            std::string::npos ||
        asymmetric_native_bridge.summary().find(
            "install-context=frame=kwin=6.3.93 backend=wayland version_supported=true backend_supported=false present=kwin=6.3.92 backend=drm version_supported=false backend_supported=true"
        ) == std::string::npos ||
        asymmetric_native_bridge.summary().find(
            "frame-install-context=kwin=6.3.93 backend=wayland version_supported=true backend_supported=false"
        ) == std::string::npos ||
        asymmetric_native_bridge.summary().find(
            "present-install-context=kwin=6.3.92 backend=drm version_supported=false backend_supported=true"
        ) == std::string::npos ||
        asymmetric_native_bridge.summary().find("all-gates-match=no") ==
            std::string::npos ||
        asymmetric_native_bridge.summary().find("all-installs-deferred=no") ==
            std::string::npos ||
        asymmetric_native_bridge.summary().find("frame-backend-blocked=yes") ==
            std::string::npos ||
        asymmetric_native_bridge.summary().find("present-backend-blocked=no") ==
            std::string::npos ||
        asymmetric_native_bridge.summary().find("frame-version-blocked=no") ==
            std::string::npos ||
        asymmetric_native_bridge.summary().find("present-version-blocked=yes") ==
            std::string::npos) {
        std::cerr << "kcm asymmetric native bridge snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    fluxma::KfiPluginRoot degraded_root(config);
    const fluxma::KfiKcmBridge degraded_bridge(degraded_root);
    for (std::uint64_t frame_id = 1; frame_id <= 5; ++frame_id) {
        const auto _ = degraded_root.primary_output().on_frame_tapped(frame(frame_id));
    }
    degraded_root.primary_output().on_present_feedback(
        fluxma::PresentFeedback {
            .frame_id = 5,
            .presented_timestamp_ns = 166'666'665,
            .refresh_interval_ns = 16'666'667,
            .presentation_mode = fluxma::PresentationMode::VSync,
            .present_success = false,
            .dropped_synthetic = true,
        }
    );
    const auto degraded_runtime = degraded_bridge.runtime(200'000'000);
    if (degraded_runtime.frame_tap_count != 5 ||
        degraded_runtime.present_feedback_count != 1 ||
        degraded_runtime.deadline_miss_count != 1 ||
        degraded_runtime.dropped_synthetic_count != 1 ||
        degraded_runtime.governor_mode != fluxma::GovernorMode::QualityHigh ||
        degraded_runtime.scheduler_mode != fluxma::SchedulerMode::Synthetic2x ||
        !degraded_runtime.classifier_allows_interpolation ||
        degraded_runtime.state_transition_count != 3 ||
        !degraded_runtime.cursor_passthrough || !degraded_runtime.cursor_recomposite ||
        !degraded_runtime.subtitle_band_active || !degraded_runtime.overlay_passthrough ||
        !degraded_runtime.protection_placeholder_only ||
        degraded_runtime.last_presented_frame_id != 5 ||
        degraded_runtime.last_presented_timestamp_ns != 166'666'665 ||
        degraded_runtime.refresh_interval_ns != 16'666'667 ||
        degraded_runtime.last_target_presentation_timestamp_ns != 166'666'665 ||
        degraded_runtime.last_predicted_render_time_ns != 2'000'000 ||
        degraded_runtime.last_presentation_mode != fluxma::PresentationMode::VSync ||
        degraded_runtime.last_content_type != fluxma::ContentType::Video ||
        degraded_runtime.summary().find("classifier=yes") == std::string::npos ||
        degraded_runtime.summary().find("cursor-passthrough=yes") ==
            std::string::npos ||
        degraded_runtime.summary().find("subtitle-band=yes") == std::string::npos ||
        degraded_runtime.summary().find("overlay-passthrough=yes") ==
            std::string::npos ||
        degraded_runtime.summary().find("protection-placeholder=yes") ==
            std::string::npos ||
        degraded_runtime.summary().find("state-transitions=") == std::string::npos ||
        degraded_runtime.summary().find("last-presented-frame=5") == std::string::npos ||
        degraded_runtime.summary().find("last-presented-ts=166666665") ==
            std::string::npos ||
        degraded_runtime.summary().find("target-present-ns=166666665") ==
            std::string::npos ||
        degraded_runtime.summary().find("predicted-render-ns=2000000") ==
            std::string::npos ||
        degraded_runtime.summary().find("present-mode=vsync") == std::string::npos ||
        degraded_runtime.summary().find("content-type=video") == std::string::npos ||
        degraded_runtime.summary().find("governor=quality-high") ==
            std::string::npos ||
        degraded_runtime.summary().find("scheduler=synthetic-2x") ==
            std::string::npos ||
        degraded_runtime.summary().find("deadline-miss=1") == std::string::npos ||
        degraded_runtime.summary().find("synthetic-dropped=1") == std::string::npos) {
        std::cerr << "kcm degraded runtime counters mismatch\n";
        return EXIT_FAILURE;
    }

    const auto version_blocked_native_bridge = degraded_bridge.native_bridge_install(
        fluxma::KwinNativeInstallContext {
            .kwin_version_supported = false,
            .backend_supported = true,
            .kwin_version = "6.3.91",
            .backend_name = "drm",
        }
    );
    if (version_blocked_native_bridge.frame_deferred_reason !=
            fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        version_blocked_native_bridge.present_deferred_reason !=
            fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        version_blocked_native_bridge.state !=
            fluxma::KwinNativeBridgeState::PlaceholderOnly ||
        !version_blocked_native_bridge.all_gates_match ||
        !version_blocked_native_bridge.all_installs_deferred ||
        !version_blocked_native_bridge.frame_version_blocked ||
        !version_blocked_native_bridge.present_version_blocked ||
        version_blocked_native_bridge.frame_backend_blocked ||
        version_blocked_native_bridge.present_backend_blocked ||
        version_blocked_native_bridge.summary().find("present-version-blocked=yes") ==
            std::string::npos ||
        version_blocked_native_bridge.install_summary.find(
            "kwin version gate blocked install for 6.3.91"
        ) == std::string::npos) {
        std::cerr << "kcm native bridge version gate snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    const auto degraded_overview = degraded_bridge.overview(
        200'000'000,
        fluxma::KwinNativeInstallContext {
            .kwin_version_supported = false,
            .backend_supported = true,
            .kwin_version = "6.3.91",
            .backend_name = "drm",
        }
    );
    if (degraded_overview.atomic ||
        !degraded_overview.assembled_from_split_reads ||
        degraded_overview.runtime_observed_at_ns != 200'000'000 ||
        degraded_overview.provenance_summary != "settings+runtime+native-install" ||
        degraded_overview.settings_provenance != "settings()" ||
        degraded_overview.runtime_provenance != "runtime(now-ns)" ||
        degraded_overview.native_install_provenance !=
            "native_bridge_install(install-context)" ||
        !degraded_overview.settings.enabled ||
        degraded_overview.runtime.state != fluxma::OutputState::Active2x ||
        degraded_overview.runtime.deadline_miss_count != 1 ||
        degraded_overview.runtime.dropped_synthetic_count != 1 ||
        degraded_overview.native_install.frame_deferred_reason !=
            fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        degraded_overview.native_install.present_deferred_reason !=
            fluxma::KwinNativeDeferredReason::KwinVersionGate ||
        !degraded_overview.native_install.all_gates_match ||
        !degraded_overview.native_install.all_installs_deferred ||
        degraded_overview.summary().find("atomic=no") == std::string::npos ||
        degraded_overview.summary().find("split-reads=yes") == std::string::npos ||
        degraded_overview.summary().find("runtime-observed-at-ns=200000000") ==
            std::string::npos ||
        degraded_overview.summary().find("provenance=settings+runtime+native-install") ==
            std::string::npos ||
        degraded_overview.summary().find("settings-source=settings()") ==
            std::string::npos ||
        degraded_overview.summary().find("runtime-source=runtime(now-ns)") ==
            std::string::npos ||
        degraded_overview.summary().find(
            "native-install-source=native_bridge_install(install-context)"
        ) == std::string::npos ||
        degraded_overview.summary().find("deadline-miss=1") == std::string::npos ||
        degraded_overview.summary().find("synthetic-dropped=1") == std::string::npos ||
        degraded_overview.summary().find("frame-deferred=kwin-version-gate") ==
            std::string::npos) {
        std::cerr << "kcm degraded overview snapshot mismatch\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
