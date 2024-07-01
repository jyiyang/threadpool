#include <chrono>
#include <iostream>

#include "thread_pool.h"

std::mutex m;

int main() {
  ThreadPool pool(5);
  std::vector<std::future<int>> results;

  for (int i = 0; i < 50; ++i) {
    auto future = pool.Run([=]() {
      int wait_time_ms = (i % 5) * 1000;
      std::this_thread::sleep_for(std::chrono::milliseconds(wait_time_ms));
      {
        std::scoped_lock<std::mutex> lock(m);
        std::cout << "Current task is: " << i << std::endl;
      }
      return i;
    });

    results.emplace_back(std::move(future));
  }

  for (auto& future : results) {
    int res = future.get();
    {
      std::scoped_lock<std::mutex> lock(m);
      std::cout << "task return: " << res << std::endl;
    }
  }
  return 0;
}
