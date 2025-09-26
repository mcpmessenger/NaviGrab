#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <chrono>
#include <functional>
#include <optional>
#include "browser_control.h"

namespace chromium_playwright {

// Forward declarations
class ElementHandle;
class Locator;

// Rectangle structure for bounding boxes
struct Rect {
    double x = 0.0;
    double y = 0.0;
    double width = 0.0;
    double height = 0.0;
    
    bool IsEmpty() const { return width <= 0.0 || height <= 0.0; }
    bool Contains(double px, double py) const {
        return px >= x && px <= x + width && py >= y && py <= y + height;
    }
};

// Click options
struct ClickOptions {
    bool force = false; // Bypass actionability checks
    std::chrono::milliseconds delay{0}; // Delay between mouse down and up
    int click_count = 1; // Number of clicks
    std::string button = "left"; // "left", "right", "middle"
    std::vector<std::string> modifiers; // "Shift", "Control", "Alt", "Meta"
    double timeout = 30000; // Timeout in milliseconds
};

// Type options
struct TypeOptions {
    std::chrono::milliseconds delay{10}; // Delay between key presses
    bool clear = false; // Clear existing text before typing
    double timeout = 30000; // Timeout in milliseconds
};

// Fill options
struct FillOptions {
    bool force = false; // Bypass actionability checks
    double timeout = 30000; // Timeout in milliseconds
};

// Hover options
struct HoverOptions {
    bool force = false; // Bypass actionability checks
    std::vector<std::string> modifiers; // "Shift", "Control", "Alt", "Meta"
    double timeout = 30000; // Timeout in milliseconds
};

// Select options
struct SelectOptions {
    bool force = false; // Bypass actionability checks
    double timeout = 30000; // Timeout in milliseconds
};

// Drag options
struct DragOptions {
    bool force = false; // Bypass actionability checks
    double timeout = 30000; // Timeout in milliseconds
};

// Locator strategies
enum class LocatorStrategy {
    CSS_SELECTOR,
    XPATH,
    TEXT,
    ROLE,
    LABEL,
    PLACEHOLDER,
    ALT_TEXT,
    TITLE,
    TEST_ID,
    NTH
};

// Locator options
struct LocatorOptions {
    LocatorStrategy strategy = LocatorStrategy::CSS_SELECTOR;
    std::string selector;
    std::string name; // For role-based locators
    bool exact = false; // For text-based locators
    bool ignore_case = false; // For text-based locators
    int nth = 0; // For nth-based locators
};

// Element Handle Interface
class ElementHandle {
public:
    virtual ~ElementHandle() = default;

    // Basic properties
    virtual std::string TagName() const = 0;
    virtual std::string TextContent() const = 0;
    virtual std::string InnerHtml() const = 0;
    virtual std::string OuterHtml() const = 0;
    virtual std::string GetAttribute(const std::string& name) const = 0;
    virtual std::map<std::string, std::string> GetAllAttributes() const = 0;
    virtual bool HasAttribute(const std::string& name) const = 0;

    // State checks
    virtual bool IsVisible() const = 0;
    virtual bool IsEnabled() const = 0;
    virtual bool IsChecked() const = 0;
    virtual bool IsSelected() const = 0;
    virtual bool IsFocused() const = 0;

    // Geometry
    virtual Rect BoundingBox() const = 0;
    virtual Rect ContentBox() const = 0;
    virtual std::vector<Rect> AllBoundingBoxes() const = 0;

    // Actions
    virtual bool Click(const ClickOptions& options = {}) = 0;
    virtual bool DoubleClick(const ClickOptions& options = {}) = 0;
    virtual bool RightClick(const ClickOptions& options = {}) = 0;
    virtual bool Hover(const HoverOptions& options = {}) = 0;
    virtual bool Type(const std::string& text, const TypeOptions& options = {}) = 0;
    virtual bool Fill(const std::string& text, const FillOptions& options = {}) = 0;
    virtual bool Clear(const FillOptions& options = {}) = 0;
    virtual bool Check(const ClickOptions& options = {}) = 0;
    virtual bool Uncheck(const ClickOptions& options = {}) = 0;
    virtual bool SelectOption(const std::vector<std::string>& values, const SelectOptions& options = {}) = 0;
    virtual bool DragTo(ElementHandle& target, const DragOptions& options = {}) = 0;

    // Focus and blur
    virtual bool Focus() = 0;
    virtual bool Blur() = 0;

    // Screenshot
    virtual std::vector<uint8_t> Screenshot(const std::string& path = "", 
                                          const std::map<std::string, std::string>& options = {}) = 0;

    // JavaScript evaluation
    virtual std::string Evaluate(const std::string& javascript_expression) = 0;

    // Wait for conditions
    virtual bool WaitForVisible(const std::chrono::milliseconds& timeout = std::chrono::milliseconds(30000)) = 0;
    virtual bool WaitForHidden(const std::chrono::milliseconds& timeout = std::chrono::milliseconds(30000)) = 0;
    virtual bool WaitForEnabled(const std::chrono::milliseconds& timeout = std::chrono::milliseconds(30000)) = 0;
    virtual bool WaitForDisabled(const std::chrono::milliseconds& timeout = std::chrono::milliseconds(30000)) = 0;

    // Element ID (internal)
    virtual std::string GetElementId() const = 0;
};

// Locator Interface
class Locator {
public:
    virtual ~Locator() = default;

    // Element finding
    virtual std::unique_ptr<ElementHandle> Element() = 0;
    virtual std::vector<std::unique_ptr<ElementHandle>> AllElements() = 0;
    virtual std::unique_ptr<ElementHandle> First() = 0;
    virtual std::unique_ptr<ElementHandle> Last() = 0;
    virtual std::unique_ptr<ElementHandle> Nth(int index) = 0;

    // Count and existence
    virtual int Count() = 0;
    virtual bool Exists() = 0;

    // Actions (performed on all matching elements)
    virtual bool Click(const ClickOptions& options = {}) = 0;
    virtual bool DoubleClick(const ClickOptions& options = {}) = 0;
    virtual bool RightClick(const ClickOptions& options = {}) = 0;
    virtual bool Hover(const HoverOptions& options = {}) = 0;
    virtual bool Type(const std::string& text, const TypeOptions& options = {}) = 0;
    virtual bool Fill(const std::string& text, const FillOptions& options = {}) = 0;
    virtual bool Clear(const FillOptions& options = {}) = 0;
    virtual bool Check(const ClickOptions& options = {}) = 0;
    virtual bool Uncheck(const ClickOptions& options = {}) = 0;
    virtual bool SelectOption(const std::vector<std::string>& values, const SelectOptions& options = {}) = 0;
    virtual bool DragTo(Locator& target, const DragOptions& options = {}) = 0;

    // State checks (for all matching elements)
    virtual bool IsVisible() = 0;
    virtual bool IsEnabled() = 0;
    virtual bool IsChecked() = 0;
    virtual bool IsSelected() = 0;
    virtual bool IsFocused() = 0;

    // Text and content
    virtual std::string TextContent() = 0;
    virtual std::string InnerHtml() = 0;
    virtual std::string OuterHtml() = 0;
    virtual std::string GetAttribute(const std::string& name) = 0;
    virtual std::map<std::string, std::string> GetAllAttributes() = 0;

    // Geometry
    virtual Rect BoundingBox() = 0;
    virtual std::vector<Rect> AllBoundingBoxes() = 0;

    // Screenshot
    virtual std::vector<uint8_t> Screenshot(const std::string& path = "", 
                                          const std::map<std::string, std::string>& options = {}) = 0;

    // Wait for conditions
    virtual bool WaitForVisible(const std::chrono::milliseconds& timeout = std::chrono::milliseconds(30000)) = 0;
    virtual bool WaitForHidden(const std::chrono::milliseconds& timeout = std::chrono::milliseconds(30000)) = 0;
    virtual bool WaitForEnabled(const std::chrono::milliseconds& timeout = std::chrono::milliseconds(30000)) = 0;
    virtual bool WaitForDisabled(const std::chrono::milliseconds& timeout = std::chrono::milliseconds(30000)) = 0;

    // Locator chaining
    virtual std::unique_ptr<Locator> And(Locator& other) = 0;
    virtual std::unique_ptr<Locator> Or(Locator& other) = 0;
    virtual std::unique_ptr<Locator> Not() = 0;

    // Locator information
    virtual LocatorStrategy GetStrategy() const = 0;
    virtual std::string GetSelector() const = 0;
    virtual LocatorOptions GetOptions() const = 0;
};

// Page extension for DOM interaction
class Page {
public:
    // Locator creation methods
    virtual std::unique_ptr<Locator> GetByCss(const std::string& selector) = 0;
    virtual std::unique_ptr<Locator> GetByXPath(const std::string& expression) = 0;
    virtual std::unique_ptr<Locator> GetByRole(const std::string& role, const std::string& name = "") = 0;
    virtual std::unique_ptr<Locator> GetByText(const std::string& text, const LocatorOptions& options = {}) = 0;
    virtual std::unique_ptr<Locator> GetByLabel(const std::string& label) = 0;
    virtual std::unique_ptr<Locator> GetByPlaceholder(const std::string& placeholder) = 0;
    virtual std::unique_ptr<Locator> GetByAltText(const std::string& alt_text) = 0;
    virtual std::unique_ptr<Locator> GetByTitle(const std::string& title) = 0;
    virtual std::unique_ptr<Locator> GetByTestId(const std::string& test_id) = 0;
    virtual std::unique_ptr<Locator> GetByNth(const std::string& selector, int nth) = 0;

    // Generic locator creation
    virtual std::unique_ptr<Locator> GetByLocator(const LocatorOptions& options) = 0;

    // Global actions
    virtual bool Click(const std::string& selector, const ClickOptions& options = {}) = 0;
    virtual bool Type(const std::string& selector, const std::string& text, const TypeOptions& options = {}) = 0;
    virtual bool Fill(const std::string& selector, const std::string& text, const FillOptions& options = {}) = 0;
    virtual bool Hover(const std::string& selector, const HoverOptions& options = {}) = 0;
    virtual bool Check(const std::string& selector, const ClickOptions& options = {}) = 0;
    virtual bool Uncheck(const std::string& selector, const ClickOptions& options = {}) = 0;
    virtual bool SelectOption(const std::string& selector, const std::vector<std::string>& values, const SelectOptions& options = {}) = 0;

    // Keyboard actions
    virtual bool PressKey(const std::string& key) = 0;
    virtual bool PressKeySequence(const std::vector<std::string>& keys) = 0;
    virtual bool PressKeyWithModifiers(const std::string& key, const std::vector<std::string>& modifiers) = 0;

    // Mouse actions
    virtual bool MouseMove(double x, double y) = 0;
    virtual bool MouseClick(double x, double y, const ClickOptions& options = {}) = 0;
    virtual bool MouseDown(const ClickOptions& options = {}) = 0;
    virtual bool MouseUp(const ClickOptions& options = {}) = 0;
    virtual bool MouseWheel(double delta_x, double delta_y) = 0;

    // Touch actions
    virtual bool TouchStart(const std::vector<std::map<std::string, double>>& touches) = 0;
    virtual bool TouchMove(const std::vector<std::map<std::string, double>>& touches) = 0;
    virtual bool TouchEnd() = 0;

    // Wait for elements
    virtual std::unique_ptr<ElementHandle> WaitForElement(const std::string& selector, 
                                                         const std::chrono::milliseconds& timeout = std::chrono::milliseconds(30000)) = 0;
    virtual bool WaitForElementVisible(const std::string& selector, 
                                     const std::chrono::milliseconds& timeout = std::chrono::milliseconds(30000)) = 0;
    virtual bool WaitForElementHidden(const std::string& selector, 
                                     const std::chrono::milliseconds& timeout = std::chrono::milliseconds(30000)) = 0;
};

// Factory functions
std::unique_ptr<Locator> CreateLocator(const LocatorOptions& options, Page* page);
std::unique_ptr<ElementHandle> CreateElementHandle(const std::string& element_id, Page* page);

} // namespace chromium_playwright
