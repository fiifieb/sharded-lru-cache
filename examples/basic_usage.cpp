#include <iostream>
#include <string>

#include "sharded_lru/cache_config.hpp"
#include "sharded_lru/sharded_lru_cache.hpp"

int main() {
  sharded_lru::CacheConfig config{};
  config.capacity = 1024;
  config.shards = 16;

  sharded_lru::ShardedLruCache<std::string, std::string> cache(config);

  (void)cache.put("user:42", "Alice");
  const auto value = cache.get("user:42");

  if (value.has_value()) {
    std::cout << *value << '\n';
  }

  return 0;
}
