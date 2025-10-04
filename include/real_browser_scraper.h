#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration
typedef void RealBrowserScraper;

// C API for real browser scraping
RealBrowserScraper* create_real_scraper();
void destroy_real_scraper(RealBrowserScraper* scraper);
int start_real_scraping(RealBrowserScraper* scraper, const char* url, int max_depth, int max_pages);
void stop_real_scraping(RealBrowserScraper* scraper, int session_id);
int is_real_scraping_running(RealBrowserScraper* scraper, int session_id);
int get_real_pages_scraped(RealBrowserScraper* scraper, int session_id);

#ifdef __cplusplus
}
#endif


