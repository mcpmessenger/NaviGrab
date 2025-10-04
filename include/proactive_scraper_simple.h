#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration
typedef void SimpleProactiveScraper;

// C API for proactive web scraping
SimpleProactiveScraper* create_scraper();
void destroy_scraper(SimpleProactiveScraper* scraper);
int start_scraping(SimpleProactiveScraper* scraper, const char* url, int max_depth, int max_pages);
void stop_scraping(SimpleProactiveScraper* scraper, int session_id);
int is_scraping_running(SimpleProactiveScraper* scraper, int session_id);
int get_pages_scraped(SimpleProactiveScraper* scraper, int session_id);

#ifdef __cplusplus
}
#endif


