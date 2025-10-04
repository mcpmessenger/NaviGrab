# NaviGrab Universal Tooltip System - Browser Extension

## 🕷️ Overview

This browser extension provides beautiful tooltips with real screenshots on every link and button across all websites. It proactively scrapes websites, captures real screenshots, and stores them in localStorage for instant tooltip display.

## ✨ Features

- **Universal Tooltips**: Works on any website with any link or button
- **Real Screenshots**: Captures actual screenshots of scraped pages
- **Proactive Scraping**: Automatically scrapes popular websites in the background
- **Beautiful UI**: Dark theme with glowing effects and smooth animations
- **Local Storage**: Stores all data locally for privacy
- **Real-time Updates**: Tooltips update as new data is scraped

## 🚀 Installation

### Method 1: Load as Unpacked Extension (Development)

1. Open Chrome and go to `chrome://extensions/`
2. Enable "Developer mode" in the top right
3. Click "Load unpacked" and select the `browser_extension` folder
4. The extension will be installed and ready to use

### Method 2: Package and Install

1. Zip the `browser_extension` folder
2. Go to `chrome://extensions/`
3. Enable "Developer mode"
4. Click "Load unpacked" and select the zip file

## 🎯 How It Works

1. **Content Script Injection**: The extension injects a content script into every website
2. **Element Detection**: Automatically detects all links, buttons, and clickable elements
3. **Hover Detection**: Listens for mouse hover events on detected elements
4. **Data Lookup**: Checks localStorage for scraped data matching the hovered element's URL
5. **Tooltip Display**: Shows beautiful tooltip with real screenshot if data exists
6. **Proactive Scraping**: Background script scrapes popular websites and stores screenshots

## 🔧 Configuration

### Scraping Settings

The extension automatically scrapes these popular websites:
- Hacker News (news.ycombinator.com)
- GitHub (github.com)
- Stack Overflow (stackoverflow.com)
- Reddit Programming (reddit.com/r/programming)
- Dev.to (dev.to)

### Storage

All scraped data is stored in Chrome's local storage under the key `navigrab_screenshots`. Each entry contains:
- `url`: The scraped URL
- `title`: Page title
- `base64`: Screenshot as base64 string
- `linksFound`: Number of links found on the page
- `elementsClicked`: Number of elements clicked during scraping
- `depth`: Scraping depth level
- `timestamp`: When the page was scraped

## 🎨 Customization

### Tooltip Styling

Edit `tooltip.css` to customize the appearance:
- Colors and gradients
- Border radius and shadows
- Animation timing
- Font sizes and spacing

### Scraping Behavior

Modify `background.js` to:
- Add more websites to scrape
- Change scraping frequency
- Adjust screenshot quality
- Modify data collection

## 🐛 Troubleshooting

### Tooltips Not Showing

1. Check if the extension is enabled
2. Refresh the page
3. Check browser console for errors
4. Verify scraped data exists in storage

### Scraping Not Working

1. Check extension permissions
2. Verify target websites are accessible
3. Check background script console for errors
4. Ensure storage quota is not exceeded

### Performance Issues

1. Reduce scraping frequency in background.js
2. Lower screenshot quality
3. Clear old scraped data
4. Disable scraping for specific sites

## 📊 Monitoring

Use the extension popup to:
- View scraping statistics
- Start/stop scraping
- Test tooltips
- Clear stored data
- Refresh data

## 🔒 Privacy

- All data is stored locally in your browser
- No data is sent to external servers
- Scraping only occurs on websites you visit
- You can clear all data at any time

## 🚀 Development

### File Structure

```
browser_extension/
├── manifest.json          # Extension manifest
├── content.js             # Content script (injected into websites)
├── background.js          # Background script (handles scraping)
├── popup.html             # Extension popup UI
├── popup.js               # Popup functionality
├── tooltip.css            # Tooltip styles
└── README.md              # This file
```

### Building

1. Make changes to the source files
2. Reload the extension in Chrome
3. Test on various websites
4. Package for distribution

### Testing

1. Load the extension in developer mode
2. Visit various websites
3. Hover over links and buttons
4. Check popup for statistics
5. Verify tooltips appear with real screenshots

## 📝 License

This project is part of the NaviGrab C++ Playwright clone system.

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## 🐛 Bug Reports

Please report bugs by:
1. Describing the issue
2. Providing steps to reproduce
3. Including browser version and OS
4. Attaching console logs if available

## 💡 Feature Requests

We welcome feature requests! Please describe:
1. What you'd like to see
2. How it would improve the tool
3. Any implementation ideas you have

---

**NaviGrab Universal Tooltip System** - Beautiful tooltips with real screenshots on every link and button! 🕷️✨


