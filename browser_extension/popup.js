// NaviGrab Universal Tooltip System - Popup Script

document.addEventListener('DOMContentLoaded', function() {
    loadStats();
    
    // Event listeners
    document.getElementById('scrapeBtn').addEventListener('click', startScraping);
    document.getElementById('refreshBtn').addEventListener('click', refreshData);
    document.getElementById('testBtn').addEventListener('click', testTooltips);
    document.getElementById('clearBtn').addEventListener('click', clearData);
});

function loadStats() {
    chrome.storage.local.get(['navigrab_screenshots'], function(result) {
        const scrapedData = result.navigrab_screenshots || [];
        
        // Update stats
        document.getElementById('pagesCount').textContent = scrapedData.length;
        
        const totalLinks = scrapedData.reduce((sum, page) => sum + (page.linksFound || 0), 0);
        document.getElementById('linksCount').textContent = totalLinks;
        
        if (scrapedData.length > 0) {
            const lastPage = scrapedData[scrapedData.length - 1];
            const lastUpdated = new Date(lastPage.timestamp).toLocaleString();
            document.getElementById('lastUpdated').textContent = lastUpdated;
        } else {
            document.getElementById('lastUpdated').textContent = 'Never';
        }
    });
}

function startScraping() {
    showStatus('Starting proactive scraping...', 'info');
    
    // Send message to background script
    chrome.runtime.sendMessage({action: 'startScraping'}, function(response) {
        if (response && response.success) {
            showStatus('Scraping started! Check console for progress.', 'success');
            setTimeout(loadStats, 2000);
        } else {
            showStatus('Failed to start scraping', 'error');
        }
    });
}

function refreshData() {
    showStatus('Refreshing data...', 'info');
    
    // Send message to content script
    chrome.tabs.query({active: true, currentWindow: true}, function(tabs) {
        chrome.tabs.sendMessage(tabs[0].id, {action: 'refreshData'}, function(response) {
            if (response && response.success) {
                showStatus('Data refreshed!', 'success');
                loadStats();
            } else {
                showStatus('Failed to refresh data', 'error');
            }
        });
    });
}

function testTooltips() {
    showStatus('Testing tooltips...', 'info');
    
    // Send message to content script
    chrome.tabs.query({active: true, currentWindow: true}, function(tabs) {
        chrome.tabs.sendMessage(tabs[0].id, {action: 'testTooltips'}, function(response) {
            if (response && response.success) {
                showStatus('Tooltips are working! Hover over links.', 'success');
            } else {
                showStatus('Tooltips test failed', 'error');
            }
        });
    });
}

function clearData() {
    if (confirm('Are you sure you want to clear all scraped data?')) {
        chrome.storage.local.set({navigrab_screenshots: []}, function() {
            showStatus('All data cleared', 'success');
            loadStats();
        });
    }
}

function showStatus(message, type) {
    const status = document.getElementById('status');
    status.textContent = message;
    status.className = `status ${type}`;
    status.style.display = 'block';
    
    setTimeout(() => {
        status.style.display = 'none';
    }, 3000);
}


