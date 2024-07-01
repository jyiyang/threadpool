#include "thread_pool.h"

std::function<void(void)> ThreadPool::Queue::Pop() {
  std::scoped_lock<std::mutex> lock(m_);
  // Return an empty job if the job queue is empty.
  if (jobs_.empty()) {
    return std::function<void(void)>([]() { return; });
  }

  auto job = jobs_.front();
  jobs_.pop();
  return job;
}

ThreadPool::ThreadPool(int num_threads)
    : is_alive_(true), workers_(num_threads) {
  Init(num_threads);
}

ThreadPool::~ThreadPool() {
  {
    std::scoped_lock<std::mutex> lock(job_mutex_);
    is_alive_ = false;
  }

  cv_.notify_all();
  for (auto& worker : workers_) {
    if (worker.joinable()) {
      worker.join();
    }
  }
}

void ThreadPool::Init(int num_threads) {
  for (int i = 0; i < num_threads; ++i) {
    workers_.emplace_back([this]() {
      while (is_alive_) {
        {
          std::unique_lock<std::mutex> lock(job_mutex_);
          cv_.wait(lock, [this]() { return !is_alive_ || !jobs_.empty(); });
        }

        auto job = jobs_.Pop();
        job();
      }
    });
  }
}
