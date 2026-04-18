#pragma once

#include <cstddef>
#include <functional>
#include <optional>
#include <shared_mutex>
#include <unordered_map>

#include "sharded_lru/detail/lru_list.hpp"

namespace sharded_lru::detail {

template <typename Key, typename Value, typename Hasher>
class Shard {
 public:
  Shard(std::size_t capacity, Hasher hasher = Hasher{});

  bool put(const Key& key, Value value);
  std::optional<Value> get(const Key& key);
  bool erase(const Key& key);
  bool contains(const Key& key) const;
  void clear();
  std::size_t size() const;

 private:
  std::size_t capacity_;
  Hasher hasher_;
  mutable std::shared_mutex mutex_;
  std::unordered_map<Key, Value, Hasher> items_;
  LruList<Key> lru_;
};

}  // namespace sharded_lru::detail
