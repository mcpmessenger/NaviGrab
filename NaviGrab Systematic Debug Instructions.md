# NaviGrab Systematic Debug Instructions

This document provides systematic debugging instructions for the NaviGrab project, based on the information gathered from the project's README, DEPLOYMENT guide, and IMPLEMENTATION PLAN.

## 1. Project Overview

NaviGrab is a modern C++ implementation aiming to replicate Playwright functionality with Chromium integration. It's designed for browsing and scraping, following MCP communication standards and modern C++ best practices.

## 2. Current Known Issues (from README.md)

The project is currently marked as "STUCK" with the following primary issues:

### 2.1. Build System Issues
*   **Problem**: CMake compilation errors.
*   **Error Example**: `Cannot open include file: 'chromium_playwright/image_processing/image_encoder.h'`
*   **Cause**: Header file path resolution problems.
*   **Impact**: Prevents compilation with real source files.

### 2.2. Compiler Environment Issues
*   **Problem**: Missing C++ compiler in PATH.
*   **Error Example**: `g++` and `cl` commands not recognized.
*   **Cause**: Visual Studio Build Tools not properly configured in system PATH.
*   **Impact**: Cannot compile C++ code directly.

### 2.3. Mock vs. Real Implementation
*   **Problem**: All code is currently mock/simulation.
*   **Issue**: No real Chromium integration.
*   **Cause**: Complexity of Chromium's build system.
*   **Impact**: Cannot perform real web automation.

### 2.4. Other Known Issues
*   Limited error handling in current code.
*   No SSL/TLS support for HTTPS.

## 3. General Debugging Approach

When encountering issues, follow these steps systematically:

1.  **Check Prerequisites**: Ensure all prerequisites from `DEPLOYMENT.md` are met.
2.  **Verify Build Environment**: Confirm CMake and compiler (Visual Studio Build Tools) are correctly installed and configured in PATH.
3.  **Start Simple**: Begin with the provided `CMakeLists_simple.txt` and basic examples.
4.  **Isolate the Problem**: Determine if the issue is related to build, runtime, or specific module integration.
5.  **Consult Documentation**: Refer to `README.md`, `DEPLOYMENT.md`, and `IMPLEMENTATION_PLAN.md` for context and specific instructions.
6.  **Review Logs**: For runtime issues, check application logs (if implemented) or console output.
7.  **Step-by-Step Debugging**: Use a debugger to step through the code and inspect variables.

## 4. Debugging Specific Issues

### 4.1. Build Issues

**Problem**: `Cannot open include file: 'chromium_playwright/image_processing/image_encoder.h'` or similar header resolution errors.
*   **Action**: Verify `CMakeLists.txt` for correct include paths. The `PROJECT_STRUCTURE.md` indicates headers are in `include/chromium_playwright/`. Ensure your build system is correctly pointing to this directory.
*   **Action**: As a workaround, use `CMakeLists_simple.txt` as described in `README.md` to get a basic build working:
    ```shell
    # Navigate to project
    cd "C:\Users\senti\OneDrive\Desktop\New folder" # Adjust path as necessary

    # Use simple CMake config
    copy CMakeLists_simple.txt build\CMakeLists.txt
    cd build
    "C:\Program Files\CMake\bin\cmake.exe" --build . --config Release # Adjust CMake path
    ```

**Problem**: `g++` or `cl` commands not recognized (Compiler not in PATH).
*   **Action**: Ensure Visual Studio Build Tools are installed. If on Windows, confirm that the necessary environment variables are set to include the compiler's path. This usually involves running the appropriate `vcvarsall.bat` script or using the Developer Command Prompt for VS.
*   **Action**: For Docker deployments, review the `Dockerfile` in `DEPLOYMENT.md` to ensure the Visual Studio Build Tools and CMake installation steps are successful.

**Problem**: General CMake compilation errors.
*   **Action**: Check CMake version (Prerequisite: CMake 3.5+). The `IMPLEMENTATION_PLAN.md` suggests CMake 3.20+ for modern features.
*   **Action**: Inspect CMake output for specific error messages. These often provide clues about missing libraries, incorrect syntax, or configuration issues.
*   **Action**: Ensure all external dependencies (e.g., googletest, benchmark, nlohmann_json) are correctly managed, possibly using FetchContent as per `IMPLEMENTATION_PLAN.md`.

### 4.2. Runtime Issues (with Mock Implementations)

**Problem**: Examples not performing real web automation.
*   **Cause**: As stated in `README.md`, all current implementations are mocks. The project is not yet integrated with a real Chromium instance.
*   **Action**: Understand that current examples are for demonstration purposes only and will not interact with actual web pages. Focus on verifying the logic of the mock implementations.

**Problem**: Unexpected behavior in mock implementations.
*   **Action**: Review the `src/` directory, specifically the mock implementations mentioned in `PROJECT_STRUCTURE.md` (e.g., `browser_control_impl.cpp`, `dom_agent.cpp`).
*   **Action**: Check the `tests/unit/` directory for existing unit tests that might shed light on expected behavior of mock components.

### 4.3. Deployment Issues

**Problem**: Docker or Kubernetes deployment failures.
*   **Action**: Refer to the `DEPLOYMENT.md` guide for detailed instructions on Docker and Kubernetes setup.
*   **Action**: For Docker, ensure the `Dockerfile` builds successfully and that the container starts without errors. Use `docker logs navigrab-container` to check logs.
*   **Action**: For Kubernetes, verify `navigrab-deployment.yaml` syntax and check pod/service status using `kubectl get pods` and `kubectl get services`.
*   **Action**: Confirm environment variables and configuration file settings (as detailed in `DEPLOYMENT.md`) are correctly applied in the deployment environment.

## 5. Next Steps for Development (from README.md & IMPLEMENTATION_PLAN.md)

These are the planned steps to move the project forward, which also highlight areas where current limitations might cause 


issues:

1.  **Fix CMake build system**: Resolve header path issues and integrate with Chromium's build system (GN).
2.  **Set up compiler environment**: Ensure Visual Studio Build Tools are correctly configured in PATH.
3.  **Integrate real Chromium**: Replace mock implementations with real Chromium code and proper Mojo IPC.
4.  **Add SSL/TLS support**: Implement for HTTPS requests.
5.  **Optimize image processing**: Add compression and other optimizations.
6.  **Implement core interfaces**: Following the specifications in `IMPLEMENTATION_PLAN.md`.
7.  **Add comprehensive testing**: Expand unit and integration test coverage.
8.  **Document APIs** and usage patterns.

## 6. Project Structure for Code Navigation

Understanding the project structure is crucial for debugging. Key directories include:

*   `include/chromium_playwright/`: Public header files for various modules (browser control, DOM interaction, screenshot capture, etc.) and MCP communication interfaces.
*   `src/`: Source implementation files, currently containing mock implementations.
*   `examples/`: Example programs for basic usage, advanced scraping, and MCP integration.
*   `web_interface/`: Contains the Navi Grab HTML interface.
*   `tests/`: Unit and integration tests.
*   `CMakeLists.txt`: Main build configuration.

## 7. Key Design Principles to Consider

When debugging or extending the project, keep in mind the following design principles outlined in `PROJECT_STRUCTURE.md`:

*   **Modern C++ Standards**: Use C++20 features, RAII, smart pointers, move semantics, and coroutines.
*   **MCP Communication Standards**: Message-based, asynchronous, type-safe communication with error handling and JSON serialization.
*   **Chromium Integration**: Utilizes Mojo IPC, Chromium's base library, and Blink rendering engine.
*   **Testing and Quality**: Relies on Google Test, Google Benchmark, Clang-tidy, and Clang-format.
*   **Build System**: CMake 3.20+ with FetchContent for dependency management.

## 8. Support

For further deployment issues or unresolved problems, refer to the `DEPLOYMENT.md` for support steps:

1.  Check the troubleshooting guide (this document).
2.  Review logs and metrics.
3.  Open an issue on GitHub.
4.  Contact the development team.

This document will be updated as the project progresses and more real-world debugging scenarios emerge.

