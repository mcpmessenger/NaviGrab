#pragma once

#include <string>
#include <memory>
#include <vector>
#include <chrono>
#include <functional>
#include <optional>
#include "mcp/mcp_protocol.h"

namespace chromium_playwright {

// Forward declarations
class Page;
class BrowserContext;

// Type aliases
using PageId = int;
using ContextId = int;

// Navigation options
struct NavigationOptions {
    std::chrono::milliseconds timeout{30000}; // 30 seconds default
    std::string wait_until = "load"; // 'load', 'domcontentloaded', 'networkidle'
    bool ignore_https_errors = false;
    std::optional<std::string> user_agent;
    std::optional<std::map<std::string, std::string>> extra_headers;
};

// Timeout options
struct TimeoutOptions {
    std::chrono::milliseconds timeout{30000};
};

// Viewport configuration
struct ViewportConfig {
    int width = 1280;
    int height = 720;
    double device_scale_factor = 1.0;
    bool is_mobile = false;
    bool has_touch = false;
    bool is_landscape = true;
};

// Page lifecycle events
enum class PageEvent {
    LOAD,
    DOM_CONTENT_LOADED,
    NETWORK_IDLE,
    CLOSE,
    CRASH,
    CONSOLE_MESSAGE,
    DIALOG
};

// Event handler type
using PageEventHandler = std::function<void(PageEvent event, const std::string& data)>;

// Browser Context Interface
class BrowserContext {
public:
    virtual ~BrowserContext() = default;

    // Page management
    virtual PageId NewPage() = 0;
    virtual std::unique_ptr<Page> GetPage(PageId id) = 0;
    virtual bool ClosePage(PageId id) = 0;
    virtual std::vector<PageId> GetPageIds() const = 0;
    virtual void Close() = 0;

    // Context configuration
    virtual void SetViewport(const ViewportConfig& config) = 0;
    virtual ViewportConfig GetViewport() const = 0;
    virtual void SetUserAgent(const std::string& user_agent) = 0;
    virtual std::string GetUserAgent() const = 0;

    // Cookie management
    virtual bool SetCookie(const std::string& name, const std::string& value, 
                          const std::string& domain = "", const std::string& path = "/") = 0;
    virtual std::string GetCookie(const std::string& name, const std::string& domain = "") const = 0;
    virtual bool DeleteCookie(const std::string& name, const std::string& domain = "") = 0;
    virtual void ClearCookies() = 0;

    // Storage management
    virtual void ClearStorage() = 0;
    virtual void ClearLocalStorage() = 0;
    virtual void ClearSessionStorage() = 0;

    // Permissions
    virtual void GrantPermissions(const std::vector<std::string>& permissions) = 0;
    virtual void RevokePermissions(const std::vector<std::string>& permissions) = 0;

    // Context ID
    virtual ContextId GetId() const = 0;
};

// Page Interface
class Page {
public:
    virtual ~Page() = default;

    // Navigation
    virtual bool Goto(const std::string& url, const NavigationOptions& options = {}) = 0;
    virtual bool GoBack(const NavigationOptions& options = {}) = 0;
    virtual bool GoForward(const NavigationOptions& options = {}) = 0;
    virtual bool Reload(const NavigationOptions& options = {}) = 0;
    virtual void Close() = 0;

    // Page information
    virtual std::string Url() const = 0;
    virtual std::string Title() const = 0;
    virtual std::string Content() const = 0;

    // Viewport and emulation
    virtual void SetViewport(const ViewportConfig& config) = 0;
    virtual ViewportConfig GetViewport() const = 0;
    virtual void EmulateDevice(const std::string& device_name) = 0;

    // Event handling
    virtual void OnPageEvent(PageEvent event, PageEventHandler handler) = 0;
    virtual void RemovePageEventHandler(PageEvent event) = 0;

    // Navigation waiting
    virtual bool WaitForNavigation(const std::string& event_name, 
                                 const TimeoutOptions& options = {}) = 0;
    virtual bool WaitForUrl(const std::string& url_pattern, 
                           const TimeoutOptions& options = {}) = 0;
    virtual bool WaitForLoadState(const std::string& state, 
                                 const TimeoutOptions& options = {}) = 0;

    // JavaScript execution
    virtual std::string Evaluate(const std::string& javascript_expression) = 0;
    virtual std::string EvaluateAsync(const std::string& javascript_expression, 
                                    const TimeoutOptions& options = {}) = 0;

    // Network interception
    virtual void SetRequestInterceptor(std::function<bool(const std::string& url)> interceptor) = 0;
    virtual void SetResponseInterceptor(std::function<void(const std::string& url, int status_code)> interceptor) = 0;

    // Dialog handling
    virtual void OnDialog(std::function<void(const std::string& type, const std::string& message)> handler) = 0;
    virtual void AcceptDialog() = 0;
    virtual void DismissDialog() = 0;

    // Console handling
    virtual void OnConsoleMessage(std::function<void(const std::string& level, const std::string& message)> handler) = 0;

    // Page ID
    virtual PageId GetId() const = 0;
    virtual ContextId GetContextId() const = 0;
};

// Browser Control Interface
class BrowserControl {
public:
    virtual ~BrowserControl() = default;

    // Singleton access
    static BrowserControl& GetInstance();

    // Context management
    virtual ContextId NewContext() = 0;
    virtual std::unique_ptr<BrowserContext> GetContext(ContextId id) = 0;
    virtual bool CloseContext(ContextId id) = 0;
    virtual std::vector<ContextId> GetContextIds() const = 0;

    // Global configuration
    virtual void SetDefaultTimeout(std::chrono::milliseconds timeout) = 0;
    virtual std::chrono::milliseconds GetDefaultTimeout() const = 0;
    virtual void SetDefaultUserAgent(const std::string& user_agent) = 0;
    virtual std::string GetDefaultUserAgent() const = 0;

    // Browser information
    virtual std::string GetVersion() const = 0;
    virtual std::string GetUserAgent() const = 0;

    // Cleanup
    virtual void Shutdown() = 0;
};

// Factory functions
std::unique_ptr<BrowserControl> CreateBrowserControl();
std::unique_ptr<BrowserContext> CreateBrowserContext(ContextId id);
std::unique_ptr<Page> CreatePage(PageId id, ContextId context_id);

} // namespace chromium_playwright
