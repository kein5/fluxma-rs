cmake_minimum_required(VERSION 3.24)

project(fluxma
    VERSION 0.1.0
    DESCRIPTION "Fluxma KWin native module skeleton"
    LANGUAGES CXX
)

include(CTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_program(CARGO_EXECUTABLE cargo REQUIRED)

set(FLUXMA_RUST_MANIFEST ${CMAKE_CURRENT_SOURCE_DIR}/src/rust/Cargo.toml)
set(FLUXMA_RUST_TARGET_DIR ${CMAKE_CURRENT_BINARY_DIR}/cargo-target)
set(FLUXMA_RUST_PROFILE debug)
set(FLUXMA_RUST_LIBRARY
    ${FLUXMA_RUST_TARGET_DIR}/${FLUXMA_RUST_PROFILE}/libfluxma_rs_core.a
)

add_custom_command(
    OUTPUT ${FLUXMA_RUST_LIBRARY}
    COMMAND
        ${CARGO_EXECUTABLE} build
        --manifest-path ${FLUXMA_RUST_MANIFEST}
        --target-dir ${FLUXMA_RUST_TARGET_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rust/Cargo.toml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rust/build.rs
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rust/src/lib.rs
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rust/include/fluxma_rs_core.h
    COMMENT "Building Fluxma Rust core"
    VERBATIM
)

add_custom_target(fluxma_rust_core ALL DEPENDS ${FLUXMA_RUST_LIBRARY})

add_library(fluxma_native_skeleton STATIC
    src/cpp/fluxma_plugin_root.cpp
    src/cpp/fluxma_output_controller.cpp
    src/cpp/fluxma_output_policy.cpp
    src/cpp/fluxma_bypass_pipeline.cpp
    src/cpp/fluxma_config.cpp
    src/cpp/fluxma_frame_tap.cpp
    src/cpp/fluxma_fake_synth_generator.cpp
    src/cpp/fluxma_gpu_services.cpp
    src/cpp/fluxma_hud_renderer.cpp
    src/cpp/fluxma_kwin_hook_builders.cpp
    src/cpp/fluxma_kwin_hook_candidates.cpp
    src/cpp/fluxma_kwin_hook_adapter.cpp
    src/cpp/fluxma_kwin_native_bridge.cpp
    src/cpp/fluxma_logger.cpp
    src/cpp/fluxma_present_feedback_tap.cpp
    src/cpp/fluxma_synthetic_scheduler.cpp
    src/cpp/fluxma_synthetic_present_queue.cpp
    src/cpp/fluxma_texture_pool.cpp
)
add_dependencies(fluxma_native_skeleton fluxma_rust_core)

target_include_directories(fluxma_native_skeleton
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rust/include
)

target_link_libraries(fluxma_native_skeleton
    PUBLIC
        ${FLUXMA_RUST_LIBRARY}
)

add_executable(fluxma_bridge_smoke
    tests/cpp/bridge_smoke.cpp
)
add_dependencies(fluxma_bridge_smoke fluxma_rust_core)

target_include_directories(fluxma_bridge_smoke
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rust/include
)

target_link_libraries(fluxma_bridge_smoke
    PRIVATE
        fluxma_native_skeleton
)

add_executable(fluxma_disabled_smoke
    tests/cpp/disabled_smoke.cpp
)
add_dependencies(fluxma_disabled_smoke fluxma_rust_core)

target_include_directories(fluxma_disabled_smoke
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rust/include
)

target_link_libraries(fluxma_disabled_smoke
    PRIVATE
        fluxma_native_skeleton
)

add_executable(fluxma_hook_unavailable_smoke
    tests/cpp/hook_unavailable_smoke.cpp
)
add_dependencies(fluxma_hook_unavailable_smoke fluxma_rust_core)

target_include_directories(fluxma_hook_unavailable_smoke
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rust/include
)

target_link_libraries(fluxma_hook_unavailable_smoke
    PRIVATE
        fluxma_native_skeleton
)

add_executable(fluxma_logger_smoke
    tests/cpp/logger_smoke.cpp
)
add_dependencies(fluxma_logger_smoke fluxma_rust_core)

target_include_directories(fluxma_logger_smoke
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rust/include
)

target_link_libraries(fluxma_logger_smoke
    PRIVATE
        fluxma_native_skeleton
)

add_executable(fluxma_unsupported_output_smoke
    tests/cpp/unsupported_output_smoke.cpp
)
add_dependencies(fluxma_unsupported_output_smoke fluxma_rust_core)

target_include_directories(fluxma_unsupported_output_smoke
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rust/include
)

target_link_libraries(fluxma_unsupported_output_smoke
    PRIVATE
        fluxma_native_skeleton
)

add_executable(fluxma_present_feedback_mismatch_smoke
    tests/cpp/present_feedback_mismatch_smoke.cpp
)
add_dependencies(fluxma_present_feedback_mismatch_smoke fluxma_rust_core)

target_include_directories(fluxma_present_feedback_mismatch_smoke
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rust/include
)

target_link_libraries(fluxma_present_feedback_mismatch_smoke
    PRIVATE
        fluxma_native_skeleton
)

add_executable(fluxma_non_primary_output_smoke
    tests/cpp/non_primary_output_smoke.cpp
)
add_dependencies(fluxma_non_primary_output_smoke fluxma_rust_core)

target_include_directories(fluxma_non_primary_output_smoke
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rust/include
)

target_link_libraries(fluxma_non_primary_output_smoke
    PRIVATE
        fluxma_native_skeleton
)

add_executable(fluxma_hud_opt_out_smoke
    tests/cpp/hud_opt_out_smoke.cpp
)
add_dependencies(fluxma_hud_opt_out_smoke fluxma_rust_core)

target_include_directories(fluxma_hud_opt_out_smoke
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rust/include
)

target_link_libraries(fluxma_hud_opt_out_smoke
    PRIVATE
        fluxma_native_skeleton
)

add_executable(fluxma_hook_context_smoke
    tests/cpp/hook_context_smoke.cpp
)
add_dependencies(fluxma_hook_context_smoke fluxma_rust_core)

target_include_directories(fluxma_hook_context_smoke
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rust/include
)

target_link_libraries(fluxma_hook_context_smoke
    PRIVATE
        fluxma_native_skeleton
)

add_executable(fluxma_adapter_candidate_smoke
    tests/cpp/adapter_candidate_smoke.cpp
)
add_dependencies(fluxma_adapter_candidate_smoke fluxma_rust_core)

target_include_directories(fluxma_adapter_candidate_smoke
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rust/include
)

target_link_libraries(fluxma_adapter_candidate_smoke
    PRIVATE
        fluxma_native_skeleton
)

add_executable(fluxma_native_bridge_smoke
    tests/cpp/native_bridge_smoke.cpp
)
add_dependencies(fluxma_native_bridge_smoke fluxma_rust_core)

target_include_directories(fluxma_native_bridge_smoke
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rust/include
)

target_link_libraries(fluxma_native_bridge_smoke
    PRIVATE
        fluxma_native_skeleton
)

add_executable(fluxma_plugin_root_native_bridge_smoke
    tests/cpp/plugin_root_native_bridge_smoke.cpp
)
add_dependencies(fluxma_plugin_root_native_bridge_smoke fluxma_rust_core)

target_include_directories(fluxma_plugin_root_native_bridge_smoke
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rust/include
)

target_link_libraries(fluxma_plugin_root_native_bridge_smoke
    PRIVATE
        fluxma_native_skeleton
)

add_executable(fluxma_hook_input_builder_smoke
    tests/cpp/hook_input_builder_smoke.cpp
)
add_dependencies(fluxma_hook_input_builder_smoke fluxma_rust_core)

target_include_directories(fluxma_hook_input_builder_smoke
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rust/include
)

target_link_libraries(fluxma_hook_input_builder_smoke
    PRIVATE
        fluxma_native_skeleton
)

add_executable(fluxma_hook_field_source_smoke
    tests/cpp/hook_field_source_smoke.cpp
)
add_dependencies(fluxma_hook_field_source_smoke fluxma_rust_core)

target_include_directories(fluxma_hook_field_source_smoke
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rust/include
)

target_link_libraries(fluxma_hook_field_source_smoke
    PRIVATE
        fluxma_native_skeleton
)

add_executable(fluxma_hook_candidate_plan_smoke
    tests/cpp/hook_candidate_plan_smoke.cpp
)
add_dependencies(fluxma_hook_candidate_plan_smoke fluxma_rust_core)

target_include_directories(fluxma_hook_candidate_plan_smoke
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rust/include
)

target_link_libraries(fluxma_hook_candidate_plan_smoke
    PRIVATE
        fluxma_native_skeleton
)

add_executable(fluxma_incomplete_bundle_path_smoke
    tests/cpp/incomplete_bundle_path_smoke.cpp
)
add_dependencies(fluxma_incomplete_bundle_path_smoke fluxma_rust_core)

target_include_directories(fluxma_incomplete_bundle_path_smoke
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rust/include
)

target_link_libraries(fluxma_incomplete_bundle_path_smoke
    PRIVATE
        fluxma_native_skeleton
)

add_executable(fluxma_invalid_present_feedback_smoke
    tests/cpp/invalid_present_feedback_smoke.cpp
)
add_dependencies(fluxma_invalid_present_feedback_smoke fluxma_rust_core)

target_include_directories(fluxma_invalid_present_feedback_smoke
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rust/include
)

target_link_libraries(fluxma_invalid_present_feedback_smoke
    PRIVATE
        fluxma_native_skeleton
)

add_executable(fluxma_protected_unsupported_precedence_smoke
    tests/cpp/protected_unsupported_precedence_smoke.cpp
)
add_dependencies(fluxma_protected_unsupported_precedence_smoke fluxma_rust_core)

target_include_directories(fluxma_protected_unsupported_precedence_smoke
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rust/include
)

target_link_libraries(fluxma_protected_unsupported_precedence_smoke
    PRIVATE
        fluxma_native_skeleton
)

add_executable(fluxma_state_core_smoke
    tests/cpp/state_core_smoke.cpp
)
add_dependencies(fluxma_state_core_smoke fluxma_rust_core)

target_include_directories(fluxma_state_core_smoke
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rust/include
)

target_link_libraries(fluxma_state_core_smoke
    PRIVATE
        fluxma_native_skeleton
)

add_executable(fluxma_abi_layout_smoke
    tests/cpp/abi_layout_smoke.cpp
)
add_dependencies(fluxma_abi_layout_smoke fluxma_rust_core)

target_include_directories(fluxma_abi_layout_smoke
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rust/include
)

add_executable(fluxma_fake_synth_plan_smoke
    tests/cpp/fake_synth_plan_smoke.cpp
)
add_dependencies(fluxma_fake_synth_plan_smoke fluxma_rust_core)

target_include_directories(fluxma_fake_synth_plan_smoke
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rust/include
)

target_link_libraries(fluxma_fake_synth_plan_smoke
    PRIVATE
        fluxma_native_skeleton
)

add_executable(fluxma_output_runtime_observation_smoke
    tests/cpp/output_runtime_observation_smoke.cpp
)
add_dependencies(fluxma_output_runtime_observation_smoke fluxma_rust_core)

target_include_directories(fluxma_output_runtime_observation_smoke
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rust/include
)

target_link_libraries(fluxma_output_runtime_observation_smoke
    PRIVATE
        fluxma_native_skeleton
)

add_executable(fluxma_texture_pool_smoke
    tests/cpp/texture_pool_smoke.cpp
)
add_dependencies(fluxma_texture_pool_smoke fluxma_rust_core)

target_include_directories(fluxma_texture_pool_smoke
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rust/include
)

target_link_libraries(fluxma_texture_pool_smoke
    PRIVATE
        fluxma_native_skeleton
)

if(BUILD_TESTING)
    add_test(NAME fluxma_bridge_smoke COMMAND fluxma_bridge_smoke)
    add_test(NAME fluxma_disabled_smoke COMMAND fluxma_disabled_smoke)
    add_test(NAME fluxma_hook_unavailable_smoke COMMAND fluxma_hook_unavailable_smoke)
    add_test(NAME fluxma_logger_smoke COMMAND fluxma_logger_smoke)
    add_test(NAME fluxma_unsupported_output_smoke COMMAND fluxma_unsupported_output_smoke)
    add_test(
        NAME fluxma_present_feedback_mismatch_smoke
        COMMAND fluxma_present_feedback_mismatch_smoke
    )
    add_test(NAME fluxma_non_primary_output_smoke COMMAND fluxma_non_primary_output_smoke)
    add_test(NAME fluxma_hud_opt_out_smoke COMMAND fluxma_hud_opt_out_smoke)
    add_test(NAME fluxma_hook_context_smoke COMMAND fluxma_hook_context_smoke)
    add_test(NAME fluxma_adapter_candidate_smoke COMMAND fluxma_adapter_candidate_smoke)
    add_test(NAME fluxma_native_bridge_smoke COMMAND fluxma_native_bridge_smoke)
    add_test(
        NAME fluxma_plugin_root_native_bridge_smoke
        COMMAND fluxma_plugin_root_native_bridge_smoke
    )
    add_test(NAME fluxma_hook_input_builder_smoke COMMAND fluxma_hook_input_builder_smoke)
    add_test(NAME fluxma_hook_field_source_smoke COMMAND fluxma_hook_field_source_smoke)
    add_test(NAME fluxma_hook_candidate_plan_smoke COMMAND fluxma_hook_candidate_plan_smoke)
    add_test(
        NAME fluxma_incomplete_bundle_path_smoke
        COMMAND fluxma_incomplete_bundle_path_smoke
    )
    add_test(
        NAME fluxma_invalid_present_feedback_smoke
        COMMAND fluxma_invalid_present_feedback_smoke
    )
    add_test(
        NAME fluxma_protected_unsupported_precedence_smoke
        COMMAND fluxma_protected_unsupported_precedence_smoke
    )
    add_test(NAME fluxma_state_core_smoke COMMAND fluxma_state_core_smoke)
    add_test(NAME fluxma_abi_layout_smoke COMMAND fluxma_abi_layout_smoke)
    add_test(NAME fluxma_fake_synth_plan_smoke COMMAND fluxma_fake_synth_plan_smoke)
    add_test(
        NAME fluxma_output_runtime_observation_smoke
        COMMAND fluxma_output_runtime_observation_smoke
    )
    add_test(NAME fluxma_texture_pool_smoke COMMAND fluxma_texture_pool_smoke)
endif()
