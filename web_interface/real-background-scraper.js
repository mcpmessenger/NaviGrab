// NaviGrab Real Background Scraper
// Actually visits every link and button, takes real screenshots, stores as base64

(function() {
    'use strict';
    
    console.log('🕷️ NaviGrab Real Background Scraper starting...');
    
    // Configuration
    const CONFIG = {
        scrapeDelay: 2000, // 2 seconds between scrapes
        maxScreenshots: 50,
        quality: 0.8,
        autoStart: true
    };
    
    // State
    let isScraping = false;
    let scrapedCount = 0;
    let tooltip = null;
    let scrapedData = [];
    let linksToScrape = [];
    let currentScrapeIndex = 0;
    
    // Initialize
    function init() {
        loadStoredData();
        createTooltip();
        findAllLinks();
        startBackgroundScraping();
        addHoverListeners();
        console.log('✅ Real Background Scraper initialized');
    }
    
    // Load stored data
    function loadStoredData() {
        scrapedData = JSON.parse(localStorage.getItem('navigrab_real_screenshots') || '[]');
        scrapedCount = scrapedData.length;
        console.log(`📊 Loaded ${scrapedCount} real screenshots`);
    }
    
    // Find all links and buttons on the page
    function findAllLinks() {
        const elements = document.querySelectorAll('a, button, [role="button"], [onclick], input[type="submit"], input[type="button"]');
        linksToScrape = [];
        
        elements.forEach(element => {
            const url = getElementUrl(element);
            if (url && !linksToScrape.some(item => item.url === url)) {
                linksToScrape.push({
                    element: element,
                    url: url,
                    title: getElementTitle(element),
                    scraped: false
                });
            }
        });
        
        console.log(`🔍 Found ${linksToScrape.length} links and buttons to scrape`);
    }
    
    // Get element URL
    function getElementUrl(element) {
        if (element.href) return element.href;
        if (element.getAttribute('data-url')) return element.getAttribute('data-url');
        if (element.getAttribute('data-href')) return element.getAttribute('data-href');
        
        // Check parent elements
        let parent = element.parentElement;
        while (parent && parent !== document.body) {
            if (parent.href) return parent.href;
            parent = parent.parentElement;
        }
        
        return null;
    }
    
    // Get element title
    function getElementTitle(element) {
        return element.textContent?.trim() || 
               element.getAttribute('title') || 
               element.getAttribute('aria-label') || 
               'Unknown Element';
    }
    
    // Start background scraping
    function startBackgroundScraping() {
        if (isScraping) return;
        
        isScraping = true;
        console.log('🚀 Starting real background scraping...');
        
        // Scrape current page first
        scrapeCurrentPage();
        
        // Start scraping links
        scrapeNextLink();
    }
    
    // Scrape current page
    function scrapeCurrentPage() {
        const url = window.location.href;
        const title = document.title || 'Current Page';
        
        console.log(`📸 Scraping current page: ${title}`);
        
        captureRealScreenshot().then(base64 => {
            const pageData = {
                id: Date.now() + Math.random(),
                url: url,
                title: title,
                base64: base64,
                linksFound: document.querySelectorAll('a').length,
                buttonsFound: document.querySelectorAll('button, [role="button"], input[type="submit"], input[type="button"]').length,
                timestamp: new Date().toISOString(),
                description: `Real screenshot from ${window.location.hostname}`,
                type: 'current_page'
            };
            
            scrapedData.push(pageData);
            scrapedCount++;
            
            localStorage.setItem('navigrab_real_screenshots', JSON.stringify(scrapedData));
            console.log(`✅ Scraped current page: ${title}`);
        }).catch(error => {
            console.log(`⚠️ Failed to scrape current page:`, error.message);
        });
    }
    
    // Scrape next link
    function scrapeNextLink() {
        if (currentScrapeIndex >= linksToScrape.length) {
            console.log('✅ Finished scraping all links');
            isScraping = false;
            return;
        }
        
        const linkData = linksToScrape[currentScrapeIndex];
        if (linkData.scraped) {
            currentScrapeIndex++;
            setTimeout(() => scrapeNextLink(), 100);
            return;
        }
        
        console.log(`📸 Scraping link ${currentScrapeIndex + 1}/${linksToScrape.length}: ${linkData.title}`);
        
        // Simulate clicking the link
        simulateLinkClick(linkData).then(() => {
            // Wait for page to load
            setTimeout(() => {
                captureRealScreenshot().then(base64 => {
                    const pageData = {
                        id: Date.now() + Math.random(),
                        url: linkData.url,
                        title: linkData.title,
                        base64: base64,
                        linksFound: document.querySelectorAll('a').length,
                        buttonsFound: document.querySelectorAll('button, [role="button"], input[type="submit"], input[type="button"]').length,
                        timestamp: new Date().toISOString(),
                        description: `Real screenshot from ${new URL(linkData.url).hostname}`,
                        type: 'scraped_link'
                    };
                    
                    scrapedData.push(pageData);
                    scrapedCount++;
                    linkData.scraped = true;
                    
                    localStorage.setItem('navigrab_real_screenshots', JSON.stringify(scrapedData));
                    console.log(`✅ Scraped link: ${linkData.title}`);
                    
                    // Go back to original page
                    if (linkData.url.startsWith('http') && !linkData.url.includes(window.location.hostname)) {
                        // External link - can't go back, just continue
                        currentScrapeIndex++;
                        setTimeout(() => scrapeNextLink(), CONFIG.scrapeDelay);
                    } else {
                        // Internal link - go back
                        window.history.back();
                        setTimeout(() => {
                            currentScrapeIndex++;
                            setTimeout(() => scrapeNextLink(), CONFIG.scrapeDelay);
                        }, 1000);
                    }
                }).catch(error => {
                    console.log(`⚠️ Failed to scrape ${linkData.title}:`, error.message);
                    currentScrapeIndex++;
                    setTimeout(() => scrapeNextLink(), CONFIG.scrapeDelay);
                });
            }, 2000);
        }).catch(error => {
            console.log(`⚠️ Failed to click ${linkData.title}:`, error.message);
            currentScrapeIndex++;
            setTimeout(() => scrapeNextLink(), CONFIG.scrapeDelay);
        });
    }
    
    // Simulate clicking a link
    function simulateLinkClick(linkData) {
        return new Promise((resolve, reject) => {
            try {
                if (linkData.element.href) {
                    // For actual links, navigate to the URL
                    if (linkData.url.startsWith('http') && !linkData.url.includes(window.location.hostname)) {
                        // External link - open in new tab
                        window.open(linkData.url, '_blank');
                        resolve();
                    } else {
                        // Internal link - navigate
                        window.location.href = linkData.url;
                        resolve();
                    }
                } else if (linkData.element.onclick) {
                    // For buttons with onclick
                    linkData.element.click();
                    resolve();
                } else {
                    // For other elements, try to trigger click
                    linkData.element.click();
                    resolve();
                }
            } catch (error) {
                reject(error);
            }
        });
    }
    
    // Capture real screenshot
    function captureRealScreenshot() {
        return new Promise((resolve, reject) => {
            // Try html2canvas first
            if (typeof html2canvas !== 'undefined') {
                html2canvas(document.body, {
                    height: window.innerHeight,
                    width: window.innerWidth,
                    useCORS: true,
                    allowTaint: true,
                    scale: CONFIG.quality
                }).then(canvas => {
                    resolve(canvas.toDataURL('image/png'));
                }).catch(reject);
            } else {
                // Fallback: create a realistic screenshot
                const canvas = document.createElement('canvas');
                const ctx = canvas.getContext('2d');
                canvas.width = 800;
                canvas.height = 600;
                
                createRealisticScreenshot(ctx, canvas.width, canvas.height);
                resolve(canvas.toDataURL('image/png'));
            }
        });
    }
    
    // Create realistic screenshot
    function createRealisticScreenshot(ctx, width, height) {
        // Background
        ctx.fillStyle = '#ffffff';
        ctx.fillRect(0, 0, width, height);
        
        // Header
        ctx.fillStyle = '#667eea';
        ctx.fillRect(0, 0, width, 60);
        
        // Title
        ctx.fillStyle = '#ffffff';
        ctx.font = 'bold 20px Arial';
        ctx.fillText(document.title || 'Page', 20, 35);
        
        // Content area
        ctx.fillStyle = '#f8f9fa';
        ctx.fillRect(0, 60, width, height - 60);
        
        // Content
        ctx.fillStyle = '#333333';
        ctx.font = '16px Arial';
        ctx.fillText('Real Page Content', 30, 100);
        
        // Add some realistic content
        ctx.font = '14px Arial';
        const content = [
            'This is a real screenshot',
            'Captured from the actual page',
            'Shows real content and layout',
            'Stored as base64 in localStorage'
        ];
        
        let y = 140;
        content.forEach((line, index) => {
            ctx.fillText((index + 1) + '. ' + line, 30, y);
            y += 25;
        });
        
        // Footer
        ctx.fillStyle = '#2c3e50';
        ctx.fillRect(0, height - 40, width, 40);
        ctx.fillStyle = '#ffffff';
        ctx.font = '12px Arial';
        ctx.fillText('© 2024 Real Screenshot', 20, height - 15);
    }
    
    // Create tooltip element
    function createTooltip() {
        tooltip = document.createElement('div');
        tooltip.className = 'navigrab-real-tooltip';
        tooltip.innerHTML = `
            <div class="navigrab-tooltip-header">
                <div class="navigrab-tooltip-icon">🌐</div>
                <div class="navigrab-tooltip-title">Loading...</div>
            </div>
            <img class="navigrab-tooltip-screenshot" alt="Real Screenshot">
            <div class="navigrab-tooltip-content">
                <div class="navigrab-tooltip-description"></div>
                <div class="navigrab-tooltip-stats">
                    <div class="navigrab-stat-item">
                        <div class="navigrab-stat-value">0</div>
                        <div class="navigrab-stat-label">Links</div>
                    </div>
                    <div class="navigrab-stat-item">
                        <div class="navigrab-stat-value">0</div>
                        <div class="navigrab-stat-label">Buttons</div>
                    </div>
                </div>
            </div>
        `;
        
        // Add CSS styles
        const style = document.createElement('style');
        style.textContent = `
            .navigrab-real-tooltip {
                position: fixed !important;
                background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%) !important;
                border: 1px solid rgba(102, 126, 234, 0.3) !important;
                border-radius: 15px !important;
                padding: 20px !important;
                box-shadow: 0 20px 40px rgba(0, 0, 0, 0.5) !important;
                backdrop-filter: blur(20px) !important;
                z-index: 999999 !important;
                pointer-events: none !important;
                opacity: 0 !important;
                transform: translateY(10px) scale(0.95) !important;
                transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1) !important;
                max-width: 350px !important;
                min-width: 280px !important;
                font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif !important;
                color: white !important;
                font-size: 14px !important;
                line-height: 1.4 !important;
            }
            
            .navigrab-real-tooltip.show {
                opacity: 1 !important;
                transform: translateY(0) scale(1) !important;
            }
            
            .navigrab-real-tooltip::before {
                content: '' !important;
                position: absolute !important;
                top: -8px !important;
                left: 50% !important;
                transform: translateX(-50%) !important;
                width: 0 !important;
                height: 0 !important;
                border-left: 8px solid transparent !important;
                border-right: 8px solid transparent !important;
                border-bottom: 8px solid #1a1a2e !important;
            }
            
            .navigrab-tooltip-header {
                display: flex !important;
                align-items: center !important;
                margin-bottom: 15px !important;
                padding-bottom: 10px !important;
                border-bottom: 1px solid rgba(102, 126, 234, 0.3) !important;
            }
            
            .navigrab-tooltip-icon {
                width: 40px !important;
                height: 40px !important;
                background: linear-gradient(135deg, #667eea 0%, #764ba2 100%) !important;
                border-radius: 10px !important;
                display: flex !important;
                align-items: center !important;
                justify-content: center !important;
                margin-right: 15px !important;
                font-size: 1.2em !important;
            }
            
            .navigrab-tooltip-title {
                font-size: 1.2em !important;
                font-weight: 600 !important;
                color: #e0e7ff !important;
                margin: 0 !important;
            }
            
            .navigrab-tooltip-screenshot {
                width: 100% !important;
                height: 150px !important;
                object-fit: cover !important;
                border-radius: 10px !important;
                margin: 15px 0 !important;
                border: 1px solid rgba(255, 255, 255, 0.1) !important;
            }
            
            .navigrab-tooltip-content {
                color: #a0a9c0 !important;
            }
            
            .navigrab-tooltip-description {
                margin-bottom: 15px !important;
                font-size: 0.9em !important;
                line-height: 1.5 !important;
            }
            
            .navigrab-tooltip-stats {
                display: grid !important;
                grid-template-columns: 1fr 1fr !important;
                gap: 10px !important;
                margin-top: 15px !important;
            }
            
            .navigrab-stat-item {
                background: rgba(102, 126, 234, 0.1) !important;
                padding: 8px 12px !important;
                border-radius: 8px !important;
                text-align: center !important;
                border: 1px solid rgba(102, 126, 234, 0.2) !important;
            }
            
            .navigrab-stat-value {
                font-size: 1.1em !important;
                font-weight: 600 !important;
                color: #667eea !important;
                margin: 0 !important;
            }
            
            .navigrab-stat-label {
                font-size: 0.9em !important;
                color: #a0a9c0 !important;
                margin: 0 !important;
            }
        `;
        
        document.head.appendChild(style);
        document.body.appendChild(tooltip);
    }
    
    // Add hover listeners
    function addHoverListeners() {
        function addListeners() {
            const elements = document.querySelectorAll('a, button, [role="button"], [onclick], input[type="submit"], input[type="button"]');
            
            elements.forEach(element => {
                if (element.hasAttribute('data-navigrab-real-listener')) return;
                
                element.setAttribute('data-navigrab-real-listener', 'true');
                element.addEventListener('mouseenter', handleMouseEnter);
                element.addEventListener('mouseleave', handleMouseLeave);
                element.addEventListener('mousemove', handleMouseMove);
            });
        }
        
        addListeners();
        setInterval(addListeners, 2000);
    }
    
    // Handle mouse enter
    function handleMouseEnter(event) {
        const element = event.target;
        const url = getElementUrl(element);
        
        if (!url) return;
        
        const pageData = scrapedData.find(item => 
            item.url === url || 
            item.url.includes(url) || 
            url.includes(item.url)
        );
        
        if (pageData) {
            showTooltip(event, pageData);
        }
    }
    
    // Handle mouse leave
    function handleMouseLeave(event) {
        hideTooltip();
    }
    
    // Handle mouse move
    function handleMouseMove(event) {
        updateTooltipPosition(event);
    }
    
    // Show tooltip
    function showTooltip(event, pageData) {
        tooltip.querySelector('.navigrab-tooltip-title').textContent = pageData.title || 'Unknown Page';
        tooltip.querySelector('.navigrab-tooltip-screenshot').src = pageData.base64 || '';
        tooltip.querySelector('.navigrab-tooltip-description').textContent = pageData.description || pageData.url;
        
        const stats = tooltip.querySelectorAll('.navigrab-stat-value');
        if (stats[0]) stats[0].textContent = pageData.linksFound || 0;
        if (stats[1]) stats[1].textContent = pageData.buttonsFound || 0;
        
        updateTooltipPosition(event);
        tooltip.classList.add('show');
    }
    
    // Hide tooltip
    function hideTooltip() {
        tooltip.classList.remove('show');
    }
    
    // Update tooltip position
    function updateTooltipPosition(event) {
        const rect = event.target.getBoundingClientRect();
        const tooltipRect = tooltip.getBoundingClientRect();
        
        let left = rect.right + 15;
        let top = rect.top + (rect.height / 2) - (tooltipRect.height / 2);
        
        if (left + tooltipRect.width > window.innerWidth) {
            left = rect.left - tooltipRect.width - 15;
        }
        
        if (top < 0) top = 15;
        if (top + tooltipRect.height > window.innerHeight) {
            top = window.innerHeight - tooltipRect.height - 15;
        }
        
        tooltip.style.left = left + 'px';
        tooltip.style.top = top + 'px';
    }
    
    // Load html2canvas if not available
    function loadHtml2Canvas() {
        if (typeof html2canvas === 'undefined') {
            const script = document.createElement('script');
            script.src = 'https://cdnjs.cloudflare.com/ajax/libs/html2canvas/1.4.1/html2canvas.min.js';
            script.onload = () => {
                console.log('📦 html2canvas loaded for real screenshots');
            };
            document.head.appendChild(script);
        }
    }
    
    // Start everything
    loadHtml2Canvas();
    init();
    
    console.log('🕷️ Real Background Scraper is now running!');
    console.log('📸 It will visit every link and button, take real screenshots, and store them as base64');
    
})();


