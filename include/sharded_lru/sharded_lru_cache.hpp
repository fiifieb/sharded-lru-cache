#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "sharded_lru/cache_config.hpp"
#include "sharded_lru/detail/shard.hpp"

namespace sharded_lru {

template <typename Key, typename Value, typename Hasher = std::hash<Key>>
class ShardedLruCache {
 public:
  explicit ShardedLruCache(CacheConfig config, Hasher hasher = Hasher{});

  bool put(const Key& key, Value value);
  std::optional<Value> get(const Key& key);
  bool erase(const Key& key);
  bool contains(const Key& key) const;
  void clear();

  std::size_t size() const;
  std::size_t capacity() const;
  std::size_t shard_count() const;

 private:
  CacheConfig config_;
  Hasher hasher_;
  std::vector<std::unique_ptr<detail::Shard<Key, Value, Hasher>>> shards_;
};

}  // namespace sharded_lru
