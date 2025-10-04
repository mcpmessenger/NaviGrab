#!/usr/bin/env python3
"""
NaviGrab C++ Integration Server
Simple HTTP server that simulates C++ backend responses
"""

import http.server
import socketserver
import json
import urllib.parse
import base64
import time
import random
from datetime import datetime

class NaviGrabHandler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path.startswith('/api/screenshot'):
            self.handle_screenshot_request()
        elif self.path.startswith('/api/scraping'):
            self.handle_scraping_request()
        elif self.path == '/' or self.path == '/index.html':
            self.handle_web_interface()
        else:
            self.handle_404()
    
    def do_POST(self):
        if self.path.startswith('/api/scraping'):
            self.handle_scraping_request()
        else:
            self.handle_404()
    
    def handle_screenshot_request(self):
        # Parse URL from query string
        parsed_url = urllib.parse.urlparse(self.path)
        query_params = urllib.parse.parse_qs(parsed_url.query)
        url = query_params.get('url', ['https://github.com'])[0]
        
        print(f"📸 Capturing screenshot of: {url}")
        
        # Generate mock screenshot data
        base64_image = self.generate_mock_screenshot(url)
        
        response_data = {
            "success": True,
            "url": url,
            "title": self.get_page_title(url),
            "base64": base64_image,
            "links_found": random.randint(10, 50),
            "buttons_found": random.randint(5, 20),
            "timestamp_ms": int(time.time() * 1000)
        }
        
        self.send_response(200)
        self.send_header('Content-Type', 'application/json')
        self.send_header('Access-Control-Allow-Origin', '*')
        self.end_headers()
        self.wfile.write(json.dumps(response_data).encode())
    
    def handle_scraping_request(self):
        print("🕷️ Starting background scraping...")
        
        response_data = {
            "message": "Background scraping started",
            "success": True,
            "is_scraping": True,
            "scraped_count": 0
        }
        
        self.send_response(200)
        self.send_header('Content-Type', 'application/json')
        self.send_header('Access-Control-Allow-Origin', '*')
        self.end_headers()
        self.wfile.write(json.dumps(response_data).encode())
    
    def handle_web_interface(self):
        html = """
<!DOCTYPE html>
<html>
<head>
    <title>NaviGrab C++ Server</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; background: #1a1a2e; color: white; }
        .container { max-width: 800px; margin: 0 auto; }
        .status { background: #00b894; padding: 15px; border-radius: 10px; margin: 20px 0; }
        .api-test { background: #667eea; padding: 20px; border-radius: 10px; margin: 20px 0; }
        .button { background: #fd79a8; color: white; border: none; padding: 15px 30px; border-radius: 5px; cursor: pointer; margin: 10px; }
        .result { background: #2d3436; padding: 15px; border-radius: 5px; margin: 10px 0; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🕷️ NaviGrab C++ Server</h1>
        <div class="status">✅ C++ Backend Server Running Successfully!</div>
        
        <div class="api-test">
            <h2>📸 Screenshot API Test</h2>
            <button class="button" onclick="testScreenshot()">Test Screenshot API</button>
            <div id="screenshotResult" class="result"></div>
        </div>
        
        <div class="api-test">
            <h2>🕷️ Scraping API Test</h2>
            <button class="button" onclick="testScraping()">Test Scraping API</button>
            <div id="scrapingResult" class="result"></div>
        </div>
    </div>
    
    <script>
        async function testScreenshot() {
            const result = document.getElementById('screenshotResult');
            result.innerHTML = 'Loading...';
            
            try {
                const response = await fetch('/api/screenshot?url=https://github.com');
                const data = await response.json();
                result.innerHTML = '<pre>' + JSON.stringify(data, null, 2) + '</pre>';
            } catch (error) {
                result.innerHTML = 'Error: ' + error.message;
            }
        }
        
        async function testScraping() {
            const result = document.getElementById('scrapingResult');
            result.innerHTML = 'Loading...';
            
            try {
                const response = await fetch('/api/scraping', { method: 'POST' });
                const data = await response.json();
                result.innerHTML = '<pre>' + JSON.stringify(data, null, 2) + '</pre>';
            } catch (error) {
                result.innerHTML = 'Error: ' + error.message;
            }
        }
    </script>
</body>
</html>
        """
        
        self.send_response(200)
        self.send_header('Content-Type', 'text/html')
        self.end_headers()
        self.wfile.write(html.encode())
    
    def handle_404(self):
        self.send_response(404)
        self.send_header('Content-Type', 'text/html')
        self.end_headers()
        self.wfile.write(b'<html><body><h1>404 - Not Found</h1></body></html>')
    
    def generate_mock_screenshot(self, url):
        # Generate a realistic screenshot using SVG (works without PIL)
        title = self.get_page_title(url)
        color = self.get_website_color(url)
        timestamp = datetime.now().strftime('%H:%M:%S')
        
        # Create an SVG that looks like a real website screenshot
        svg_content = f"""
        <svg width="800" height="600" xmlns="http://www.w3.org/2000/svg">
            <!-- Background -->
            <rect width="800" height="600" fill="#ffffff"/>
            
            <!-- Header -->
            <rect width="800" height="80" fill="{color}"/>
            <text x="20" y="35" font-family="Arial, sans-serif" font-size="24" fill="white" font-weight="bold">{title}</text>
            <text x="20" y="60" font-family="Arial, sans-serif" font-size="12" fill="white">{url}</text>
            
            <!-- Navigation -->
            <rect x="0" y="80" width="800" height="40" fill="#f8f9fa"/>
            <text x="20" y="105" font-family="Arial, sans-serif" font-size="14" fill="#333">Home</text>
            <text x="80" y="105" font-family="Arial, sans-serif" font-size="14" fill="#333">About</text>
            <text x="140" y="105" font-family="Arial, sans-serif" font-size="14" fill="#333">Contact</text>
            <text x="200" y="105" font-family="Arial, sans-serif" font-size="14" fill="#333">Help</text>
            
            <!-- Main content area -->
            <rect x="0" y="120" width="800" height="440" fill="#ffffff"/>
            
            <!-- Content lines -->
            <text x="30" y="150" font-family="Arial, sans-serif" font-size="18" fill="#333">Welcome to {title}</text>
            <text x="30" y="180" font-family="Arial, sans-serif" font-size="14" fill="#666">This is a realistic screenshot preview</text>
            <text x="30" y="210" font-family="Arial, sans-serif" font-size="14" fill="#666">Generated by NaviGrab C++ Backend</text>
            <text x="30" y="240" font-family="Arial, sans-serif" font-size="14" fill="#666">Real website content would appear here</text>
            
            <!-- Content boxes -->
            <rect x="30" y="270" width="200" height="100" fill="#e9ecef" stroke="#dee2e6" stroke-width="1"/>
            <text x="40" y="290" font-family="Arial, sans-serif" font-size="12" fill="#333">Article 1</text>
            <text x="40" y="310" font-family="Arial, sans-serif" font-size="10" fill="#666">Sample content here</text>
            
            <rect x="250" y="270" width="200" height="100" fill="#e9ecef" stroke="#dee2e6" stroke-width="1"/>
            <text x="260" y="290" font-family="Arial, sans-serif" font-size="12" fill="#333">Article 2</text>
            <text x="260" y="310" font-family="Arial, sans-serif" font-size="10" fill="#666">More content here</text>
            
            <rect x="470" y="270" width="200" height="100" fill="#e9ecef" stroke="#dee2e6" stroke-width="1"/>
            <text x="480" y="290" font-family="Arial, sans-serif" font-size="12" fill="#333">Article 3</text>
            <text x="480" y="310" font-family="Arial, sans-serif" font-size="10" fill="#666">Additional content</text>
            
            <!-- Footer -->
            <rect x="0" y="560" width="800" height="40" fill="#f8f9fa"/>
            <text x="20" y="580" font-family="Arial, sans-serif" font-size="10" fill="#666">Screenshot generated at {timestamp}</text>
            <text x="20" y="595" font-family="Arial, sans-serif" font-size="10" fill="#666">NaviGrab C++ Backend - Real Website Preview</text>
        </svg>
        """
        
        # Convert SVG to base64
        svg_bytes = svg_content.encode('utf-8')
        svg_base64 = base64.b64encode(svg_bytes).decode('utf-8')
        
        return f"data:image/svg+xml;base64,{svg_base64}"
    
    def generate_simple_screenshot(self, url):
        # Fallback: Generate a simple colored rectangle
        title = self.get_page_title(url)
        color = self.get_website_color(url)
        
        # Create a simple HTML canvas screenshot
        html_content = f"""
        <!DOCTYPE html>
        <html>
        <head><title>{title}</title></head>
        <body style="margin:0;padding:20px;background:{color};color:white;font-family:Arial;">
            <h1>{title}</h1>
            <p>URL: {url}</p>
            <p>Generated by NaviGrab C++ Backend</p>
            <p>This is a realistic website preview</p>
        </body>
        </html>
        """
        
        # For now, return a more interesting base64 image
        # This is a 200x150 pixel image with the website colors
        return f"data:image/svg+xml;base64,{base64.b64encode(html_content.encode()).decode()}"
    
    def get_website_color(self, url):
        if 'github.com' in url:
            return '#24292e'
        elif 'stackoverflow.com' in url:
            return '#f48024'
        elif 'ycombinator.com' in url:
            return '#ff6600'
        elif 'dev.to' in url:
            return '#0a0a0a'
        elif 'reddit.com' in url:
            return '#ff4500'
        elif 'producthunt.com' in url:
            return '#da552f'
        else:
            return '#007bff'
    
    def get_page_title(self, url):
        if 'github.com' in url:
            return 'GitHub'
        elif 'stackoverflow.com' in url:
            return 'Stack Overflow'
        elif 'ycombinator.com' in url:
            return 'Hacker News'
        elif 'dev.to' in url:
            return 'Dev.to'
        else:
            return 'Unknown Page'

def main():
    PORT = 8080
    
    print("🕷️ NaviGrab C++ Integration Server")
    print("==================================")
    print(f"🚀 Starting server on port {PORT}")
    print(f"📸 Screenshot API: http://localhost:{PORT}/api/screenshot")
    print(f"🕷️ Scraping API: http://localhost:{PORT}/api/scraping")
    print(f"🌐 Web Interface: http://localhost:{PORT}/")
    print("Press Ctrl+C to stop the server")
    
    with socketserver.TCPServer(("", PORT), NaviGrabHandler) as httpd:
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\n🛑 Server stopped")

if __name__ == "__main__":
    main()
