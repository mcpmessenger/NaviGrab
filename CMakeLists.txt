cmake_minimum_required(VERSION 3.20)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

project(chromium-playwright
    VERSION 1.0.0
    DESCRIPTION "C++ Playwright Clone for Chromium Integration"
    LANGUAGES CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler-specific options
if(MSVC)
    add_compile_options(/W4 /permissive- /Zc:__cplusplus)
    add_compile_definitions(_WIN32_WINNT=0x0A00)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0 -fsanitize=address -fsanitize=undefined)
        add_link_options(-fsanitize=address -fsanitize=undefined)
    endif()
endif()

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Find required packages
find_package(Threads REQUIRED)

# Chromium dependencies (these would be actual Chromium paths in real implementation)
# For now, we'll use placeholders and mock implementations
set(CHROMIUM_ROOT_DIR "" CACHE PATH "Path to Chromium source directory")
if(NOT CHROMIUM_ROOT_DIR)
    message(WARNING "CHROMIUM_ROOT_DIR not set. Using mock implementations.")
    add_compile_definitions(USE_MOCK_CHROMIUM)
endif()

# Third-party dependencies
include(FetchContent)

# Google Test
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

# Google Benchmark
FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.3
)
FetchContent_MakeAvailable(benchmark)

# nlohmann/json
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(nlohmann_json)

# spdlog for logging
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.11.0
)
FetchContent_MakeAvailable(spdlog)

# SQLite3
find_package(SQLite3 REQUIRED)

# Create the main library
add_library(chromium_playwright_core
    # MCP Protocol
    src/mcp/mcp_protocol_impl.cpp
    src/mcp/mcp_client_impl.cpp
    src/mcp/mcp_server_impl.cpp
    
    # Browser Control Module
    src/browser_control/browser_control_impl.cpp
    src/browser_control/browser_context_impl.cpp
    src/browser_control/page_impl.cpp
    
    # DOM Interaction Module
    src/dom_interaction/locator_impl.cpp
    src/dom_interaction/element_handle_impl.cpp
    src/dom_interaction/dom_agent.cpp
    
    # Screenshot Capture Module
    src/screenshot_capture/screenshot_capture_impl.cpp
    src/screenshot_capture/image_processor.cpp
    
    # Proactive Scraping Module
    src/proactive_scraping/scraper_impl.cpp
    src/proactive_scraping/traversal_engine.cpp
    src/proactive_scraping/change_detector.cpp
    
    # Storage Integration Module
    src/storage_integration/storage_manager_impl.cpp
    src/storage_integration/sqlite_storage.cpp
    src/storage_integration/indexeddb_storage.cpp
    
    # API Layer
    src/api_layer/api_layer_impl.cpp
    src/api_layer/mojo_interfaces.cpp
)

# Set target properties
set_target_properties(chromium_playwright_core PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# Link libraries
target_link_libraries(chromium_playwright_core
    PUBLIC
        Threads::Threads
        nlohmann_json::nlohmann_json
        spdlog::spdlog
        SQLite::SQLite3
    PRIVATE
        $<$<BOOL:${USE_MOCK_CHROMIUM}>:mock_chromium>
)

# Include directories for the library
target_include_directories(chromium_playwright_core
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

# Compile definitions
target_compile_definitions(chromium_playwright_core
    PRIVATE
        CHROMIUM_PLAYWRIGHT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        CHROMIUM_PLAYWRIGHT_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        CHROMIUM_PLAYWRIGHT_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)

# Create examples
add_executable(basic_usage examples/basic_usage.cpp)
target_link_libraries(basic_usage chromium_playwright_core)

add_executable(advanced_scraping examples/advanced_scraping.cpp)
target_link_libraries(advanced_scraping chromium_playwright_core)

add_executable(mcp_integration examples/mcp_integration.cpp)
target_link_libraries(mcp_integration chromium_playwright_core)

# Create tests
enable_testing()

# Unit tests
add_executable(unit_tests
    tests/unit/browser_control_test.cpp
    tests/unit/dom_interaction_test.cpp
    tests/unit/screenshot_capture_test.cpp
    tests/unit/proactive_scraping_test.cpp
    tests/unit/storage_integration_test.cpp
    tests/unit/api_layer_test.cpp
    tests/unit/mcp_protocol_test.cpp
)

target_link_libraries(unit_tests
    chromium_playwright_core
    gtest_main
    gmock_main
)

# Integration tests
add_executable(integration_tests
    tests/integration/end_to_end_test.cpp
    tests/integration/mcp_communication_test.cpp
)

target_link_libraries(integration_tests
    chromium_playwright_core
    gtest_main
    gmock_main
)

# Benchmark tests
add_executable(benchmark_tests
    tests/benchmark/performance_benchmark.cpp
)

target_link_libraries(benchmark_tests
    chromium_playwright_core
    benchmark::benchmark
    benchmark::benchmark_main
)

# Add tests to CTest
add_test(NAME unit_tests COMMAND unit_tests)
add_test(NAME integration_tests COMMAND integration_tests)

# Set test properties
set_tests_properties(unit_tests PROPERTIES
    TIMEOUT 300
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

set_tests_properties(integration_tests PROPERTIES
    TIMEOUT 600
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Mock Chromium implementation (for testing without full Chromium)
if(USE_MOCK_CHROMIUM)
    add_library(mock_chromium
        src/mock/mock_browser_context.cpp
        src/mock/mock_web_contents.cpp
        src/mock/mock_render_widget_host_view.cpp
        src/mock/mock_dom_agent.cpp
    )
    
    target_include_directories(mock_chromium
        PUBLIC
            ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/src/mock
    )
endif()

# Installation
install(TARGETS chromium_playwright_core
    EXPORT chromium_playwright_targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/ DESTINATION include)

# Export targets
install(EXPORT chromium_playwright_targets
    FILE chromium_playwright_targets.cmake
    NAMESPACE chromium_playwright::
    DESTINATION lib/cmake/chromium_playwright
)

# Create config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    chromium_playwright_config_version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    ${CMAKE_SOURCE_DIR}/cmake/chromium_playwright_config.cmake.in
    ${CMAKE_BINARY_DIR}/chromium_playwright_config.cmake
    INSTALL_DESTINATION lib/cmake/chromium_playwright
)

install(FILES
    ${CMAKE_BINARY_DIR}/chromium_playwright_config.cmake
    ${CMAKE_BINARY_DIR}/chromium_playwright_config_version.cmake
    DESTINATION lib/cmake/chromium_playwright
)

# Code formatting (if clang-format is available)
find_program(CLANG_FORMAT clang-format)
if(CLANG_FORMAT)
    add_custom_target(format
        COMMAND ${CLANG_FORMAT} -i -style=file ${CMAKE_SOURCE_DIR}/src/*.cpp
        COMMAND ${CLANG_FORMAT} -i -style=file ${CMAKE_SOURCE_DIR}/include/*.h
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endif()

# Static analysis (if clang-tidy is available)
find_program(CLANG_TIDY clang-tidy)
if(CLANG_TIDY)
    add_custom_target(tidy
        COMMAND ${CLANG_TIDY} ${CMAKE_SOURCE_DIR}/src/*.cpp
        COMMAND ${CLANG_TIDY} ${CMAKE_SOURCE_DIR}/include/*.h
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endif()

# Documentation (if Doxygen is available)
find_package(Doxygen)
if(DOXYGEN_FOUND)
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/docs/Doxyfile
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "Chromium Playwright Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Chromium Root: ${CHROMIUM_ROOT_DIR}")
message(STATUS "  Use Mock Chromium: ${USE_MOCK_CHROMIUM}")
message(STATUS "  Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
