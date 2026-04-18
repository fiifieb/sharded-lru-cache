#pragma once

#include <list>
#include <unordered_map>

namespace sharded_lru::detail {

template <typename Key>
class LruList {
 public:
  using ListType = std::list<Key>;
  using Iterator = typename ListType::iterator;

  void touch(const Key& key);
  void remove(const Key& key);
  const Key& evict_candidate() const;
  std::size_t size() const;

 private:
  ListType order_;
  std::unordered_map<Key, Iterator> index_;
};

}  // namespace sharded_lru::detail
