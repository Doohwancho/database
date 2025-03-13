//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// trie_store.h
//
// Identification: src/include/primer/trie_store.h
//
// Copyright (c) 2015-2025, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <optional>
#include <shared_mutex>
#include <utility>

#include "primer/trie.h"

namespace bustub {

// This class is used to guard the value returned by the trie. It holds a reference to the root so
// that the reference to the value will not be invalidated.
template <class T>
class ValueGuard {
 public:
  ValueGuard(Trie root, const T &value) : root_(std::move(root)), value_(value) {}
  auto operator*() const -> const T & { return value_; }

 private:
  Trie root_;
  const T &value_;
};

// This class is a thread-safe wrapper around the Trie class. It provides a simple interface for
// accessing the trie. It should allow concurrent reads and a single write operation at the same
// time.
class TrieStore {
 public:
  template <class T>
  auto Get(std::string_view key) -> std::optional<ValueGuard<T>>;

  template <class T>
  void Put(std::string_view key, T value);

  void Remove(std::string_view key);

 private:
  std::mutex root_lock_; //root node에 대한 접근만 protect함. 다르노드 말고.

  std::mutex write_lock_; //모든 tree에 대한 접근을 protect 하는 듯 

  // Stores the current root for the trie.
  Trie root_;
};

}  // namespace bustub
