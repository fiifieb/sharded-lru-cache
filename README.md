# Sharded LRU Cache (C++)

A concurrent sharded LRU cache intended for high write throughput.

## MVP Behavior Contract

### Configuration

- `capacity` is the global maximum entry count for the cache.
- `shards` is the number of independent shard instances used for concurrency.
- Capacity is split across shards using even distribution with deterministic remainder:
  - `base = capacity / shards`
  - `remainder = capacity % shards`
  - first `remainder` shards get `base + 1`, others get `base`

### Core API Semantics

- `put(key, value)`:
  - inserts new key or overwrites existing value in the selected shard
  - touched key becomes most-recently-used (MRU) in that shard
  - returns `true` when a new key is inserted, `false` on overwrite
- `get(key)`:
  - returns `std::optional<Value>`
  - on hit: returns value and updates recency (touch-on-read)
  - on miss: returns `std::nullopt`
- `erase(key)`:
  - removes key if present
  - returns `true` if removed, `false` if key not found
- `contains(key)`:
  - returns whether key currently exists
  - does not modify recency
- `clear()`:
  - removes all keys from all shards
- `size()`:
  - returns current total entry count across all shards
- `capacity()`:
  - returns configured global capacity
- `shard_count()`:
  - returns configured shard count

### Eviction Policy

- LRU is **per-shard**, not globally strict across all keys.
- On insert overflow in a shard, evict exactly one least-recently-used key from that same shard.
- A successful `get` moves key to MRU position for that shard.

### Concurrency Guarantees (MVP)

- Cache is shard-linearizable: operations on the same shard are serialized by that shard lock.
- Different shards can proceed concurrently.
- No global linearizability guarantee across all shards.
- `size()` is an aggregate snapshot across shards (accurate at return time per shard read, not a global transaction boundary).

## Project Layout

- `include/sharded_lru/`: public headers and internal detail headers.
- `tests/`: unit and concurrency tests.
- `benchmarks/`: microbenchmarks for throughput and latency.
- `examples/`: minimal usage examples.

## Build

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```
