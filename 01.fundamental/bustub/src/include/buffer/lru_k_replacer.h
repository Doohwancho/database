//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lru_k_replacer.h
//
// Identification: src/include/buffer/lru_k_replacer.h
//
// Copyright (c) 2015-2025, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <limits>
#include <list>
#include <map>
#include <mutex>  // NOLINT
#include <optional>
#include <unordered_map>
#include <vector>

#include "common/config.h"
#include "common/macros.h"

/*

1. LRUKNode 클래스: 프레임 하나 정보 저장하는 구조체같은 놈임
  1. 변수 
    - history_: 접근 타임스탬프 기록하는 리스트
    - size_t k_{0}; : Default value for k
    - frame_id_t fid_{0}; : Default value for frame_id
    - bool is_evictable_{false}; : evict()해서 내쫓기 가능함? flag 
  2. 함수 
    - GetKBackDist(): K-거리 계산해주는 함수
    - IsEvictable(): 이 프레임 쫓아낼 수 있냐?
    - RecordAccess(): 접근 기록 추가하는 함수
    - GetEarliestTimestamp(): 가장 오래된 접근 시간 알려주는 함수
2. LRUKReplacer 클래스 
  1. 변수 
    - std::map<frame_id_t, LRUKNode> node_store_; : 이게 전체 LRUKNode 담는 map, frame 별로.
    - size_t current_timestamp_{1}; : 현재 시간 
    - size_t curr_size_{0}; : 총 LRUKNode가 몇개있는지?
    - size_t replacer_size_; : capacity인 듯?
    - size_t k_; : k값
    - std::mutex latch_; : lock 
  2. 함수 
    - RecordAccess(): 어떤 프레임에 접근했다고 기록
    - SetEvictable(): 프레임 쫓아낼 수 있게/없게 설정
    - Evict(): 실제로 쫓아낼 놈 고르는 핵심 함수 <------------ 핵심!!!!!!!!!!!!!
    - Remove(): 지정한 프레임 강제로 제거
    - Size(): 쫓아낼 수 있는 프레임 개수 반환
*/


namespace bustub {

enum class AccessType { Unknown = 0, Lookup, Scan, Index };

class LRUKNode {
  public:
  LRUKNode() = default;

  /**
   * @brief Constructs a new LRU-K Node.
   * @param fid The frame id associated with this node.
   * @param k The value of K for the LRU-K algorithm.
   */
  LRUKNode(frame_id_t fid, size_t k);

  /**
   * @brief Retrieves the frame id associated with this node.
   * @return The frame id.
   */
  auto GetFrameId() const -> frame_id_t;

  /**
   * @brief Checks if the node is evictable.
   * @return True if the node is evictable, otherwise False.
   */
  auto IsEvictable() const -> bool;

  /**
   * @brief Sets the evictable status of this node.
   * @param set_evictable The evictable status to be set.
   */
  auto SetEvictable(bool set_evictable) -> void;

  /**
   * @brief Records an access at the given timestamp.
   * @param timestamp The timestamp of the access.
   */
  auto RecordAccess(size_t timestamp, AccessType type = AccessType::Unknown) -> void;

  /**
   * @brief Retrieves the earliest timestamp from the node's access history.
   * @return The earliest timestamp.
   */
  auto GetEarliestTimestamp() const -> size_t;

  /**
   * @brief Computes the backward K-distance based on the current timestamp.
   * @param current_timestamp The current timestamp.
   * @return The computed backward K-distance.
   */
  auto GetKBackDist(size_t current_timestamp) const -> size_t;

  auto GetWeightedKBackDist(size_t current_timestamp) const -> size_t;

 private:
  static auto GetAccessWeight(AccessType type) -> size_t;

  std::list<size_t> history_;
  size_t k_{0};  // Default value for k
  frame_id_t fid_{0};  // Default value for frame_id
  bool is_evictable_{false};
};

/**
 * LRUKReplacer implements the LRU-k replacement policy.
 *
 * The LRU-k algorithm evicts a frame whose backward k-distance is maximum
 * of all frames. Backward k-distance is computed as the difference in time between
 * current timestamp and the timestamp of kth previous access.
 *
 * A frame with less than k historical references is given
 * +inf as its backward k-distance. When multiple frames have +inf backward k-distance,
 * classical LRU algorithm is used to choose victim.
 */
class LRUKReplacer {
  public:
  /**
   * @brief a new LRUKReplacer.
   * @param num_frames the maximum number of frames the LRUReplacer will be required to store
   */
  explicit LRUKReplacer(size_t num_frames, size_t k);

  DISALLOW_COPY_AND_MOVE(LRUKReplacer);

  /**
   *
   * @brief Destroys the LRUReplacer.
   */
  ~LRUKReplacer() = default;

  /**
   *
   * @brief Find the frame with largest backward k-distance and evict that frame. Only frames
   * that are marked as 'evictable' are candidates for eviction.
   *
   * A frame with less than k historical references is given +inf as its backward k-distance.
   * If multiple frames have inf backward k-distance, then evict frame with earliest timestamp
   * based on LRU.
   *
   * Successful eviction of a frame should decrement the size of replacer and remove the frame's
   * access history.
   *
   * @return frame_id of the frame that is evicted, or std::nullopt if no frames can be evicted.
   */
  auto Evict() -> std::optional<frame_id_t>;

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
  void RecordAccess(frame_id_t frame_id, AccessType access_type = AccessType::Unknown);

  /**
   *
   * @brief Toggle whether a frame is evictable or non-evictable. This function also
   * controls replacer's size. Note that size is equal to number of evictable entries.
   *
   * If a frame was previously evictable and is to be set to non-evictable, then size should
   * decrement. If a frame was previously non-evictable and is to be set to evictable,
   * then size should increment.
   *
   * If frame id is invalid, throw an exception or abort the process.
   *
   * For other scenarios, this function should terminate without modifying anything.
   *
   * @param frame_id id of frame whose 'evictable' status will be modified
   * @param set_evictable whether the given frame is evictable or not
   */
  void SetEvictable(frame_id_t frame_id, bool set_evictable);

  /**
   *
   * @brief Remove an evictable frame from replacer, along with its access history.
   * This function should also decrement replacer's size if removal is successful.
   *
   * Note that this is different from evicting a frame, which always remove the frame
   * with largest backward k-distance. This function removes specified frame id,
   * no matter what its backward k-distance is.
   *
   * If Remove is called on a non-evictable frame, throw an exception or abort the
   * process.
   *
   * If specified frame is not found, directly return from this function.
   *
   * @param frame_id id of frame to be removed
   */
  void Remove(frame_id_t frame_id);

  /**
   *
   * @brief Return replacer's size, which tracks the number of evictable frames.
   *
   * @return size_t
   */
  auto Size() -> size_t;

 private:
  std::map<frame_id_t, LRUKNode> node_store_;
  size_t current_timestamp_{1};
  size_t curr_size_{0};
  size_t replacer_size_;
  size_t k_;
  std::mutex latch_;
};

}  // namespace bustub
