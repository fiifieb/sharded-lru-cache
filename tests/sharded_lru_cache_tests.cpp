#include <cassert>
#include <string>

#include "sharded_lru/cache_config.hpp"
#include "sharded_lru/sharded_lru_cache.hpp"

int main() {
  sharded_lru::CacheConfig config{};
  config.capacity = 8;
  config.shards = 2;

  sharded_lru::ShardedLruCache<std::string, int> cache(config);
  cache.put("a", 1);
  const auto value = cache.get("a");

  assert(value.has_value());
  assert(*value == 1);

  return 0;
}
