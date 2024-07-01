#pragma once

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
 public:
  class Queue {
   public:
    auto size() {
      std::scoped_lock<std::mutex> lock(m_);
      return jobs_.size();
    }

    bool empty() {
      std::scoped_lock<std::mutex> lock(m_);
      return jobs_.empty();
    }

    template <typename Fn, typename... Args>
    auto Push(Fn&& fn, Args&&... args)
        -> std::future<std::invoke_result_t<Fn, Args...>> {
      using Result = std::invoke_result_t<Fn, Args...>;
      auto task_ptr = std::make_shared<std::packaged_task<Result()>>(
          [fn = std::forward<Fn>(fn),
           ... args = std::forward<Args>(args)]() mutable {
            return fn(std::move(args)...);
          });

      std::future<Result> future = task_ptr->get_future();
      jobs_.emplace(
          [task_ptr = std::move(task_ptr)]() mutable { (*task_ptr)(); });
      return future;
    }

    std::function<void(void)> Pop();

   private:
    std::queue<std::function<void(void)>> jobs_;
    mutable std::mutex m_;
  };

  ThreadPool(int num_threads);
  ~ThreadPool();
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;
  ThreadPool(ThreadPool&&) = delete;
  ThreadPool& operator=(ThreadPool&&) = delete;

  template <typename Fn, typename... Args>
  auto Run(Fn&& fn, Args&&... args) {
    std::unique_lock<std::mutex> lock(job_mutex_);
    auto future = jobs_.Push(std::forward<Fn>(fn), std::forward<Args>(args)...);
    cv_.notify_one();
    return future;
  }

 private:
  void Init(int num_threads);

  bool is_alive_;

  Queue jobs_;

  mutable std::mutex job_mutex_;

  std::vector<std::thread> workers_;

  std::condition_variable cv_;
};
