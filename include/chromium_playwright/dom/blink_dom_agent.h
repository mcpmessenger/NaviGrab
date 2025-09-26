#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <chrono>

namespace chromium_playwright::dom {

// Element search types
enum class ElementSearchType {
    CSS_SELECTOR,
    XPATH,
    TEXT_CONTENT,
    ROLE,
    PLACEHOLDER,
    ALT_TEXT,
    TITLE,
    TEST_ID
};

// Rectangle structure
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

// Element handle structure
struct ElementHandle {
    std::string element_id;
    std::string tag_name;
    std::string text_content;
    Rect bounding_box;
    
    // Additional properties
    std::map<std::string, std::string> attributes;
    bool visible = true;
    bool enabled = true;
    bool checked = false;
    bool focused = false;
    bool hovered = false;
    bool clicked = false;
    uint64_t last_click_time = 0;
};

// Blink DOM Agent interface
class BlinkDOMAgent {
public:
    virtual ~BlinkDOMAgent() = default;
    
    // Element finding
    virtual std::vector<ElementHandle> FindElements(const std::string& selector, ElementSearchType type) = 0;
    
    // Element actions
    virtual bool ClickElement(const std::string& element_id) = 0;
    virtual bool TypeText(const std::string& element_id, const std::string& text) = 0;
    virtual bool HoverElement(const std::string& element_id) = 0;
    virtual bool FocusElement(const std::string& element_id) = 0;
    virtual bool BlurElement(const std::string& element_id) = 0;
    virtual bool CheckElement(const std::string& element_id) = 0;
    virtual bool UncheckElement(const std::string& element_id) = 0;
    virtual bool SelectOption(const std::string& element_id, const std::string& value) = 0;
    virtual bool DragElement(const std::string& element_id, double x, double y) = 0;
    
    // Element properties
    virtual std::string GetElementText(const std::string& element_id) = 0;
    virtual std::string GetElementHTML(const std::string& element_id) = 0;
    virtual std::string GetElementAttribute(const std::string& element_id, const std::string& attribute_name) = 0;
    virtual bool SetElementAttribute(const std::string& element_id, const std::string& attribute_name, const std::string& value) = 0;
    virtual bool RemoveElementAttribute(const std::string& element_id, const std::string& attribute_name) = 0;
    
    // Element state
    virtual bool IsElementVisible(const std::string& element_id) = 0;
    virtual bool IsElementEnabled(const std::string& element_id) = 0;
    virtual bool IsElementChecked(const std::string& element_id) = 0;
    virtual bool IsElementFocused(const std::string& element_id) = 0;
    virtual bool IsElementHovered(const std::string& element_id) = 0;
    
    // Element geometry
    virtual Rect GetElementBoundingBox(const std::string& element_id) = 0;
    virtual std::vector<Rect> GetElementAllBoundingBoxes(const std::string& element_id) = 0;
    virtual bool IsElementInViewport(const std::string& element_id) = 0;
    
    // JavaScript execution
    virtual std::string ExecuteJavaScript(const std::string& script) = 0;
    virtual std::string ExecuteJavaScriptInElement(const std::string& element_id, const std::string& script) = 0;
    
    // Page navigation
    virtual bool NavigateTo(const std::string& url) = 0;
    virtual bool GoBack() = 0;
    virtual bool GoForward() = 0;
    virtual bool Reload() = 0;
    virtual std::string GetCurrentURL() = 0;
    virtual std::string GetPageTitle() = 0;
    
    // Page content
    virtual std::string GetPageHTML() = 0;
    virtual std::string GetPageText() = 0;
    virtual std::vector<std::string> GetPageLinks() = 0;
    virtual std::vector<std::string> GetPageImages() = 0;
    
    // Event handling
    virtual void AddEventListener(const std::string& element_id, const std::string& event_type, 
                                 std::function<void()> callback) = 0;
    virtual void RemoveEventListener(const std::string& element_id, const std::string& event_type) = 0;
    virtual void TriggerEvent(const std::string& element_id, const std::string& event_type) = 0;
    
    // Wait conditions
    virtual bool WaitForElement(const std::string& selector, ElementSearchType type, int timeout_ms = 5000) = 0;
    virtual bool WaitForElementVisible(const std::string& element_id, int timeout_ms = 5000) = 0;
    virtual bool WaitForElementHidden(const std::string& element_id, int timeout_ms = 5000) = 0;
    virtual bool WaitForElementEnabled(const std::string& element_id, int timeout_ms = 5000) = 0;
    virtual bool WaitForNavigation(int timeout_ms = 5000) = 0;
    virtual bool WaitForLoadState(const std::string& state, int timeout_ms = 5000) = 0;
    
    // Screenshot
    virtual std::vector<uint8_t> CaptureElementScreenshot(const std::string& element_id) = 0;
    virtual std::vector<uint8_t> CapturePageScreenshot() = 0;
    
    // Form handling
    virtual bool FillForm(const std::map<std::string, std::string>& form_data) = 0;
    virtual bool SubmitForm(const std::string& form_id) = 0;
    virtual std::map<std::string, std::string> GetFormData(const std::string& form_id) = 0;
    
    // Cookie management
    virtual bool SetCookie(const std::string& name, const std::string& value, 
                          const std::string& domain = "", const std::string& path = "/") = 0;
    virtual std::string GetCookie(const std::string& name) = 0;
    virtual bool DeleteCookie(const std::string& name) = 0;
    virtual void ClearCookies() = 0;
    
    // Local storage
    virtual bool SetLocalStorage(const std::string& key, const std::string& value) = 0;
    virtual std::string GetLocalStorage(const std::string& key) = 0;
    virtual bool RemoveLocalStorage(const std::string& key) = 0;
    virtual void ClearLocalStorage() = 0;
    
    // Session storage
    virtual bool SetSessionStorage(const std::string& key, const std::string& value) = 0;
    virtual std::string GetSessionStorage(const std::string& key) = 0;
    virtual bool RemoveSessionStorage(const std::string& key) = 0;
    virtual void ClearSessionStorage() = 0;
};

// Factory function
std::unique_ptr<BlinkDOMAgent> CreateBlinkDOMAgent();

// Utility functions
namespace dom_utils {
    // Selector validation
    bool IsValidCSSSelector(const std::string& selector);
    bool IsValidXPath(const std::string& xpath);
    
    // Element filtering
    std::vector<ElementHandle> FilterVisibleElements(const std::vector<ElementHandle>& elements);
    std::vector<ElementHandle> FilterEnabledElements(const std::vector<ElementHandle>& elements);
    std::vector<ElementHandle> FilterByTagName(const std::vector<ElementHandle>& elements, const std::string& tag_name);
    
    // Element sorting
    std::vector<ElementHandle> SortElementsByPosition(const std::vector<ElementHandle>& elements);
    std::vector<ElementHandle> SortElementsBySize(const std::vector<ElementHandle>& elements);
    
    // Element comparison
    bool ElementsEqual(const ElementHandle& a, const ElementHandle& b);
    bool ElementContains(const ElementHandle& parent, const ElementHandle& child);
    
    // Geometry helpers
    bool ElementsOverlap(const ElementHandle& a, const ElementHandle& b);
    double CalculateDistance(const ElementHandle& a, const ElementHandle& b);
    Rect GetUnionRect(const std::vector<ElementHandle>& elements);
    
    // Text helpers
    std::string NormalizeText(const std::string& text);
    bool TextContains(const std::string& text, const std::string& substring, bool case_sensitive = true);
    std::vector<std::string> ExtractWords(const std::string& text);
    
    // Attribute helpers
    std::map<std::string, std::string> ParseAttributes(const std::string& html);
    std::string BuildAttributeString(const std::map<std::string, std::string>& attributes);
    bool HasAttribute(const ElementHandle& element, const std::string& attribute_name);
    std::string GetAttributeValue(const ElementHandle& element, const std::string& attribute_name);
}

} // namespace chromium_playwright::dom