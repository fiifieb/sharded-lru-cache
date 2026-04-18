#include <chrono>
#include <cstddef>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "sharded_lru/cache_config.hpp"
#include "sharded_lru/sharded_lru_cache.hpp"

int main() {
  sharded_lru::CacheConfig config{};
  config.capacity = 100000;
  config.shards = 64;

  sharded_lru::ShardedLruCache<std::string, int> cache(config);

  constexpr std::size_t threads = 8;
  constexpr std::size_t ops_per_thread = 10000;

  std::vector<std::thread> workers;
  workers.reserve(threads);

  const auto start = std::chrono::steady_clock::now();
  for (std::size_t t = 0; t < threads; ++t) {
    workers.emplace_back([t, &cache]() {
      for (std::size_t i = 0; i < ops_per_thread; ++i) {
        cache.put("key:" + std::to_string(t) + ":" + std::to_string(i), static_cast<int>(i));
      }
    });
  }

  for (auto& worker : workers) {
    worker.join();
  }

  const auto end = std::chrono::steady_clock::now();
  const auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

  std::cout << "elapsed_ms=" << elapsed_ms << '\n';
  std::cout << "final_size=" << cache.size() << '\n';

  return 0;
}
