#include <chrono>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "sharded_lru/cache_config.hpp"
#include "sharded_lru/sharded_lru_cache.hpp"

int main() {
  constexpr std::size_t threads = 8;
  constexpr std::size_t ops_per_thread = 10000;
  constexpr std::size_t total_ops = threads * ops_per_thread;
  const std::vector<std::size_t> shard_counts = {1, 4, 16, 64};

  std::cout << "benchmark=put_throughput threads=" << threads
            << " ops_per_thread=" << ops_per_thread << '\n';

  for (const std::size_t shard_count : shard_counts) {
    sharded_lru::CacheConfig config{};
    config.capacity = 200000;
    config.shards = shard_count;

    sharded_lru::ShardedLruCache<std::string, int> cache(config);
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
    const auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    const double ops_per_second = static_cast<double>(total_ops) / elapsed;

    std::cout << "shards=" << shard_count << " elapsed_ms=" << std::fixed << std::setprecision(2)
              << (elapsed * 1000.0) << " ops_per_sec=" << static_cast<std::size_t>(ops_per_second)
              << " final_size=" << cache.size() << '\n';
  }

  return 0;
}
