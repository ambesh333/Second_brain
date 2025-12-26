#include "include/disk_scheduler.h"
#include <vector>
#include "include/disk_manager.h"

namespace disk_scheduler {

DiskScheduler::DiskScheduler(DiskManager *disk_manager) : disk_manager_(disk_manager) {
  background_thread_.emplace([&] { StartWorkerThread(); });
}

DiskScheduler::~DiskScheduler() {
  request_queue_.Put(std::nullopt);
  if (background_thread_.has_value()) {
    background_thread_->join();
  }
}

void DiskScheduler::Schedule(std::vector<DiskRequest> &requests) {
  for (auto &req : requests) {
    request_queue_.Put(std::move(req));
  }
}

void DiskScheduler::StartWorkerThread() {
  while (true) {
    auto maybe_request = request_queue_.Get();
    if (!maybe_request.has_value()) {
      return;
    }

    DiskRequest &request = maybe_request.value();

    if (request.is_write_) {
      disk_manager_->WritePage(request.page_id_, request.data_);
    } else {
      disk_manager_->ReadPage(request.page_id_, request.data_);
    }
    request.callback_.set_value(true);
  }
}

}  // namespace disk_scheduler