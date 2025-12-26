#pragma once

#include <atomic>
#include <filesystem>
#include <fstream>
#include <future>  // NOLINT
#include <mutex>   // NOLINT
#include <string>
#include <unordered_map>
#include <vector>


namespace disk_scheduler {

class DiskManager {
 public:
    explicit DiskManager(const std::filesystem::path &db_file);

    DiskManager() = default;

    virtual ~DiskManager() = default;

    void ShutDown();

    virtual void WritePage(page_id_t page_id, const char *page_data);

    virtual void ReadPage(page_id_t page_id, char *page_data);

    virtual void DeletePage(page_id_t page_id);

    void WriteLog(char *log_data, int size);

    auto ReadLog(char *log_data, int size, int offset) -> bool;

    auto GetNumFlushes() const -> int;

    auto GetFlushState() const -> bool;

    auto GetNumWrites() const -> int;

    auto GetNumDeletes() const -> int;

    inline void SetFlushLogFuture(std::future<void> *f) { flush_log_f_ = f; }

    inline auto HasFlushLogFuture() -> bool { return flush_log_f_ != nullptr; }
    inline auto GetLogFileName() const -> std::filesystem::path { return log_file_name_; }

    auto GetDbFileSize() -> size_t {
        auto file_size = GetFileSize(db_file_name_);
        if (file_size < 0) {
        LOG_DEBUG("I/O error: Fail to get db file size");
        return -1;
        }
        return static_cast<size_t>(file_size);
    }

 protected:
  int num_flushes_{0};
  int num_writes_{0};
  int num_deletes_{0};

  size_t page_capacity_{DEFAULT_DB_IO_SIZE};

 private:
  auto GetFileSize(const std::string &file_name) -> int;

  auto AllocatePage() -> size_t;

  std::fstream log_io_;
  std::filesystem::path log_file_name_;
  std::fstream db_io_;
  std::filesystem::path db_file_name_;

  std::unordered_map<page_id_t, size_t> pages_;
  std::vector<size_t> free_slots_;

  bool flush_log_{false};
  std::future<void> *flush_log_f_{nullptr};
  std::mutex db_io_latch_;
};

}  // namespace disk_scheduler