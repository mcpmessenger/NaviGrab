#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <chrono>
#include <functional>
#include <optional>
#include "proactive_scraping.h"
#include "screenshot_capture.h"

namespace chromium_playwright {

// Forward declarations
class StorageManager;
class SQLiteStorage;
class IndexedDBStorage;

// Storage types
enum class StorageType {
    SQLITE,
    INDEXEDDB,
    MEMORY,
    FILE_SYSTEM
};

// Storage configuration
struct StorageConfig {
    StorageType type = StorageType::SQLITE;
    std::string connection_string = "";
    std::string database_path = "./chromium_playwright.db";
    size_t max_connections = 10;
    size_t cache_size = 1000;
    std::chrono::milliseconds connection_timeout{30000};
    bool enable_compression = true;
    bool enable_encryption = false;
    std::string encryption_key = "";
    std::map<std::string, std::string> custom_settings;
};

// Storage key for data retrieval
struct StorageKey {
    int session_id = 0;
    std::string url = "";
    std::chrono::system_clock::time_point timestamp;
    std::string element_selector = "";
    std::string data_type = ""; // "screenshot", "scraped_data", "metadata"
    
    bool operator<(const StorageKey& other) const {
        if (session_id != other.session_id) return session_id < other.session_id;
        if (url != other.url) return url < other.url;
        if (timestamp != other.timestamp) return timestamp < other.timestamp;
        if (element_selector != other.element_selector) return element_selector < other.element_selector;
        return data_type < other.data_type;
    }
};

// Storage metadata
struct StorageMetadata {
    std::string id;
    StorageKey key;
    size_t size_bytes = 0;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point updated_at;
    std::string content_type = "";
    std::string checksum = "";
    std::map<std::string, std::string> custom_attributes;
    bool is_compressed = false;
    bool is_encrypted = false;
};

// Storage query options
struct StorageQueryOptions {
    std::vector<int> session_ids;
    std::vector<std::string> urls;
    std::vector<std::string> data_types;
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
    std::map<std::string, std::string> metadata_filters;
    int limit = 1000;
    int offset = 0;
    std::string order_by = "timestamp";
    bool ascending = true;
};

// Storage statistics
struct StorageStatistics {
    size_t total_items = 0;
    size_t total_size_bytes = 0;
    size_t screenshots_count = 0;
    size_t scraped_data_count = 0;
    size_t metadata_count = 0;
    std::chrono::system_clock::time_point oldest_item;
    std::chrono::system_clock::time_point newest_item;
    std::map<std::string, size_t> data_type_counts;
    std::map<int, size_t> session_counts;
};

// Storage Manager Interface
class StorageManager {
public:
    virtual ~StorageManager() = default;

    // Initialization
    virtual bool Initialize(const StorageConfig& config) = 0;
    virtual bool IsInitialized() const = 0;
    virtual void Shutdown() = 0;

    // Scraped data operations
    virtual std::string StoreScrapedPageData(const ScrapedPageData& data) = 0;
    virtual std::optional<ScrapedPageData> RetrieveScrapedPageData(const std::string& id) = 0;
    virtual bool UpdateScrapedPageData(const std::string& id, const ScrapedPageData& data) = 0;
    virtual bool DeleteScrapedPageData(const std::string& id) = 0;
    virtual bool ScrapedPageDataExists(const std::string& id) const = 0;

    // Screenshot operations
    virtual std::string StoreScreenshot(int session_id, const std::string& url, 
                                      const std::vector<uint8_t>& image_data, 
                                      const std::string& format,
                                      const std::map<std::string, std::string>& metadata = {}) = 0;
    virtual std::optional<std::vector<uint8_t>> RetrieveScreenshot(const std::string& id) = 0;
    virtual std::optional<ScreenshotMetadata> RetrieveScreenshotMetadata(const std::string& id) = 0;
    virtual bool DeleteScreenshot(const std::string& id) = 0;
    virtual bool ScreenshotExists(const std::string& id) const = 0;

    // Generic data operations
    virtual std::string StoreData(const StorageKey& key, const std::vector<uint8_t>& data, 
                                const std::map<std::string, std::string>& metadata = {}) = 0;
    virtual std::optional<std::vector<uint8_t>> RetrieveData(const std::string& id) = 0;
    virtual std::optional<StorageMetadata> RetrieveMetadata(const std::string& id) = 0;
    virtual bool UpdateData(const std::string& id, const std::vector<uint8_t>& data) = 0;
    virtual bool DeleteData(const std::string& id) = 0;
    virtual bool DataExists(const std::string& id) const = 0;

    // Batch operations
    virtual std::vector<std::string> StoreScrapedPageDataBatch(const std::vector<ScrapedPageData>& data_list) = 0;
    virtual std::vector<ScrapedPageData> RetrieveScrapedPageDataBatch(const std::vector<std::string>& ids) = 0;
    virtual bool DeleteScrapedPageDataBatch(const std::vector<std::string>& ids) = 0;

    virtual std::vector<std::string> StoreScreenshotBatch(int session_id, 
                                                        const std::vector<std::pair<std::string, std::vector<uint8_t>>>& screenshots,
                                                        const std::string& format) = 0;
    virtual std::vector<std::vector<uint8_t>> RetrieveScreenshotBatch(const std::vector<std::string>& ids) = 0;
    virtual bool DeleteScreenshotBatch(const std::vector<std::string>& ids) = 0;

    // Query operations
    virtual std::vector<std::string> FindScrapedPageData(int session_id) = 0;
    virtual std::vector<std::string> FindScrapedPageData(const StorageQueryOptions& options) = 0;
    virtual std::vector<std::string> FindScreenshots(int session_id) = 0;
    virtual std::vector<std::string> FindScreenshots(const StorageQueryOptions& options) = 0;
    virtual std::vector<std::string> FindDataByUrl(const std::string& url) = 0;
    virtual std::vector<std::string> FindDataBySession(int session_id) = 0;
    virtual std::vector<std::string> FindDataByTimeRange(
        const std::chrono::system_clock::time_point& start,
        const std::chrono::system_clock::time_point& end) = 0;

    // Session management
    virtual bool ClearSessionData(int session_id) = 0;
    virtual bool ClearAllData() = 0;
    virtual std::vector<int> GetActiveSessions() = 0;
    virtual bool SessionExists(int session_id) const = 0;

    // Statistics and monitoring
    virtual StorageStatistics GetStatistics() const = 0;
    virtual StorageStatistics GetSessionStatistics(int session_id) const = 0;
    virtual size_t GetStorageSize() const = 0;
    virtual size_t GetItemCount() const = 0;
    virtual std::vector<std::string> GetStorageInfo() const = 0;

    // Maintenance operations
    virtual bool OptimizeStorage() = 0;
    virtual bool CleanupOldData(const std::chrono::system_clock::time_point& cutoff) = 0;
    virtual bool VacuumDatabase() = 0;
    virtual bool RebuildIndexes() = 0;

    // Backup and restore
    virtual bool CreateBackup(const std::string& backup_path) = 0;
    virtual bool RestoreFromBackup(const std::string& backup_path) = 0;
    virtual std::vector<std::string> ListBackups() const = 0;
    virtual bool DeleteBackup(const std::string& backup_path) = 0;

    // Configuration
    virtual StorageConfig GetConfig() const = 0;
    virtual bool UpdateConfig(const StorageConfig& config) = 0;
    virtual void SetCompressionEnabled(bool enabled) = 0;
    virtual void SetEncryptionEnabled(bool enabled, const std::string& key = "") = 0;
};

// SQLite Storage Implementation
class SQLiteStorage : public StorageManager {
public:
    virtual ~SQLiteStorage() = default;

    // SQLite-specific operations
    virtual bool CreateTables() = 0;
    virtual bool DropTables() = 0;
    virtual bool TableExists(const std::string& table_name) const = 0;
    virtual std::vector<std::string> GetTableNames() const = 0;
    virtual std::string GetTableSchema(const std::string& table_name) const = 0;

    // Index management
    virtual bool CreateIndex(const std::string& index_name, const std::string& table_name, 
                           const std::vector<std::string>& columns) = 0;
    virtual bool DropIndex(const std::string& index_name) = 0;
    virtual std::vector<std::string> GetIndexNames() const = 0;

    // Transaction management
    virtual bool BeginTransaction() = 0;
    virtual bool CommitTransaction() = 0;
    virtual bool RollbackTransaction() = 0;
    virtual bool IsInTransaction() const = 0;

    // Custom queries
    virtual std::vector<std::map<std::string, std::string>> ExecuteQuery(const std::string& sql) = 0;
    virtual bool ExecuteCommand(const std::string& sql) = 0;
    virtual std::string GetLastError() const = 0;
};

// IndexedDB Storage Implementation
class IndexedDBStorage : public StorageManager {
public:
    virtual ~IndexedDBStorage() = default;

    // IndexedDB-specific operations
    virtual bool CreateObjectStore(const std::string& store_name) = 0;
    virtual bool DeleteObjectStore(const std::string& store_name) = 0;
    virtual std::vector<std::string> GetObjectStoreNames() const = 0;

    // Index management
    virtual bool CreateIndex(const std::string& store_name, const std::string& index_name, 
                           const std::string& key_path, bool unique = false) = 0;
    virtual bool DeleteIndex(const std::string& store_name, const std::string& index_name) = 0;
    virtual std::vector<std::string> GetIndexNames(const std::string& store_name) const = 0;

    // Cursor operations
    virtual std::vector<std::map<std::string, std::string>> GetCursorData(const std::string& store_name, 
                                                                         const std::string& index_name = "",
                                                                         const std::string& key_range = "") = 0;
};

// Storage Factory
class StorageFactory {
public:
    static std::unique_ptr<StorageManager> CreateStorage(StorageType type);
    static std::unique_ptr<StorageManager> CreateStorage(const StorageConfig& config);
    static std::unique_ptr<SQLiteStorage> CreateSQLiteStorage(const StorageConfig& config);
    static std::unique_ptr<IndexedDBStorage> CreateIndexedDBStorage(const StorageConfig& config);
};

// Utility functions
namespace storage_utils {
    // Configuration helpers
    StorageConfig CreateDefaultConfig(StorageType type = StorageType::SQLITE);
    bool ValidateConfig(const StorageConfig& config);
    std::vector<std::string> ValidateConfigErrors(const StorageConfig& config);
    
    // Key generation
    std::string GenerateStorageId();
    StorageKey CreateStorageKey(int session_id, const std::string& url, 
                              const std::chrono::system_clock::time_point& timestamp);
    
    // Data serialization
    std::vector<uint8_t> SerializeScrapedPageData(const ScrapedPageData& data);
    ScrapedPageData DeserializeScrapedPageData(const std::vector<uint8_t>& data);
    std::vector<uint8_t> SerializeScreenshotMetadata(const ScreenshotMetadata& metadata);
    ScreenshotMetadata DeserializeScreenshotMetadata(const std::vector<uint8_t>& data);
    
    // Compression
    std::vector<uint8_t> CompressData(const std::vector<uint8_t>& data);
    std::vector<uint8_t> DecompressData(const std::vector<uint8_t>& compressed_data);
    
    // Encryption
    std::vector<uint8_t> EncryptData(const std::vector<uint8_t>& data, const std::string& key);
    std::vector<uint8_t> DecryptData(const std::vector<uint8_t>& encrypted_data, const std::string& key);
    
    // Checksum
    std::string ComputeChecksum(const std::vector<uint8_t>& data);
    bool VerifyChecksum(const std::vector<uint8_t>& data, const std::string& expected_checksum);
    
    // File operations
    bool SaveToFile(const std::vector<uint8_t>& data, const std::string& file_path);
    std::vector<uint8_t> LoadFromFile(const std::string& file_path);
    bool FileExists(const std::string& file_path);
    size_t GetFileSize(const std::string& file_path);
}

} // namespace chromium_playwright
