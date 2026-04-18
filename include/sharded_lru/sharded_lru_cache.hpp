#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <cassert>
#include <stdexcept>
#include <utility>
#include <vector>

#include "sharded_lru/cache_config.hpp"
#include "sharded_lru/detail/shard.hpp"

namespace sharded_lru {

template <typename Key, typename Value, typename Hasher = std::hash<Key>>
class ShardedLruCache {
 public:
  explicit ShardedLruCache(CacheConfig config, Hasher hasher = Hasher{})
      : config_(config), hasher_(std::move(hasher)) {
    if (config_.shards == 0) {
      throw std::invalid_argument("cache shards must be greater than zero");
    }

    shards_.reserve(config_.shards);

    const std::size_t base_capacity = config_.capacity / config_.shards;
    const std::size_t remainder = config_.capacity % config_.shards;

    for (std::size_t index = 0; index < config_.shards; ++index) {
      const std::size_t shard_capacity = base_capacity + (index < remainder ? 1 : 0);
      shards_.push_back(std::make_unique<detail::Shard<Key, Value, Hasher>>(shard_capacity, hasher_));
    }
  }

  [[nodiscard]] bool put(const Key& key, Value value) {
    return shards_[shard_index(key)]->put(key, std::move(value));
  }

  [[nodiscard]] std::optional<Value> get(const Key& key) {
    return shards_[shard_index(key)]->get(key);
  }

  [[nodiscard]] bool erase(const Key& key) { return shards_[shard_index(key)]->erase(key); }

  [[nodiscard]] bool contains(const Key& key) const {
    return shards_[shard_index(key)]->contains(key);
  }

  void clear() {
    for (const auto& shard : shards_) {
      shard->clear();
    }
  }

  [[nodiscard]] std::size_t size() const {
    std::size_t total = 0;
    for (const auto& shard : shards_) {
      total += shard->size();
    }
    return total;
  }

  [[nodiscard]] std::size_t capacity() const { return config_.capacity; }

  [[nodiscard]] std::size_t shard_count() const { return shards_.size(); }

  [[nodiscard]] bool empty() const { return size() == 0; }

 private:
  std::size_t shard_index(const Key& key) const {
    assert(!shards_.empty());
    return hasher_(key) % shards_.size();
  }

  CacheConfig config_;
  Hasher hasher_;
  std::vector<std::unique_ptr<detail::Shard<Key, Value, Hasher>>> shards_;
};

}  // namespace sharded_lru
