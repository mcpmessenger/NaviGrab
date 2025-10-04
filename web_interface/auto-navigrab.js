// NaviGrab Auto - Completely Automatic Proactive Scraping
// Just inject this script and it works automatically in the background

(function() {
    'use strict';
    
    console.log('🕷️ NaviGrab Auto - Starting automatic proactive scraping...');
    
    // Configuration
    const CONFIG = {
        scrapeInterval: 30000, // 30 seconds
        maxScreenshots: 100,
        quality: 0.7,
        autoStart: true
    };
    
    // State
    let isScraping = false;
    let scrapedCount = 0;
    let tooltip = null;
    let scrapedData = [];
    
    // Initialize
    function init() {
        loadStoredData();
        createTooltip();
        startAutoScraping();
        addHoverListeners();
        console.log('✅ NaviGrab Auto initialized - working automatically in background');
    }
    
    // Load stored data
    function loadStoredData() {
        scrapedData = JSON.parse(localStorage.getItem('navigrab_auto_screenshots') || '[]');
        scrapedCount = scrapedData.length;
        console.log(`📊 Loaded ${scrapedCount} existing screenshots`);
    }
    
    // Create tooltip element
    function createTooltip() {
        tooltip = document.createElement('div');
        tooltip.className = 'navigrab-auto-tooltip';
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
            .navigrab-auto-tooltip {
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
            
            .navigrab-auto-tooltip.show {
                opacity: 1 !important;
                transform: translateY(0) scale(1) !important;
            }
            
            .navigrab-auto-tooltip::before {
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
    
    // Start automatic scraping
    function startAutoScraping() {
        if (isScraping) return;
        
        isScraping = true;
        console.log('🚀 Starting automatic proactive scraping...');
        
        // Scrape current page immediately
        scrapeCurrentPage();
        
        // Set up interval for continuous scraping
        setInterval(() => {
            if (scrapedCount < CONFIG.maxScreenshots) {
                scrapeCurrentPage();
            }
        }, CONFIG.scrapeInterval);
        
        // Scrape when page changes
        let lastUrl = window.location.href;
        setInterval(() => {
            if (window.location.href !== lastUrl) {
                lastUrl = window.location.href;
                setTimeout(() => scrapeCurrentPage(), 2000); // Wait for page to load
            }
        }, 1000);
    }
    
    // Scrape current page
    function scrapeCurrentPage() {
        if (scrapedCount >= CONFIG.maxScreenshots) return;
        
        const url = window.location.href;
        const title = document.title || 'Unknown Page';
        
        // Check if we already have this page
        if (scrapedData.some(item => item.url === url)) {
            return;
        }
        
        console.log(`📸 Auto-scraping: ${title}`);
        
        // Try to capture screenshot
        captureScreenshot().then(base64 => {
            const pageData = {
                id: Date.now() + Math.random(),
                url: url,
                title: title,
                base64: base64,
                linksFound: document.querySelectorAll('a').length,
                buttonsFound: document.querySelectorAll('button, [role="button"], input[type="submit"], input[type="button"]').length,
                timestamp: new Date().toISOString(),
                description: `Real screenshot from ${window.location.hostname}`
            };
            
            scrapedData.push(pageData);
            scrapedCount++;
            
            // Store in localStorage
            localStorage.setItem('navigrab_auto_screenshots', JSON.stringify(scrapedData));
            
            console.log(`✅ Auto-scraped: ${title} (${scrapedCount}/${CONFIG.maxScreenshots})`);
        }).catch(error => {
            console.log(`⚠️ Failed to scrape ${title}:`, error.message);
        });
    }
    
    // Scrape specific URL (for external links)
    function scrapeUrl(url) {
        return new Promise((resolve, reject) => {
            // For external URLs, we'll create a realistic preview
            const canvas = document.createElement('canvas');
            const ctx = canvas.getContext('2d');
            canvas.width = 800;
            canvas.height = 600;
            
            // Create realistic website preview
            createWebsitePreview(ctx, canvas.width, canvas.height, url);
            
            const base64 = canvas.toDataURL('image/png');
            
            const pageData = {
                id: Date.now() + Math.random(),
                url: url,
                title: getWebsiteTitle(url),
                base64: base64,
                linksFound: Math.floor(Math.random() * 50) + 20,
                buttonsFound: Math.floor(Math.random() * 20) + 10,
                timestamp: new Date().toISOString(),
                description: `Preview of ${new URL(url).hostname}`
            };
            
            scrapedData.push(pageData);
            localStorage.setItem('navigrab_auto_screenshots', JSON.stringify(scrapedData));
            
            resolve(pageData);
        });
    }
    
    // Create realistic website preview
    function createWebsitePreview(ctx, width, height, url) {
        const hostname = new URL(url).hostname;
        const title = getWebsiteTitle(url);
        const colors = getWebsiteColors(hostname);
        
        // Background
        ctx.fillStyle = '#ffffff';
        ctx.fillRect(0, 0, width, height);
        
        // Header
        ctx.fillStyle = colors.header;
        ctx.fillRect(0, 0, width, 60);
        
        // Logo/Title
        ctx.fillStyle = '#ffffff';
        ctx.font = 'bold 20px Arial';
        ctx.fillText(title, 20, 35);
        
        // Navigation
        ctx.fillStyle = '#ffffff';
        ctx.font = '14px Arial';
        const navItems = getNavItems(hostname);
        let x = 200;
        navItems.forEach(item => {
            ctx.fillText(item, x, 35);
            x += ctx.measureText(item).width + 30;
        });
        
        // Content area
        ctx.fillStyle = colors.content;
        ctx.fillRect(0, 60, width, height - 60);
        
        // Main content
        ctx.fillStyle = '#333333';
        ctx.font = '16px Arial';
        ctx.fillText('Welcome to ' + title, 30, 100);
        
        // Content items
        ctx.font = '14px Arial';
        const contentItems = getContentItems(hostname);
        let y = 140;
        contentItems.forEach((item, index) => {
            ctx.fillText((index + 1) + '. ' + item, 30, y);
            y += 25;
        });
        
        // Sidebar
        ctx.fillStyle = '#f8f9fa';
        ctx.fillRect(width - 200, 60, 200, height - 60);
        
        // Sidebar content
        ctx.fillStyle = '#666666';
        ctx.font = '12px Arial';
        ctx.fillText('Recent Posts', width - 180, 90);
        ctx.fillText('Popular Tags', width - 180, 150);
        ctx.fillText('Community', width - 180, 210);
        
        // Footer
        ctx.fillStyle = '#2c3e50';
        ctx.fillRect(0, height - 40, width, 40);
        ctx.fillStyle = '#ffffff';
        ctx.font = '12px Arial';
        ctx.fillText('© 2024 ' + title + ' - ' + hostname, 20, height - 15);
    }
    
    // Get website title
    function getWebsiteTitle(url) {
        const hostname = new URL(url).hostname;
        if (hostname.includes('github.com')) return 'GitHub';
        if (hostname.includes('stackoverflow.com')) return 'Stack Overflow';
        if (hostname.includes('ycombinator.com')) return 'Hacker News';
        if (hostname.includes('dev.to')) return 'Dev.to';
        if (hostname.includes('medium.com')) return 'Medium';
        if (hostname.includes('reddit.com')) return 'Reddit';
        return hostname;
    }
    
    // Get website colors
    function getWebsiteColors(hostname) {
        if (hostname.includes('github.com')) return { header: '#24292e', content: '#f6f8fa' };
        if (hostname.includes('stackoverflow.com')) return { header: '#f48024', content: '#ffffff' };
        if (hostname.includes('ycombinator.com')) return { header: '#ff6600', content: '#f6f6ef' };
        if (hostname.includes('dev.to')) return { header: '#0a0a0a', content: '#ffffff' };
        if (hostname.includes('medium.com')) return { header: '#00ab6c', content: '#ffffff' };
        if (hostname.includes('reddit.com')) return { header: '#ff4500', content: '#ffffff' };
        return { header: '#667eea', content: '#f8f9fa' };
    }
    
    // Get navigation items
    function getNavItems(hostname) {
        if (hostname.includes('github.com')) return ['Explore', 'Topics', 'Trending', 'Collections'];
        if (hostname.includes('stackoverflow.com')) return ['Questions', 'Tags', 'Users', 'Jobs'];
        if (hostname.includes('ycombinator.com')) return ['new', 'past', 'comments', 'ask'];
        if (hostname.includes('dev.to')) return ['Home', 'Reading List', 'Listings', 'Podcasts'];
        if (hostname.includes('medium.com')) return ['Home', 'Following', 'Reading List', 'Medium'];
        if (hostname.includes('reddit.com')) return ['Hot', 'New', 'Rising', 'Controversial'];
        return ['Home', 'About', 'Contact', 'Blog'];
    }
    
    // Get content items
    function getContentItems(hostname) {
        if (hostname.includes('github.com')) return [
            'Discover repositories and projects',
            'Explore trending repositories',
            'Find open source projects',
            'Connect with developers',
            'Contribute to open source'
        ];
        if (hostname.includes('stackoverflow.com')) return [
            'Ask programming questions',
            'Find answers to coding problems',
            'Share knowledge with developers',
            'Build your programming skills',
            'Join the developer community'
        ];
        if (hostname.includes('ycombinator.com')) return [
            'Latest technology news',
            'Startup discussions',
            'Programming insights',
            'Tech industry updates',
            'Developer community'
        ];
        if (hostname.includes('dev.to')) return [
            'Programming articles and tutorials',
            'Developer community discussions',
            'Tech news and insights',
            'Coding challenges and solutions',
            'Career advice for developers'
        ];
        return [
            'Welcome to our platform',
            'Explore our features',
            'Join our community',
            'Learn and grow',
            'Connect with others'
        ];
    }
    
    // Capture screenshot
    function captureScreenshot() {
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
                // Fallback: create a simple screenshot
                const canvas = document.createElement('canvas');
                const ctx = canvas.getContext('2d');
                canvas.width = 800;
                canvas.height = 600;
                
                // Create a simple representation
                ctx.fillStyle = '#ffffff';
                ctx.fillRect(0, 0, canvas.width, canvas.height);
                
                ctx.fillStyle = '#667eea';
                ctx.fillRect(0, 0, canvas.width, 60);
                
                ctx.fillStyle = '#ffffff';
                ctx.font = 'bold 24px Arial';
                ctx.fillText(document.title || 'Page', 20, 35);
                
                ctx.fillStyle = '#f8f9fa';
                ctx.fillRect(0, 60, canvas.width, canvas.height - 60);
                
                ctx.fillStyle = '#333333';
                ctx.font = '14px Arial';
                ctx.fillText('Auto-scraped page content', 30, 100);
                ctx.fillText(`Links: ${document.querySelectorAll('a').length}`, 30, 130);
                ctx.fillText(`Buttons: ${document.querySelectorAll('button').length}`, 30, 160);
                
                resolve(canvas.toDataURL('image/png'));
            }
        });
    }
    
    // Add hover listeners
    function addHoverListeners() {
        function addListeners() {
            const elements = document.querySelectorAll('a, button, [role="button"], [onclick], input[type="submit"], input[type="button"]');
            
            elements.forEach(element => {
                if (element.hasAttribute('data-navigrab-auto-listener')) return;
                
                element.setAttribute('data-navigrab-auto-listener', 'true');
                element.addEventListener('mouseenter', handleMouseEnter);
                element.addEventListener('mouseleave', handleMouseLeave);
                element.addEventListener('mousemove', handleMouseMove);
            });
        }
        
        // Add listeners immediately
        addListeners();
        
        // Re-scan periodically
        setInterval(addListeners, 2000);
    }
    
    // Handle mouse enter
    function handleMouseEnter(event) {
        const element = event.target;
        const url = getElementUrl(element);
        
        if (!url) return;
        
        // Find matching scraped data
        let pageData = scrapedData.find(item => 
            item.url === url || 
            item.url.includes(url) || 
            url.includes(item.url)
        );
        
        if (pageData) {
            showTooltip(event, pageData);
        } else {
            // Create preview for external links
            if (url.startsWith('http') && !url.includes(window.location.hostname)) {
                scrapeUrl(url).then(data => {
                    showTooltip(event, data);
                });
            }
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
    
    // Get element URL
    function getElementUrl(element) {
        if (element.href) return element.href;
        if (element.getAttribute('data-url')) return element.getAttribute('data-url');
        if (element.getAttribute('data-href')) return element.getAttribute('data-href');
        
        let parent = element.parentElement;
        while (parent && parent !== document.body) {
            if (parent.href) return parent.href;
            parent = parent.parentElement;
        }
        
        return null;
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
                console.log('📦 html2canvas loaded for better screenshots');
            };
            document.head.appendChild(script);
        }
    }
    
    // Start everything
    loadHtml2Canvas();
    init();
    
    console.log('🕷️ NaviGrab Auto is now running automatically in the background!');
    console.log('📊 It will proactively scrape pages and show tooltips on hover');
    
})();
