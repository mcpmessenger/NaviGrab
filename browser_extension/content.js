// NaviGrab Universal Tooltip System - Content Script
// This script runs on every website and adds beautiful tooltips to all links and buttons

(function() {
    'use strict';
    
    console.log('🕷️ NaviGrab Universal Tooltip System loaded');
    
    // Create tooltip element
    const tooltip = document.createElement('div');
    tooltip.className = 'navigrab-tooltip';
    tooltip.innerHTML = `
        <div class="navigrab-tooltip-header">
            <div class="navigrab-tooltip-icon">🌐</div>
            <div class="navigrab-tooltip-title">Loading...</div>
        </div>
        <img class="navigrab-tooltip-screenshot" alt="Screenshot">
        <div class="navigrab-tooltip-content">
            <div class="navigrab-tooltip-description"></div>
            <div class="navigrab-tooltip-stats">
                <div class="navigrab-stat-item">
                    <div class="navigrab-stat-value">0</div>
                    <div class="navigrab-stat-label">Links Found</div>
                </div>
                <div class="navigrab-stat-item">
                    <div class="navigrab-stat-value">0</div>
                    <div class="navigrab-stat-label">Elements Clicked</div>
                </div>
                <div class="navigrab-stat-item">
                    <div class="navigrab-stat-value">0</div>
                    <div class="navigrab-stat-label">Depth Level</div>
                </div>
                <div class="navigrab-stat-item">
                    <div class="navigrab-stat-value">0s</div>
                    <div class="navigrab-stat-label">Load Time</div>
                </div>
            </div>
        </div>
    `;
    
    document.body.appendChild(tooltip);
    
    // Load scraped data from storage
    let scrapedData = [];
    
    function loadScrapedData() {
        chrome.storage.local.get(['navigrab_screenshots'], function(result) {
            scrapedData = result.navigrab_screenshots || [];
            console.log(`🔄 Loaded ${scrapedData.length} scraped pages`);
        });
    }
    
    // Add hover listeners to all links and buttons
    function addTooltipListeners() {
        const elements = document.querySelectorAll('a, button, [role="button"], [onclick], input[type="submit"], input[type="button"]');
        
        elements.forEach(element => {
            // Skip if already has listeners
            if (element.hasAttribute('data-navigrab-listener')) return;
            
            element.setAttribute('data-navigrab-listener', 'true');
            element.addEventListener('mouseenter', handleMouseEnter);
            element.addEventListener('mouseleave', handleMouseLeave);
            element.addEventListener('mousemove', handleMouseMove);
        });
    }
    
    function handleMouseEnter(event) {
        const element = event.target;
        const url = getElementUrl(element);
        
        if (!url) return;
        
        // Find scraped data for this URL
        const pageData = scrapedData.find(item => 
            item.url === url || 
            item.url.includes(url) || 
            url.includes(item.url)
        );
        
        if (pageData) {
            showTooltip(event, pageData);
        } else {
            // If no scraped data, try to scrape this URL
            attemptScrapeAndShow(url, event);
        }
    }
    
    function handleMouseLeave(event) {
        hideTooltip();
    }
    
    function handleMouseMove(event) {
        updateTooltipPosition(event);
    }
    
    function getElementUrl(element) {
        // Try different ways to get URL
        if (element.href) return element.href;
        if (element.getAttribute('data-url')) return element.getAttribute('data-url');
        if (element.getAttribute('data-href')) return element.getAttribute('data-href');
        if (element.onclick) {
            // Try to extract URL from onclick handler
            const onclickStr = element.onclick.toString();
            const urlMatch = onclickStr.match(/https?:\/\/[^\s'"]+/);
            if (urlMatch) return urlMatch[0];
        }
        
        // Check parent elements
        let parent = element.parentElement;
        while (parent && parent !== document.body) {
            if (parent.href) return parent.href;
            parent = parent.parentElement;
        }
        
        return null;
    }
    
    function showTooltip(event, pageData) {
        // Update tooltip content
        tooltip.querySelector('.navigrab-tooltip-title').textContent = pageData.title || 'Unknown Page';
        tooltip.querySelector('.navigrab-tooltip-screenshot').src = pageData.base64 || '';
        tooltip.querySelector('.navigrab-tooltip-description').textContent = pageData.description || pageData.url;
        
        // Update stats
        const stats = tooltip.querySelectorAll('.navigrab-stat-value');
        if (stats[0]) stats[0].textContent = pageData.linksFound || 0;
        if (stats[1]) stats[1].textContent = pageData.elementsClicked || 0;
        if (stats[2]) stats[2].textContent = pageData.depth || 0;
        if (stats[3]) stats[3].textContent = pageData.loadTime || '0s';
        
        // Position tooltip
        updateTooltipPosition(event);
        
        // Show tooltip
        tooltip.classList.add('show');
    }
    
    function hideTooltip() {
        tooltip.classList.remove('show');
    }
    
    function updateTooltipPosition(event) {
        const rect = event.target.getBoundingClientRect();
        const tooltipRect = tooltip.getBoundingClientRect();
        
        let left = rect.right + 15;
        let top = rect.top + (rect.height / 2) - (tooltipRect.height / 2);
        
        // Adjust if tooltip goes off screen
        if (left + tooltipRect.width > window.innerWidth) {
            left = rect.left - tooltipRect.width - 15;
        }
        
        if (top < 0) {
            top = 15;
        }
        
        if (top + tooltipRect.height > window.innerHeight) {
            top = window.innerHeight - tooltipRect.height - 15;
        }
        
        tooltip.style.left = left + 'px';
        tooltip.style.top = top + 'px';
    }
    
    function attemptScrapeAndShow(url, event) {
        // Send message to background script to scrape this URL
        chrome.runtime.sendMessage({
            action: 'scrapeUrl',
            url: url
        }, function(response) {
            if (response && response.success) {
                // Add to scraped data
                scrapedData.push(response.data);
                
                // Show tooltip
                showTooltip(event, response.data);
            }
        });
    }
    
    // Initialize
    loadScrapedData();
    addTooltipListeners();
    
    // Re-scan for new elements periodically
    setInterval(addTooltipListeners, 2000);
    
    // Listen for storage changes
    chrome.storage.onChanged.addListener(function(changes, namespace) {
        if (changes.navigrab_screenshots) {
            scrapedData = changes.navigrab_screenshots.newValue || [];
            console.log(`🔄 Updated scraped data: ${scrapedData.length} pages`);
        }
    });
    
    // Listen for messages from popup
    chrome.runtime.onMessage.addListener(function(request, sender, sendResponse) {
        if (request.action === 'refreshData') {
            loadScrapedData();
            sendResponse({success: true});
        }
    });
    
})();


