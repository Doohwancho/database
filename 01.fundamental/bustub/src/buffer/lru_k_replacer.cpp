//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lru_k_replacer.cpp
//
// Identification: src/buffer/lru_k_replacer.cpp
//
// Copyright (c) 2015-2025, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/lru_k_replacer.h"
#include <algorithm>
#include "common/exception.h"

namespace bustub {

static const size_t INF_TIMESTAMP = std::numeric_limits<size_t>::max();

LRUKNode::LRUKNode(frame_id_t fid, size_t k) : k_(k), fid_(fid) {}

auto LRUKNode::GetFrameId() const -> frame_id_t { return fid_; }

auto LRUKNode::IsEvictable() const -> bool { return is_evictable_; }

auto LRUKNode::SetEvictable(bool set_evictable) -> void { is_evictable_ = set_evictable; }

auto LRUKNode::RecordAccess(size_t timestamp, AccessType type) -> void {
    if (history_.size() == k_) {
        history_.pop_front();
    }
    history_.push_back(timestamp);
}

auto LRUKNode::GetEarliestTimestamp() const -> size_t { 
    if (history_.empty()) {
        return INF_TIMESTAMP;
    }
    return history_.front(); 
}

auto LRUKNode::GetKBackDist(size_t current_timestamp) const -> size_t {
    //access한 빈도수가 k값보다 작으면 얜 접근 많이 안했으니까 별로 안중요한 frame인거야. 그러니 inf_timestamp 반환해서 evict() 시키자.
    if (history_.size() < k_) {
        return INF_TIMESTAMP;
    }

    //현재 시간 - access했었을 때의 시간차로 얼마나 마지막으로 access한 시간이 얼마나 오래걸렸는지 측정 
    return current_timestamp - history_.front(); 
}

auto LRUKNode::GetWeightedKBackDist(size_t current_timestamp) const -> size_t {
    return GetKBackDist(current_timestamp);
}

auto LRUKNode::GetAccessWeight(AccessType type) -> size_t {
    switch (type) {
    case (AccessType::Unknown):
    case (AccessType::Index):
        return 1;
    case (AccessType::Scan):
        return 2;
    case (AccessType::Lookup):
        return 3;
    default:
        return 1;
    }
}

LRUKReplacer::LRUKReplacer(size_t num_frames, size_t k) : replacer_size_(num_frames), k_(k) {}


/*
---
what 

이게 젤 중요! LRU-K 알고리즘 돌려서 젤 access가 덜되면서 오래된 frame을 disk에 보내자!

---
작동방식

1. 두 종류의 프레임 나눔:
    - K번 미만 접근한 애들 (inf_k_dist_frames)
    - K번 이상 접근한 애들 (k_dist_frames)
2. 누구 쫓아낼지 규칙:
    - K번 미만 접근한 애들 중에 있으면, 제일 처음에 접근한 놈(가장 오래된 타임스탬프) 쫓아냄
    - 다 K번 이상 접근했으면, K-거리 제일 큰 놈 쫓아냄
    - K-거리 똑같으면 가장 오래된 접근 시간 가진 놈 쫓아냄
*/
auto LRUKReplacer::Evict() -> std::optional<frame_id_t> {
    std::scoped_lock lock(latch_);
    //Q. what is scoped_lock?

    if (node_store_.empty() || curr_size_ == 0) {
        return std::nullopt;
    }
  
    // Use two vectors to track frames with K accesses and frames with less than K accesses
    std::vector<frame_id_t> inf_k_dist_frames;
    std::vector<std::pair<frame_id_t, size_t>> k_dist_frames;
  
    // Categorize evictable frames
    for (const auto &[fid, node] : node_store_) {
        //Q. what is evictable and non-evictable node? isn't it you evict it right away? why flag/mark it?
        if (!node.IsEvictable()) {
            continue;
        }
      
        size_t k_dist = node.GetKBackDist(current_timestamp_);
        
        if (k_dist == INF_TIMESTAMP) {
            // K번 미만 접근한 놈들(frame)
            inf_k_dist_frames.push_back(fid);
        } else {
            // K번 이상 접근한 놈들(frame)
            k_dist_frames.push_back({fid, k_dist});
        }
    }
  
    frame_id_t victim_id;
    
    // K번 미만 접근한 애들 중에서 가장 오래된 놈 쫓아냄
    // (K번 미만 접근한 애들이 우선순위 높음)
    if (!inf_k_dist_frames.empty()) {
        // If any frames have less than k accesses, evict the one with the earliest first access
        size_t earliest_timestamp = INF_TIMESTAMP;
        victim_id = inf_k_dist_frames[0];
        
        for (const auto &fid : inf_k_dist_frames) {
            const auto &node = node_store_[fid];
            if (node.GetEarliestTimestamp() < earliest_timestamp) {
                earliest_timestamp = node.GetEarliestTimestamp();
                victim_id = fid;
            }
        }
    } else {
        // K번 이상 접근한 애들 중에서 K-거리 제일 큰 놈 쫓아냄
        victim_id = k_dist_frames[0].first;
        size_t max_k_dist = k_dist_frames[0].second;
        
        for (const auto &[fid, k_dist] : k_dist_frames) {
            if (k_dist > max_k_dist) {
                max_k_dist = k_dist;
                victim_id = fid;
            } else if (k_dist == max_k_dist) {
                // Break ties using the earliest timestamp
                const auto &current_node = node_store_[victim_id];
                const auto &new_node = node_store_[fid];
                if (new_node.GetEarliestTimestamp() < current_node.GetEarliestTimestamp()) {
                    victim_id = fid;
                }
            }
        }
    }
  
    node_store_.erase(victim_id);
    curr_size_--;
    return victim_id;
}

  /**
   *
   * @brief Record the event that the given frame id is accessed at current timestamp.
   * Create a new entry for access history if frame id has not been seen before.
   *
   * If frame id is invalid (ie. larger than replacer_size_), throw an exception. You can
   * also use BUSTUB_ASSERT to abort the process if frame id is invalid.
   *
   * @param frame_id id of frame that received a new access.
   * @param access_type type of access that was received. This parameter is only needed for
   * leaderboard tests.
   */
void LRUKReplacer::RecordAccess(frame_id_t frame_id, const AccessType access_type) {
    std::scoped_lock lock(latch_);
    if (static_cast<size_t>(frame_id) >= replacer_size_) {
        throw std::invalid_argument{"invalid frame id"};
    }
    
    auto it = node_store_.find(frame_id);
    
    if (it == node_store_.end()) {
        // Create a new node and initialize it
        LRUKNode new_node(frame_id, k_);
        new_node.RecordAccess(current_timestamp_, access_type);
        node_store_.insert({frame_id, new_node});
    } else {
        it->second.RecordAccess(current_timestamp_, access_type);
    }
    
    current_timestamp_++;
}

void LRUKReplacer::SetEvictable(frame_id_t frame_id, bool set_evictable) {
    std::scoped_lock lock(latch_);
    
    // Check if frame_id is valid
    if (static_cast<size_t>(frame_id) >= replacer_size_) {
        throw std::invalid_argument{"invalid frame id"};
    }
    
    auto it = node_store_.find(frame_id);
    
    if (it == node_store_.end()) {
        return;  // If frame not found, return without modification
    }
  
    if (set_evictable && !it->second.IsEvictable()) {
        it->second.SetEvictable(true);
        curr_size_++;
    } else if (!set_evictable && it->second.IsEvictable()) {
        it->second.SetEvictable(false);
        curr_size_--;
    }
}

void LRUKReplacer::Remove(frame_id_t frame_id) {
    std::scoped_lock lock(latch_);
    auto it = node_store_.find(frame_id);
    if (it == node_store_.end()) {
        return;
    }
  
    if (!it->second.IsEvictable()) {
        throw std::invalid_argument{"frame is not evictable"};
    }
  
    curr_size_--;
    node_store_.erase(frame_id);
}

auto LRUKReplacer::Size() -> size_t { return curr_size_; }

}  // namespace bustub
