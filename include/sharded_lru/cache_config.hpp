#pragma once

#include <cstddef>

namespace sharded_lru {

struct CacheConfig {
  std::size_t capacity = 1024;
  std::size_t shards = 16;
};

}  // namespace sharded_lru
