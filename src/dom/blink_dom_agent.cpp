#include "blink_dom_agent.h"
#include <iostream>
#include <regex>
#include <algorithm>
#include <sstream>

namespace chromium_playwright::dom {

// Blink DOM Agent Implementation
class BlinkDOMAgentImpl : public BlinkDOMAgent {
public:
    BlinkDOMAgentImpl() {
        // Initialize mock DOM
        InitializeMockDOM();
    }
    
    // Element finding
    std::vector<ElementHandle> FindElements(const std::string& selector, ElementSearchType type) override {
        std::vector<ElementHandle> elements;
        
        switch (type) {
            case ElementSearchType::CSS_SELECTOR:
                elements = FindByCSSSelector(selector);
                break;
            case ElementSearchType::XPATH:
                elements = FindByXPath(selector);
                break;
            case ElementSearchType::TEXT_CONTENT:
                elements = FindByText(selector);
                break;
            case ElementSearchType::ROLE:
                elements = FindByRole(selector);
                break;
            case ElementSearchType::PLACEHOLDER:
                elements = FindByPlaceholder(selector);
                break;
            case ElementSearchType::ALT_TEXT:
                elements = FindByAltText(selector);
                break;
            case ElementSearchType::TITLE:
                elements = FindByTitle(selector);
                break;
            case ElementSearchType::TEST_ID:
                elements = FindByTestId(selector);
                break;
        }
        
        return elements;
    }
    
    // Element actions
    bool ClickElement(const std::string& element_id) override {
        auto element = GetElement(element_id);
        if (!element) return false;
        
        std::cout << "🖱️  Clicked element: " << element_id << " (" << element->tag_name << ")" << std::endl;
        
        // Simulate click event
        element->clicked = true;
        element->last_click_time = GetCurrentTime();
        
        // Trigger click handlers
        TriggerEvent(element_id, "click");
        
        return true;
    }
    
    bool TypeText(const std::string& element_id, const std::string& text) override {
        auto element = GetElement(element_id);
        if (!element) return false;
        
        std::cout << "⌨️  Typed text into element: " << element_id << " (" << element->tag_name << ")" << std::endl;
        std::cout << "   Text: \"" << text << "\"" << std::endl;
        
        // Update element value
        element->value = text;
        element->text_content = text;
        
        // Trigger input event
        TriggerEvent(element_id, "input");
        
        return true;
    }
    
    bool HoverElement(const std::string& element_id) override {
        auto element = GetElement(element_id);
        if (!element) return false;
        
        std::cout << "🖱️  Hovered over element: " << element_id << " (" << element->tag_name << ")" << std::endl;
        
        // Simulate hover
        element->hovered = true;
        
        // Trigger hover events
        TriggerEvent(element_id, "mouseover");
        TriggerEvent(element_id, "mouseenter");
        
        return true;
    }
    
    bool FocusElement(const std::string& element_id) override {
        auto element = GetElement(element_id);
        if (!element) return false;
        
        std::cout << "🎯  Focused element: " << element_id << " (" << element->tag_name << ")" << std::endl;
        
        // Update focus
        if (focused_element_) {
            focused_element_->focused = false;
        }
        element->focused = true;
        focused_element_ = element;
        
        // Trigger focus event
        TriggerEvent(element_id, "focus");
        
        return true;
    }
    
    // Element properties
    std::string GetElementText(const std::string& element_id) override {
        auto element = GetElement(element_id);
        return element ? element->text_content : "";
    }
    
    std::string GetElementHTML(const std::string& element_id) override {
        auto element = GetElement(element_id);
        return element ? element->outer_html : "";
    }
    
    std::string GetElementAttribute(const std::string& element_id, const std::string& attribute_name) override {
        auto element = GetElement(element_id);
        if (!element) return "";
        
        auto it = element->attributes.find(attribute_name);
        return (it != element->attributes.end()) ? it->second : "";
    }
    
    bool SetElementAttribute(const std::string& element_id, const std::string& attribute_name, const std::string& value) override {
        auto element = GetElement(element_id);
        if (!element) return false;
        
        element->attributes[attribute_name] = value;
        std::cout << "🔧 Set attribute " << attribute_name << "=\"" << value << "\" on element " << element_id << std::endl;
        
        return true;
    }
    
    // Element state
    bool IsElementVisible(const std::string& element_id) override {
        auto element = GetElement(element_id);
        return element ? element->visible : false;
    }
    
    bool IsElementEnabled(const std::string& element_id) override {
        auto element = GetElement(element_id);
        return element ? element->enabled : false;
    }
    
    bool IsElementChecked(const std::string& element_id) override {
        auto element = GetElement(element_id);
        return element ? element->checked : false;
    }
    
    // Element geometry
    Rect GetElementBoundingBox(const std::string& element_id) override {
        auto element = GetElement(element_id);
        return element ? element->bounding_box : Rect{0, 0, 0, 0};
    }
    
    // JavaScript execution
    std::string ExecuteJavaScript(const std::string& script) override {
        std::cout << "🔧 Executing JavaScript: " << script << std::endl;
        
        // Simple JavaScript simulation
        if (script.find("document.title") != std::string::npos) {
            return "\"Mock Page Title\"";
        } else if (script.find("document.URL") != std::string::npos) {
            return "\"https://example.com\"";
        } else if (script.find("document.querySelector") != std::string::npos) {
            return "\"MockElement\"";
        } else if (script.find("window.location.href") != std::string::npos) {
            return "\"https://example.com\"";
        }
        
        return "\"undefined\"";
    }
    
    // Page navigation
    bool NavigateTo(const std::string& url) override {
        current_url_ = url;
        std::cout << "🌐 Navigated to: " << url << std::endl;
        
        // Simulate page load
        LoadPageContent(url);
        
        return true;
    }
    
    std::string GetCurrentURL() override {
        return current_url_;
    }
    
    std::string GetPageTitle() override {
        return "Mock Page Title";
    }
    
    // Event handling
    void AddEventListener(const std::string& element_id, const std::string& event_type, 
                         std::function<void()> callback) override {
        auto element = GetElement(element_id);
        if (!element) return;
        
        element->event_listeners[event_type].push_back(callback);
        std::cout << "👂 Added event listener for " << event_type << " on element " << element_id << std::endl;
    }
    
    void RemoveEventListener(const std::string& element_id, const std::string& event_type) override {
        auto element = GetElement(element_id);
        if (!element) return;
        
        element->event_listeners.erase(event_type);
        std::cout << "👂 Removed event listener for " << event_type << " on element " << element_id << std::endl;
    }

private:
    struct MockElement {
        std::string id;
        std::string tag_name;
        std::string text_content;
        std::string inner_html;
        std::string outer_html;
        std::string value;
        std::map<std::string, std::string> attributes;
        Rect bounding_box;
        bool visible = true;
        bool enabled = true;
        bool checked = false;
        bool focused = false;
        bool hovered = false;
        bool clicked = false;
        uint64_t last_click_time = 0;
        std::map<std::string, std::vector<std::function<void()>>> event_listeners;
    };
    
    std::map<std::string, std::unique_ptr<MockElement>> elements_;
    std::string current_url_;
    MockElement* focused_element_ = nullptr;
    int next_element_id_ = 1;
    
    void InitializeMockDOM() {
        // Create mock elements
        CreateElement("html", "html", "Mock HTML Content");
        CreateElement("head", "head", "");
        CreateElement("body", "body", "Mock Body Content");
        CreateElement("h1", "h1", "Welcome to Mock Page");
        CreateElement("p", "p", "This is a mock paragraph with some text.");
        CreateElement("button", "button", "Click Me");
        CreateElement("input", "input", "");
        CreateElement("img", "img", "");
        
        // Set up element properties
        auto button = GetElement("button");
        if (button) {
            button->attributes["id"] = "submit-btn";
            button->attributes["class"] = "btn btn-primary";
            button->bounding_box = {100, 200, 120, 40};
        }
        
        auto input = GetElement("input");
        if (input) {
            input->attributes["id"] = "search-input";
            input->attributes["type"] = "text";
            input->attributes["placeholder"] = "Enter search term";
            input->bounding_box = {50, 150, 200, 30};
        }
        
        auto img = GetElement("img");
        if (img) {
            img->attributes["id"] = "logo";
            img->attributes["src"] = "logo.png";
            img->attributes["alt"] = "Company Logo";
            img->bounding_box = {10, 10, 100, 50};
        }
    }
    
    void CreateElement(const std::string& id, const std::string& tag_name, const std::string& text_content) {
        auto element = std::make_unique<MockElement>();
        element->id = id;
        element->tag_name = tag_name;
        element->text_content = text_content;
        element->inner_html = text_content;
        element->outer_html = "<" + tag_name + ">" + text_content + "</" + tag_name + ">";
        element->bounding_box = {10, 10, 100, 30};
        
        elements_[id] = std::move(element);
    }
    
    MockElement* GetElement(const std::string& element_id) {
        auto it = elements_.find(element_id);
        return (it != elements_.end()) ? it->second.get() : nullptr;
    }
    
    std::vector<ElementHandle> FindByCSSSelector(const std::string& selector) {
        std::vector<ElementHandle> elements;
        
        // Simple CSS selector simulation
        if (selector == "button" || selector == ".btn") {
            elements.push_back(CreateElementHandle("button"));
        } else if (selector == "input" || selector == "input[type='text']") {
            elements.push_back(CreateElementHandle("input"));
        } else if (selector == "img" || selector == "img[alt]") {
            elements.push_back(CreateElementHandle("img"));
        } else if (selector == "h1") {
            elements.push_back(CreateElementHandle("h1"));
        } else if (selector == "p") {
            elements.push_back(CreateElementHandle("p"));
        }
        
        return elements;
    }
    
    std::vector<ElementHandle> FindByXPath(const std::string& xpath) {
        std::vector<ElementHandle> elements;
        
        // Simple XPath simulation
        if (xpath == "//button" || xpath == "//*[@id='submit-btn']") {
            elements.push_back(CreateElementHandle("button"));
        } else if (xpath == "//input" || xpath == "//*[@id='search-input']") {
            elements.push_back(CreateElementHandle("input"));
        }
        
        return elements;
    }
    
    std::vector<ElementHandle> FindByText(const std::string& text) {
        std::vector<ElementHandle> elements;
        
        // Find elements containing text
        for (const auto& pair : elements_) {
            if (pair.second->text_content.find(text) != std::string::npos) {
                elements.push_back(CreateElementHandle(pair.first));
            }
        }
        
        return elements;
    }
    
    std::vector<ElementHandle> FindByRole(const std::string& role) {
        std::vector<ElementHandle> elements;
        
        // Role-based finding
        if (role == "button") {
            elements.push_back(CreateElementHandle("button"));
        } else if (role == "textbox") {
            elements.push_back(CreateElementHandle("input"));
        } else if (role == "img") {
            elements.push_back(CreateElementHandle("img"));
        }
        
        return elements;
    }
    
    std::vector<ElementHandle> FindByPlaceholder(const std::string& placeholder) {
        std::vector<ElementHandle> elements;
        
        auto input = GetElement("input");
        if (input && input->attributes["placeholder"] == placeholder) {
            elements.push_back(CreateElementHandle("input"));
        }
        
        return elements;
    }
    
    std::vector<ElementHandle> FindByAltText(const std::string& alt_text) {
        std::vector<ElementHandle> elements;
        
        auto img = GetElement("img");
        if (img && img->attributes["alt"] == alt_text) {
            elements.push_back(CreateElementHandle("img"));
        }
        
        return elements;
    }
    
    std::vector<ElementHandle> FindByTitle(const std::string& title) {
        std::vector<ElementHandle> elements;
        
        // Find elements with title attribute
        for (const auto& pair : elements_) {
            auto it = pair.second->attributes.find("title");
            if (it != pair.second->attributes.end() && it->second == title) {
                elements.push_back(CreateElementHandle(pair.first));
            }
        }
        
        return elements;
    }
    
    std::vector<ElementHandle> FindByTestId(const std::string& test_id) {
        std::vector<ElementHandle> elements;
        
        // Find elements with data-testid attribute
        for (const auto& pair : elements_) {
            auto it = pair.second->attributes.find("data-testid");
            if (it != pair.second->attributes.end() && it->second == test_id) {
                elements.push_back(CreateElementHandle(pair.first));
            }
        }
        
        return elements;
    }
    
    ElementHandle CreateElementHandle(const std::string& element_id) {
        ElementHandle handle;
        handle.element_id = element_id;
        handle.tag_name = GetElement(element_id)->tag_name;
        handle.text_content = GetElement(element_id)->text_content;
        handle.bounding_box = GetElement(element_id)->bounding_box;
        return handle;
    }
    
    void LoadPageContent(const std::string& url) {
        // Simulate loading different content based on URL
        if (url.find("example.com") != std::string::npos) {
            // Example.com content
        } else if (url.find("github.com") != std::string::npos) {
            // GitHub content
        } else {
            // Default content
        }
    }
    
    void TriggerEvent(const std::string& element_id, const std::string& event_type) {
        auto element = GetElement(element_id);
        if (!element) return;
        
        auto it = element->event_listeners.find(event_type);
        if (it != element->event_listeners.end()) {
            for (const auto& callback : it->second) {
                callback();
            }
        }
    }
    
    uint64_t GetCurrentTime() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }
};

// Factory function
std::unique_ptr<BlinkDOMAgent> CreateBlinkDOMAgent() {
    return std::make_unique<BlinkDOMAgentImpl>();
}

} // namespace chromium_playwright::dom
