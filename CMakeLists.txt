cmake_minimum_required(VERSION 3.5)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

project(NaviGrab
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

# Find required packages
find_package(Threads REQUIRED)

# Mock Chromium implementation (for testing without full Chromium)
add_compile_definitions(USE_MOCK_CHROMIUM)

# Create a simple library with just the headers
add_library(navigrab_core INTERFACE)

# Set target properties
set_target_properties(navigrab_core PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# Include directories for the library
target_include_directories(navigrab_core
    INTERFACE
        ${CMAKE_SOURCE_DIR}/include
)

# Compile definitions
target_compile_definitions(navigrab_core
    INTERFACE
        NAVIGRAB_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        NAVIGRAB_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        NAVIGRAB_VERSION_PATCH=${PROJECT_VERSION_PATCH}
        USE_MOCK_CHROMIUM
)

# Link libraries
target_link_libraries(navigrab_core
    INTERFACE
        Threads::Threads
)

# Create examples with proper source files
add_executable(simple_example examples/simple_example.cpp)
target_link_libraries(simple_example navigrab_core)

add_executable(screenshot_test_demo examples/screenshot_test_demo.cpp)
target_link_libraries(screenshot_test_demo navigrab_core)

add_executable(comprehensive_demo examples/comprehensive_demo.cpp)
target_link_libraries(comprehensive_demo navigrab_core)

add_executable(world_interface_demo examples/world_interface_demo.cpp)
target_link_libraries(world_interface_demo navigrab_core)

add_executable(real_data_demo examples/real_data_demo.cpp)
target_link_libraries(real_data_demo navigrab_core)

add_executable(simple_real_demo examples/simple_real_demo.cpp)
target_link_libraries(simple_real_demo navigrab_core)

add_executable(tooltip_demo examples/tooltip_demo.cpp)
target_link_libraries(tooltip_demo navigrab_core)

# Real Screenshot Test
add_executable(real_screenshot_test 
    examples/real_screenshot_test.cpp
    src/simple_screenshot.cpp
)
target_link_libraries(real_screenshot_test 
    navigrab_core
    gdiplus
)

# Screenshot Popup Test
add_executable(screenshot_popup_test 
    examples/screenshot_popup_test.cpp
    src/simple_screenshot_base64.cpp
)
target_link_libraries(screenshot_popup_test 
    gdiplus
)

# Proactive Scraper Test
add_executable(proactive_scraper_test 
    examples/proactive_scraper_test.cpp
    src/proactive_scraper_simple.cpp
)
target_link_libraries(proactive_scraper_test 
    gdiplus
)
target_include_directories(proactive_scraper_test PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Real Browser Scraper Test
add_executable(real_browser_scraper_test
    examples/real_browser_scraper_test.cpp
    src/real_browser_scraper.cpp
)
target_link_libraries(real_browser_scraper_test
    gdiplus
    shell32
)
target_include_directories(real_browser_scraper_test PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Tooltip System Test
add_executable(tooltip_system_test
    examples/tooltip_system_test.cpp
    src/tooltip_system/tooltip_storage.cpp
    src/tooltip_system/tooltip_screenshot_capture.cpp
    src/tooltip_system/tooltip_background_scraper.cpp
    src/tooltip_system/tooltip_system.cpp
)
target_link_libraries(tooltip_system_test
    gdiplus
)
target_include_directories(tooltip_system_test PRIVATE ${CMAKE_SOURCE_DIR}/include)

# OpenAI Screenshot Demo
add_executable(openai_screenshot_demo
    examples/openai_screenshot_demo.cpp
)
target_link_libraries(openai_screenshot_demo
    gdiplus
)

# Real OpenAI Screenshot Demo
add_executable(real_openai_screenshot_demo
    examples/real_openai_screenshot_demo.cpp
)
target_link_libraries(real_openai_screenshot_demo
    gdiplus
    winhttp
)

# NaviGrab Web Server Test
add_executable(navigrab_server_test
    examples/navigrab_server_test.cpp
    src/server/navigrab_web_server.cpp
    src/tooltip_system/tooltip_storage.cpp
    src/tooltip_system/tooltip_screenshot_capture.cpp
    src/tooltip_system/tooltip_background_scraper.cpp
    src/tooltip_system/tooltip_system.cpp
)
target_link_libraries(navigrab_server_test
    gdiplus
)
target_include_directories(navigrab_server_test PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Simple Web Server (for testing C++ integration)
add_executable(simple_web_server
    src/simple_web_server.cpp
)
target_include_directories(simple_web_server PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Windows specific libraries
if(WIN32)
    target_link_libraries(navigrab_server_test ws2_32)
    target_link_libraries(simple_web_server ws2_32)
endif()

# Web Server
add_executable(web_server 
    src/server/main.cpp
    src/server/web_server.cpp
    src/network/http_client_simple.cpp
    src/real_data/real_screenshot_capture.cpp
    src/dom/blink_dom_agent_simple.cpp
    src/image_processing/image_encoder_simple.cpp
)
target_link_libraries(web_server 
    navigrab_core
    Threads::Threads
)

# NaviGrab OpenAI Server
add_executable(navigrab_openai_server
    src/server/navigrab_openai_server.cpp
    src/openai/openai_client.cpp
    src/openai/screenshot_analyzer.cpp
    src/network/http_client_simple.cpp
    src/real_data/real_screenshot_capture.cpp
)
target_link_libraries(navigrab_openai_server
    navigrab_core
    Threads::Threads
    ws2_32
)

# Print configuration summary
message(STATUS "")
message(STATUS "NaviGrab Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Use Mock Chromium: ON")
message(STATUS "  Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
