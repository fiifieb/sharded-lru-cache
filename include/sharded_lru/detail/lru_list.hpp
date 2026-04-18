#pragma once

#include <list>
#include <stdexcept>
#include <unordered_map>

namespace sharded_lru::detail {

template <typename Key>
class LruList {
 public:
  using ListType = std::list<Key>;
  using Iterator = typename ListType::iterator;

  void touch(const Key& key) {
    const auto it = index_.find(key);
    if (it != index_.end()) {
      order_.splice(order_.begin(), order_, it->second);
      it->second = order_.begin();
      return;
    }

    order_.push_front(key);
    index_.emplace(*order_.begin(), order_.begin());
  }

  void remove(const Key& key) {
    const auto it = index_.find(key);
    if (it == index_.end()) {
      return;
    }

    order_.erase(it->second);
    index_.erase(it);
  }

  const Key& evict_candidate() const {
    if (order_.empty()) {
      throw std::out_of_range("cannot evict from empty LRU list");
    }
    return order_.back();
  }

  std::size_t size() const { return order_.size(); }

 private:
  ListType order_;
  std::unordered_map<Key, Iterator> index_;
};

}  // namespace sharded_lru::detail
