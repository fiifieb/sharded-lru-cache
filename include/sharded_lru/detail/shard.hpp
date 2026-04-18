#pragma once

#include <cassert>
#include <cstddef>
#include <functional>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <unordered_map>
#include <utility>

#include "sharded_lru/detail/lru_list.hpp"

namespace sharded_lru::detail {

template <typename Key, typename Value, typename Hasher>
class Shard {
 public:
  Shard(std::size_t capacity, Hasher hasher = Hasher{})
      : capacity_(capacity), hasher_(std::move(hasher)), items_(0, hasher_) {}

  bool put(const Key& key, Value value) {
    std::unique_lock lock(mutex_);

    if (capacity_ == 0) {
      return false;
    }

    const auto found = items_.find(key);
    if (found != items_.end()) {
      found->second = std::move(value);
      lru_.touch(key);
      assert(items_.size() == lru_.size());
      return false;
    }

    items_.emplace(key, std::move(value));
    lru_.touch(key);

    if (items_.size() > capacity_) {
      const Key evict_key = lru_.evict_candidate();
      lru_.remove(evict_key);
      items_.erase(evict_key);
    }

    assert(items_.size() == lru_.size());
    assert(items_.size() <= capacity_);
    return true;
  }

  std::optional<Value> get(const Key& key) {
    std::unique_lock lock(mutex_);
    const auto found = items_.find(key);
    if (found == items_.end()) {
      return std::nullopt;
    }

    lru_.touch(key);
    assert(items_.size() == lru_.size());
    return found->second;
  }

  bool erase(const Key& key) {
    std::unique_lock lock(mutex_);
    const auto found = items_.find(key);
    if (found == items_.end()) {
      return false;
    }

    items_.erase(found);
    lru_.remove(key);
    assert(items_.size() == lru_.size());
    return true;
  }

  bool contains(const Key& key) const {
    std::shared_lock lock(mutex_);
    return items_.find(key) != items_.end();
  }

  void clear() {
    std::unique_lock lock(mutex_);
    items_.clear();
    lru_ = LruList<Key>{};
    assert(items_.size() == lru_.size());
  }

  std::size_t size() const {
    std::shared_lock lock(mutex_);
    return items_.size();
  }

 private:
  std::size_t capacity_;
  Hasher hasher_;
  mutable std::shared_mutex mutex_;
  std::unordered_map<Key, Value, Hasher> items_;
  LruList<Key> lru_;
};

}  // namespace sharded_lru::detail
