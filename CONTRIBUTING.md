# Contributing to NaviGrab

Thank you for your interest in contributing to NaviGrab! This document provides guidelines for contributing to the project.

## 🚀 Getting Started

### Prerequisites
- Windows 10/11
- Visual Studio 2022 Build Tools
- CMake 3.5+
- Git

### Development Setup
1. Fork the repository
2. Clone your fork: `git clone https://github.com/yourusername/NaviGrab.git`
3. Create a feature branch: `git checkout -b feature/your-feature-name`
4. Make your changes
5. Test your changes
6. Submit a pull request

## 🏗️ Project Structure

```
NaviGrab/
├── include/chromium_playwright/     # Header files
├── src/                            # Source files
├── examples/                        # Example programs
├── web_interface/                   # Navi Grab HTML interface
├── tests/                          # Unit tests
├── docs/                           # Documentation
├── CMakeLists.txt                  # Build configuration
└── README.md                       # Project documentation
```

## 📝 Code Style

### C++ Guidelines
- Use C++20 features
- Follow RAII principles
- Use smart pointers (`std::unique_ptr`, `std::shared_ptr`)
- Prefer `const` correctness
- Use meaningful variable and function names
- Add comments for complex logic

### Example
```cpp
class ImageProcessor {
public:
    explicit ImageProcessor(const std::string& config_path);
    ~ImageProcessor() = default;
    
    std::unique_ptr<ImageData> ProcessImage(const ImageData& input) const;
    
private:
    std::string config_path_;
    bool ValidateInput(const ImageData& input) const;
};
```

## 🧪 Testing

### Unit Tests
- Write tests for all new functionality
- Use descriptive test names
- Test both success and failure cases
- Aim for high code coverage

### Example Test
```cpp
TEST(ImageProcessorTest, ProcessImage_ValidInput_ReturnsProcessedImage) {
    // Arrange
    ImageProcessor processor("test_config.json");
    ImageData input(100, 100, 3);
    
    // Act
    auto result = processor.ProcessImage(input);
    
    // Assert
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->width, 100);
    EXPECT_EQ(result->height, 100);
}
```

## 📋 Pull Request Process

1. **Create a feature branch** from `main`
2. **Make your changes** following the code style
3. **Add tests** for new functionality
4. **Update documentation** if needed
5. **Run tests** to ensure everything passes
6. **Submit a pull request** with a clear description

### Pull Request Template
```markdown
## Description
Brief description of changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Breaking change
- [ ] Documentation update

## Testing
- [ ] Unit tests pass
- [ ] Integration tests pass
- [ ] Manual testing completed

## Checklist
- [ ] Code follows style guidelines
- [ ] Self-review completed
- [ ] Documentation updated
- [ ] Tests added/updated
```

## 🐛 Bug Reports

When reporting bugs, please include:
- Description of the issue
- Steps to reproduce
- Expected behavior
- Actual behavior
- Environment details (OS, compiler, etc.)
- Screenshots if applicable

## 💡 Feature Requests

When requesting features, please include:
- Description of the feature
- Use case and motivation
- Proposed implementation (if you have ideas)
- Any alternatives considered

## 📚 Documentation

- Update README.md for significant changes
- Add inline comments for complex code
- Update API documentation
- Include examples for new features

## 🤝 Community Guidelines

- Be respectful and inclusive
- Help others learn and grow
- Provide constructive feedback
- Follow the code of conduct

## 📞 Getting Help

- Check existing issues and discussions
- Ask questions in discussions
- Contact maintainers if needed

Thank you for contributing to NaviGrab! 🎉
