#include <cassert>
#include <atomic>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "sharded_lru/cache_config.hpp"
#include "sharded_lru/sharded_lru_cache.hpp"

void test_basic_put_get() {
  sharded_lru::CacheConfig config{};
  config.capacity = 8;
  config.shards = 2;

  sharded_lru::ShardedLruCache<std::string, int> cache(config);
  const bool inserted = cache.put("a", 1);
  const auto value = cache.get("a");

  assert(inserted);
  assert(value.has_value());
  assert(*value == 1);
}

void test_strict_lru_ordering_single_shard() {
  sharded_lru::CacheConfig config{};
  config.capacity = 3;
  config.shards = 1;

  sharded_lru::ShardedLruCache<std::string, int> cache(config);
  cache.put("a", 1);
  cache.put("b", 2);
  cache.put("c", 3);

  const auto a_hit = cache.get("a");
  assert(a_hit.has_value() && *a_hit == 1);

  cache.put("d", 4);

  assert(cache.contains("a"));
  assert(!cache.contains("b"));
  assert(cache.contains("c"));
  assert(cache.contains("d"));
  assert(cache.size() == 3);
}

void test_overwrite_touches_recency() {
  sharded_lru::CacheConfig config{};
  config.capacity = 2;
  config.shards = 1;

  sharded_lru::ShardedLruCache<std::string, int> cache(config);
  assert(cache.put("a", 1));
  assert(cache.put("b", 2));

  const bool inserted = cache.put("a", 10);
  assert(!inserted);

  cache.put("c", 3);

  assert(cache.contains("a"));
  assert(!cache.contains("b"));
  const auto a_value = cache.get("a");
  assert(a_value.has_value() && *a_value == 10);
}

void test_erase_clear_and_size() {
  sharded_lru::CacheConfig config{};
  config.capacity = 5;
  config.shards = 1;

  sharded_lru::ShardedLruCache<std::string, int> cache(config);
  cache.put("a", 1);
  cache.put("b", 2);
  cache.put("c", 3);

  assert(cache.size() == 3);
  assert(cache.erase("b"));
  assert(!cache.erase("b"));
  assert(!cache.contains("b"));
  assert(cache.size() == 2);

  cache.clear();
  assert(cache.size() == 0);
  assert(!cache.contains("a"));
  assert(!cache.contains("c"));
}

void test_invalid_zero_shards_throws() {
  sharded_lru::CacheConfig config{};
  config.capacity = 10;
  config.shards = 0;

  bool threw = false;
  try {
    sharded_lru::ShardedLruCache<std::string, int> cache(config);
    (void)cache;
  } catch (const std::invalid_argument&) {
    threw = true;
  }

  assert(threw);
}

void test_concurrent_strict_ordering_sequence() {
  sharded_lru::CacheConfig config{};
  config.capacity = 2;
  config.shards = 1;

  sharded_lru::ShardedLruCache<std::string, int> cache(config);
  cache.put("a", 1);
  cache.put("b", 2);

  std::atomic<int> turn{0};

  std::thread first([&]() {
    while (turn.load(std::memory_order_acquire) != 1) {
      std::this_thread::yield();
    }
    const auto value = cache.get("a");
    assert(value.has_value() && *value == 1);
    turn.store(2, std::memory_order_release);
  });

  std::thread second([&]() {
    while (turn.load(std::memory_order_acquire) != 2) {
      std::this_thread::yield();
    }
    const auto value = cache.get("b");
    assert(value.has_value() && *value == 2);
    turn.store(3, std::memory_order_release);
  });

  turn.store(1, std::memory_order_release);
  while (turn.load(std::memory_order_acquire) != 3) {
    std::this_thread::yield();
  }

  first.join();
  second.join();

  cache.put("c", 3);

  assert(!cache.contains("a"));
  assert(cache.contains("b"));
  assert(cache.contains("c"));
}

void test_concurrent_write_read_stress() {
  sharded_lru::CacheConfig config{};
  constexpr std::size_t thread_count = 8;
  constexpr std::size_t keys_per_thread = 300;
  config.capacity = thread_count * keys_per_thread;
  config.shards = 1;

  sharded_lru::ShardedLruCache<std::string, int> cache(config);

  std::vector<std::thread> writers;
  writers.reserve(thread_count);

  for (std::size_t t = 0; t < thread_count; ++t) {
    writers.emplace_back([t, &cache]() {
      for (std::size_t i = 0; i < keys_per_thread; ++i) {
        const std::string key = "t" + std::to_string(t) + ":" + std::to_string(i);
        cache.put(key, static_cast<int>(i));
        const auto value = cache.get(key);
        assert(value.has_value());
      }
    });
  }

  for (auto& writer : writers) {
    writer.join();
  }

  assert(cache.size() <= cache.capacity());
  assert(cache.size() == thread_count * keys_per_thread);

  for (std::size_t t = 0; t < thread_count; ++t) {
    for (std::size_t i = 0; i < keys_per_thread; ++i) {
      const std::string key = "t" + std::to_string(t) + ":" + std::to_string(i);
      const auto value = cache.get(key);
      assert(value.has_value());
      assert(*value == static_cast<int>(i));
    }
  }
}

int main() {
  test_basic_put_get();
  test_strict_lru_ordering_single_shard();
  test_overwrite_touches_recency();
  test_erase_clear_and_size();
  test_invalid_zero_shards_throws();
  test_concurrent_strict_ordering_sequence();
  test_concurrent_write_read_stress();

  return 0;
}
