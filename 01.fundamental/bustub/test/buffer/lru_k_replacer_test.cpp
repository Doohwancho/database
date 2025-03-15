//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lru_k_replacer_test.cpp
//
// Identification: test/buffer/lru_k_replacer_test.cpp
//
// Copyright (c) 2015-2025, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

/**
 * lru_k_replacer_test.cpp
 */

#include "buffer/lru_k_replacer.h"

#include <algorithm>
#include <cstdio>
#include <memory>
#include <random>
#include <set>
#include <thread>  // NOLINT
#include <vector>

#include "gtest/gtest.h"

namespace bustub {

TEST(LRUKReplacerTest, SampleTest) {
  // Note that comparison with `std::nullopt` always results in `false`, and if the optional type actually does contain
  // a value, the comparison will compare the inner value.
  // See: https://devblogs.microsoft.com/oldnewthing/20211004-00/?p=105754
  std::optional<frame_id_t> frame;

  // Initialize the replacer.
  LRUKReplacer lru_replacer(7, 2); //num_frames = 7, k = 2

  // Add six frames to the replacer. We now have frames [1, 2, 3, 4, 5]. We set frame 6 as non-evictable.
  lru_replacer.RecordAccess(1);
  lru_replacer.RecordAccess(2);
  lru_replacer.RecordAccess(3);
  lru_replacer.RecordAccess(4);
  lru_replacer.RecordAccess(5);
  lru_replacer.RecordAccess(6);
  lru_replacer.SetEvictable(1, true);
  lru_replacer.SetEvictable(2, true);
  lru_replacer.SetEvictable(3, true);
  lru_replacer.SetEvictable(4, true);
  lru_replacer.SetEvictable(5, true);
  lru_replacer.SetEvictable(6, false);

  // The size of the replacer is the number of frames that can be evicted, _not_ the total number of frames entered.
  ASSERT_EQ(5, lru_replacer.Size());

  // Record an access for frame 1. Now frame 1 has two accesses total.
  // LRU-K 알고리즘에 의해, k값이 1만 2고, 나머지 2~5은 1씩이니까 오래된 순서인 2,3,4,5,1 순서대로 evict() 된다.
  lru_replacer.RecordAccess(1);

  // Evict three pages from the replacer.
  // To break ties, we use LRU with respect to the oldest timestamp, or the least recently used frame.
  ASSERT_EQ(2, lru_replacer.Evict());
  ASSERT_EQ(3, lru_replacer.Evict());
  ASSERT_EQ(4, lru_replacer.Evict());
  ASSERT_EQ(2, lru_replacer.Size());
  //2,3,4 뺐으니까 5,1 만 남음 (5,1), (1,2)임.

  // Insert new frames [3, 4], and update the access history for 5. Now, the ordering is [3, 1, 5, 4].
  //(5,1), (1,2), (3,1) 
  lru_replacer.RecordAccess(3);
  //(5,1), (1,2), (3,1), (4,1)
  lru_replacer.RecordAccess(4);
  //(1,2), (3,1), (4,1), (5,2)
  lru_replacer.RecordAccess(5);
  //(1,2), (3,1), (5,2), (4,2)
  lru_replacer.RecordAccess(4);
  lru_replacer.SetEvictable(3, true);
  lru_replacer.SetEvictable(4, true);
  ASSERT_EQ(4, lru_replacer.Size());
  //4개있으니까 사이즈 4

  //evict()할때, 1이 젤 오래됬지만, 그보다 더 우선순위인 각 노드당 k값이 2보다 더 큰지부터 보기 때문에, 해당사항 안되는 3을 evict()한다.
  //before) (1,2), (3,1), (5,2), (4,2) 
  //after) (1,2), (5,2), (4,2) 
  ASSERT_EQ(3, lru_replacer.Evict());
  ASSERT_EQ(3, lru_replacer.Size());
  //그래서 3이 남고 

  //나머지는 뭐 알아서...

  // Set 6 to be evictable. 6 Should be evicted next since it has the maximum backward k-distance.
  lru_replacer.SetEvictable(6, true);
  ASSERT_EQ(4, lru_replacer.Size());
  ASSERT_EQ(6, lru_replacer.Evict());
  ASSERT_EQ(3, lru_replacer.Size());

  // Mark frame 1 as non-evictable. We now have [5, 4].
  lru_replacer.SetEvictable(1, false);

  // We expect frame 5 to be evicted next.
  ASSERT_EQ(2, lru_replacer.Size());
  ASSERT_EQ(5, lru_replacer.Evict());
  ASSERT_EQ(1, lru_replacer.Size());

  // Update the access history for frame 1 and make it evictable. Now we have [4, 1].
  lru_replacer.RecordAccess(1);
  lru_replacer.RecordAccess(1);
  lru_replacer.SetEvictable(1, true);
  ASSERT_EQ(2, lru_replacer.Size());

  // Evict the last two frames.
  ASSERT_EQ(4, lru_replacer.Evict());
  ASSERT_EQ(1, lru_replacer.Size());
  ASSERT_EQ(1, lru_replacer.Evict());
  ASSERT_EQ(0, lru_replacer.Size());

  // Insert frame 1 again and mark it as non-evictable.
  lru_replacer.RecordAccess(1);
  lru_replacer.SetEvictable(1, false);
  ASSERT_EQ(0, lru_replacer.Size());

  // A failed eviction should not change the size of the replacer.
  frame = lru_replacer.Evict();
  ASSERT_EQ(false, frame.has_value());

  // Mark frame 1 as evictable again and evict it.
  lru_replacer.SetEvictable(1, true);
  ASSERT_EQ(1, lru_replacer.Size());
  ASSERT_EQ(1, lru_replacer.Evict());
  ASSERT_EQ(0, lru_replacer.Size());

  // There is nothing left in the replacer, so make sure this doesn't do something strange.
  frame = lru_replacer.Evict();
  ASSERT_EQ(false, frame.has_value());
  ASSERT_EQ(0, lru_replacer.Size());

  // Make sure that setting a non-existent frame as evictable or non-evictable doesn't do something strange.
  lru_replacer.SetEvictable(6, false);
  lru_replacer.SetEvictable(6, true);
}

}  // namespace bustub
