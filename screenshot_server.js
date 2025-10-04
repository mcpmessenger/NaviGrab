const express = require('express');
const puppeteer = require('puppeteer');
const path = require('path');
const fs = require('fs');

const app = express();
const PORT = 8080;

// Middleware
app.use(express.json());
app.use(express.static('web_interface'));

// CORS middleware
app.use((req, res, next) => {
    res.header('Access-Control-Allow-Origin', '*');
    res.header('Access-Control-Allow-Methods', 'GET, POST, PUT, DELETE, OPTIONS');
    res.header('Access-Control-Allow-Headers', 'Origin, X-Requested-With, Content-Type, Accept, Authorization');
    
    if (req.method === 'OPTIONS') {
        res.sendStatus(200);
    } else {
        next();
    }
});

// Serve main interface
app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'web_interface', 'navigrab_server_based.html'));
});

// Health check
app.get('/api/status', (req, res) => {
    res.json({
        status: 'running',
        message: 'NaviGrab Screenshot Server is running',
        timestamp: new Date().toISOString()
    });
});

// Capture screenshot endpoint
app.post('/api/capture', async (req, res) => {
    const { url, options = {} } = req.body;
    
    if (!url) {
        return res.status(400).json({
            success: false,
            error: 'URL is required'
        });
    }

    let browser;
    try {
        console.log(`📸 Capturing screenshot of: ${url}`);
        
        browser = await puppeteer.launch({
            headless: true,
            args: [
                '--no-sandbox',
                '--disable-setuid-sandbox',
                '--disable-dev-shm-usage',
                '--disable-accelerated-2d-canvas',
                '--no-first-run',
                '--no-zygote',
                '--disable-gpu'
            ]
        });

        const page = await browser.newPage();
        
        // Set viewport
        await page.setViewport({
            width: options.width || 1200,
            height: options.height || 800
        });

        // Navigate to page
        await page.goto(url, {
            waitUntil: 'networkidle2',
            timeout: 30000
        });

        // Wait a bit for dynamic content
        await page.waitForTimeout(2000);

        // Capture screenshot
        const screenshot = await page.screenshot({
            type: 'jpeg',
            quality: 80,
            fullPage: options.fullPage || false
        });

        // Extract page information
        const pageInfo = await page.evaluate(() => {
            const links = Array.from(document.querySelectorAll('a[href]'))
                .slice(0, 50)
                .map(link => ({
                    text: link.textContent.trim().substring(0, 50),
                    href: link.href,
                    internal: link.href.includes(window.location.hostname)
                }));

            const buttons = document.querySelectorAll('button, [role="button"], input[type="submit"], input[type="button"]').length;
            const images = document.querySelectorAll('img').length;

            return {
                title: document.title || 'Untitled Page',
                links,
                buttons,
                images
            };
        });

        const base64Screenshot = screenshot.toString('base64');
        const dataUrl = `data:image/jpeg;base64,${base64Screenshot}`;

        res.json({
            success: true,
            screenshot: dataUrl,
            title: pageInfo.title,
            links: pageInfo.links,
            buttons: pageInfo.buttons,
            images: pageInfo.images,
            url: url,
            timestamp: new Date().toISOString()
        });

    } catch (error) {
        console.error('Screenshot capture error:', error);
        res.status(500).json({
            success: false,
            error: error.message
        });
    } finally {
        if (browser) {
            await browser.close();
        }
    }
});

// OpenAI analysis endpoint (placeholder)
app.post('/api/screenshot-analyze', async (req, res) => {
    const { url, analysis_type = 'general', custom_prompt = '', api_key } = req.body;
    
    if (!url) {
        return res.status(400).json({
            success: false,
            error: 'URL is required'
        });
    }

    try {
        console.log(`🤖 Analyzing screenshot with OpenAI: ${url}`);
        
        // First capture the screenshot
        const captureResponse = await fetch(`http://localhost:${PORT}/api/capture`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ url, options: { fullPage: true } })
        });

        if (!captureResponse.ok) {
            throw new Error('Failed to capture screenshot for analysis');
        }

        const captureResult = await captureResponse.json();
        
        if (!captureResult.success) {
            throw new Error('Screenshot capture failed');
        }

        // For now, return the captured screenshot with a mock analysis
        // In a real implementation, you would call the OpenAI API here
        const mockAnalysis = `This is a mock analysis of the webpage at ${url}.

Key Findings:
- Found ${captureResult.links.length} links on the page
- Detected ${captureResult.buttons} interactive buttons
- Identified ${captureResult.images} images
- Page title: ${captureResult.title}

Analysis Type: ${analysis_type}
Custom Prompt: ${custom_prompt || 'Default analysis'}

Note: This is a placeholder analysis. To enable real OpenAI analysis, configure your API key and implement the OpenAI integration.`;

        res.json({
            success: true,
            screenshot: captureResult.screenshot,
            title: captureResult.title,
            links: captureResult.links,
            buttons: captureResult.buttons,
            images: captureResult.images,
            analysis: mockAnalysis,
            url: url,
            timestamp: new Date().toISOString()
        });

    } catch (error) {
        console.error('Analysis error:', error);
        res.status(500).json({
            success: false,
            error_message: error.message
        });
    }
});

// Start server
app.listen(PORT, () => {
    console.log('🕷️ NaviGrab Screenshot Server');
    console.log('===============================');
    console.log(`🌐 Server running on http://localhost:${PORT}`);
    console.log('📋 Available endpoints:');
    console.log(`   GET  / - Main interface`);
    console.log(`   GET  /api/status - Server status`);
    console.log(`   POST /api/capture - Capture screenshot`);
    console.log(`   POST /api/screenshot-analyze - Analyze with OpenAI`);
    console.log();
    console.log('🎯 Open http://localhost:8080 in your browser');
    console.log('Press Ctrl+C to stop the server');
    console.log();
});

// Graceful shutdown
process.on('SIGINT', () => {
    console.log('\n🛑 Server shutting down...');
    process.exit(0);
});

process.on('SIGTERM', () => {
    console.log('\n🛑 Server shutting down...');
    process.exit(0);
});
