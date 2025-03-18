//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// page_guard.cpp
//
// Identification: src/storage/page/page_guard.cpp
//
// Copyright (c) 2015-2025, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "storage/page/page_guard.h"
#include <memory>

namespace bustub {

/**
 * @brief The only constructor for an RAII `ReadPageGuard` that creates a valid guard.
 *
 * Note that only the buffer pool manager is allowed to call this constructor.
 *
 * TODO(P1): Add implementation.
 *
 * @param page_id The page ID of the page we want to read.
 * @param frame A shared pointer to the frame that holds the page we want to protect.
 * @param replacer A shared pointer to the buffer pool manager's replacer.
 * @param bpm_latch A shared pointer to the buffer pool manager's latch.
 * @param disk_scheduler A shared pointer to the buffer pool manager's disk scheduler.
 */
ReadPageGuard::ReadPageGuard(page_id_t page_id, std::shared_ptr<FrameHeader> frame,
                             std::shared_ptr<LRUKReplacer> replacer, std::shared_ptr<std::mutex> bpm_latch,
                             std::shared_ptr<DiskScheduler> disk_scheduler)
    : page_id_(page_id),
      frame_(std::move(frame)),
      replacer_(std::move(replacer)),
      bpm_latch_(std::move(bpm_latch)),
      disk_scheduler_(std::move(disk_scheduler)) , is_valid_(true) {
  // 공유 락을 이미 획득했다고 가정 (buffer_pool_manager.cpp에서 처리)

  // UNIMPLEMENTED("TODO(P1): Add implementation.");
}

/**
 * @brief The move constructor for `ReadPageGuard`.
 *
 * ### Implementation
 *
 * If you are unfamiliar with move semantics, please familiarize yourself with learning materials online. There are many
 * great resources (including articles, Microsoft tutorials, YouTube videos) that explain this in depth.
 *
 * Make sure you invalidate the other guard, otherwise you might run into double free problems! For both objects, you
 * need to update _at least_ 5 fields each.
 *
 * TODO(P1): Add implementation.
 *
 * @param that The other page guard.
 */
// ReadPageGuard::ReadPageGuard(ReadPageGuard &&that) noexcept {}
ReadPageGuard::ReadPageGuard(ReadPageGuard &&that) noexcept
    : page_id_(that.page_id_),
      frame_(std::move(that.frame_)),
      replacer_(std::move(that.replacer_)),
      bpm_latch_(std::move(that.bpm_latch_)),
      disk_scheduler_(std::move(that.disk_scheduler_)),
      is_valid_(that.is_valid_) {
  // 기존 가드 무효화
  that.is_valid_ = false;
  that.frame_ = nullptr;
  that.replacer_ = nullptr;
  that.bpm_latch_ = nullptr;
  that.disk_scheduler_ = nullptr;
}

/**
 * 객체 이동한 후에 원본 데이터 무효화 하는게 핵심임.
 * 원본 데이터 무효화 안하면 이미 free()된 원본 데이터에 또 free()해서 에러터질 수 있는 가능성이 있음. 
 * 
 * 이동 후에 원본 가드는 무효화되고, 새로운 가드는 유효한 상태여야 함
 * 잘못된 이동 처리하면 락이 해제 안 되거나 페이지가 영원히 고정될 수 있어서 중요함
 * 
 * @brief The move assignment operator for `ReadPageGuard`.
 *
 * ### Implementation
 *
 * If you are unfamiliar with move semantics, please familiarize yourself with learning materials online. There are many
 * great resources (including articles, Microsoft tutorials, YouTube videos) that explain this in depth.
 *
 * Make sure you invalidate the other guard, otherwise you might run into double free problems! For both objects, you
 * need to update _at least_ 5 fields each, and for the current object, make sure you release any resources it might be
 * holding on to.
 *
 * TODO(P1): Add implementation.
 *
 * @param that The other page guard.
 * @return ReadPageGuard& The newly valid `ReadPageGuard`.
 */
auto ReadPageGuard::operator=(ReadPageGuard &&that) noexcept -> ReadPageGuard & {
  if (this != &that) {
    // 현재 리소스 해제
    Drop();
    
    // 리소스 이동
    page_id_ = that.page_id_;
    frame_ = std::move(that.frame_);
    replacer_ = std::move(that.replacer_);
    bpm_latch_ = std::move(that.bpm_latch_);
    disk_scheduler_ = std::move(that.disk_scheduler_);
    is_valid_ = that.is_valid_;
    
    // 기존 가드 무효화
    that.is_valid_ = false;
    that.frame_ = nullptr;
    that.replacer_ = nullptr;
    that.bpm_latch_ = nullptr;
    that.disk_scheduler_ = nullptr;
  }
  return *this;
}

/**
 * @brief Gets the page ID of the page this guard is protecting.
 */
auto ReadPageGuard::GetPageId() const -> page_id_t {
  BUSTUB_ENSURE(is_valid_, "tried to use an invalid read guard");
  return page_id_;
}

/**
 * @brief Gets a `const` pointer to the page of data this guard is protecting.
 */
auto ReadPageGuard::GetData() const -> const char * {
  BUSTUB_ENSURE(is_valid_, "tried to use an invalid read guard");
  return frame_->GetData();
}

/**
 * @brief Returns whether the page is dirty (modified but not flushed to the disk).
 */
auto ReadPageGuard::IsDirty() const -> bool {
  BUSTUB_ENSURE(is_valid_, "tried to use an invalid read guard");
  return frame_->is_dirty_;
}

/**
 * @brief Flushes this page's data safely to disk.
 *
 * TODO(P1): Add implementation.
 */
// void ReadPageGuard::Flush() { UNIMPLEMENTED("TODO(P1): Add implementation."); }
void ReadPageGuard::Flush() {
  BUSTUB_ENSURE(is_valid_, "trying to flush an invalid read guard");
  if (frame_->is_dirty_) {
    auto promise = disk_scheduler_->CreatePromise();
    disk_scheduler_->Schedule({true, frame_->GetDataMut(), page_id_, std::move(promise)});
    frame_->is_dirty_ = false;
  }
}

/**
 * @brief Manually drops a valid `ReadPageGuard`'s data. If this guard is invalid, this function does nothing.
 *
 * ### Implementation
 *
 * Make sure you don't double free! Also, think **very** **VERY** carefully about what resources you own and the order
 * in which you release those resources. If you get the ordering wrong, you will very likely fail one of the later
 * Gradescope tests. You may also want to take the buffer pool manager's latch in a very specific scenario...
 *
 * TODO(P1): Add implementation.
 */
void ReadPageGuard::Drop() {
  if (!is_valid_) {
    return;
  }
  // std::cerr << "Dropping page guard for page " << page_id_ 
  //           << " with pin count before read::drop: " << frame_->pin_count_ << std::endl;

  
  // 공유 락 해제
  frame_->rwlatch_.unlock_shared();
  
  // 핀 카운트 감소 및 교체자 상태 업데이트
  {
    std::scoped_lock<std::mutex> lock(*bpm_latch_);
    if (frame_->pin_count_ > 0) {  // 핀 카운트가 음수가 되는 것 방지
      frame_->pin_count_--;
      if (frame_->pin_count_ == 0) {
        replacer_->SetEvictable(frame_->frame_id_, true);
      }
    }
  }
  
  // 가드 무효화
  is_valid_ = false;

  // std::cerr << "Pin count after read::drop: " << frame_->pin_count_ << std::endl;
}


//RAII -> 페이지 가드가 {} 벗어날 때 자동으로 Drop() 함수 호출 
/** @brief The destructor for `ReadPageGuard`. This destructor simply calls `Drop()`. */
ReadPageGuard::~ReadPageGuard() { Drop(); }

/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/

/**
 * @brief The only constructor for an RAII `WritePageGuard` that creates a valid guard.
 *
 * Note that only the buffer pool manager is allowed to call this constructor.
 *
 * TODO(P1): Add implementation.
 *
 * @param page_id The page ID of the page we want to write to.
 * @param frame A shared pointer to the frame that holds the page we want to protect.
 * @param replacer A shared pointer to the buffer pool manager's replacer.
 * @param bpm_latch A shared pointer to the buffer pool manager's latch.
 * @param disk_scheduler A shared pointer to the buffer pool manager's disk scheduler.
 */
WritePageGuard::WritePageGuard(page_id_t page_id, std::shared_ptr<FrameHeader> frame,
                               std::shared_ptr<LRUKReplacer> replacer, std::shared_ptr<std::mutex> bpm_latch,
                               std::shared_ptr<DiskScheduler> disk_scheduler)
    : page_id_(page_id),
      frame_(std::move(frame)),
      replacer_(std::move(replacer)),
      bpm_latch_(std::move(bpm_latch)),
      disk_scheduler_(std::move(disk_scheduler)), is_valid_(true) {
  // 배타적 락을 이미 획득했다고 가정 (buffer_pool_manager.cpp에서 처리)

  // UNIMPLEMENTED("TODO(P1): Add implementation.");
}

/**
 * @brief The move constructor for `WritePageGuard`.
 *
 * ### Implementation
 *
 * If you are unfamiliar with move semantics, please familiarize yourself with learning materials online. There are many
 * great resources (including articles, Microsoft tutorials, YouTube videos) that explain this in depth.
 *
 * Make sure you invalidate the other guard, otherwise you might run into double free problems! For both objects, you
 * need to update _at least_ 5 fields each.
 *
 * TODO(P1): Add implementation.
 *
 * @param that The other page guard.
 */
// WritePageGuard::WritePageGuard(WritePageGuard &&that) noexcept {}
WritePageGuard::WritePageGuard(WritePageGuard &&that) noexcept
    : page_id_(that.page_id_),
      frame_(std::move(that.frame_)),
      replacer_(std::move(that.replacer_)),
      bpm_latch_(std::move(that.bpm_latch_)),
      disk_scheduler_(std::move(that.disk_scheduler_)),
      is_valid_(that.is_valid_) {
  // 기존 가드 무효화
  that.is_valid_ = false;
  that.frame_ = nullptr;
  that.replacer_ = nullptr;
  that.bpm_latch_ = nullptr;
  that.disk_scheduler_ = nullptr;
}

/**
 * @brief The move assignment operator for `WritePageGuard`.
 *
 * ### Implementation
 *
 * If you are unfamiliar with move semantics, please familiarize yourself with learning materials online. There are many
 * great resources (including articles, Microsoft tutorials, YouTube videos) that explain this in depth.
 *
 * Make sure you invalidate the other guard, otherwise you might run into double free problems! For both objects, you
 * need to update _at least_ 5 fields each, and for the current object, make sure you release any resources it might be
 * holding on to.
 *
 * TODO(P1): Add implementation.
 *
 * @param that The other page guard.
 * @return WritePageGuard& The newly valid `WritePageGuard`.
 */
// auto WritePageGuard::operator=(WritePageGuard &&that) noexcept -> WritePageGuard & { return *this; }
auto WritePageGuard::operator=(WritePageGuard &&that) noexcept -> WritePageGuard & {
  if (this != &that) {
    // 현재 리소스 해제
    Drop();
    
    // 리소스 이동
    page_id_ = that.page_id_;
    frame_ = std::move(that.frame_);
    replacer_ = std::move(that.replacer_);
    bpm_latch_ = std::move(that.bpm_latch_);
    disk_scheduler_ = std::move(that.disk_scheduler_);
    is_valid_ = that.is_valid_;
    
    // 기존 가드 무효화
    that.is_valid_ = false;
    that.frame_ = nullptr;
    that.replacer_ = nullptr;
    that.bpm_latch_ = nullptr;
    that.disk_scheduler_ = nullptr;
  }
  return *this;
}

/**
 * @brief Gets the page ID of the page this guard is protecting.
 */
auto WritePageGuard::GetPageId() const -> page_id_t {
  BUSTUB_ENSURE(is_valid_, "tried to use an invalid write guard");
  return page_id_;
}

/**
 * @brief Gets a `const` pointer to the page of data this guard is protecting.
 */
auto WritePageGuard::GetData() const -> const char * {
  BUSTUB_ENSURE(is_valid_, "tried to use an invalid write guard");
  return frame_->GetData();
}

/**
 * @brief Gets a mutable pointer to the page of data this guard is protecting.
 */
auto WritePageGuard::GetDataMut() -> char * {
  BUSTUB_ENSURE(is_valid_, "tried to use an invalid write guard");
  frame_->is_dirty_ = true; // 이거 덕분에 데이터 수정하면 알아서 더티 상태 됨
                            // 나중에 프레임 교체할 때 더티 프레임이면 디스크에 쓰는 로직 발동함
  return frame_->GetDataMut();
}

/**
 * @brief Returns whether the page is dirty (modified but not flushed to the disk).
 */
auto WritePageGuard::IsDirty() const -> bool {
  BUSTUB_ENSURE(is_valid_, "tried to use an invalid write guard");
  return frame_->is_dirty_;
}

/**
 * @brief Flushes this page's data safely to disk.
 *
 * TODO(P1): Add implementation.
 */
void WritePageGuard::Flush() {
  BUSTUB_ENSURE(is_valid_, "trying to flush an invalid write guard");
  auto promise = disk_scheduler_->CreatePromise();
  disk_scheduler_->Schedule({true, frame_->GetDataMut(), page_id_, std::move(promise)});
  frame_->is_dirty_ = false;
}

/**
 * 서순이 매우 중요함!!
 * 1) 먼저 락 해제
 * 2) 그 다음 핀 카운트 감소
 * 3) 가드 무효화
 * 
 * 이 순서가 틀리면 리소스 누수 발생하거나 deadlock 걸릴 수 있음.
 * 특히 락 먼저 해제하고 핀 카운트 감소하는 순서 중요함
 * 
 * 만약 핀 카운트 먼저 감소시키고 락을 그 다음에 해체한다?
 * 1. 핀 카운트가 0이 되면서 페이지가 교체 가능 상태가 됨
 * 2. 하지만 락은 아직 해제되지 않은 상태
 * 3. 이때 다른 스레드가 같은 페이지를 요청하고 교체하려고 하면
 * 4. 락이 아직 해제되지 않아서 영원히 기다리게 됨 (데드락)
 * 
 * 
 * Q. frame lock을 해제시키고 buffer pool manager의 lock을 쓰네? 왜지?
 * A. 프레임의 락(rwlatch_)을 오래 유지할수록 다른 스레드들이 대기하는 시간이 길어짐
 *    그래서 가능한 빨리 프레임 락을 해제하는 것이 중요함
 * 
 * @brief Manually drops a valid `WritePageGuard`'s data. If this guard is invalid, this function does nothing.
 *
 * ### Implementation
 *
 * Make sure you don't double free! Also, think **very** **VERY** carefully about what resources you own and the order
 * in which you release those resources. If you get the ordering wrong, you will very likely fail one of the later
 * Gradescope tests. You may also want to take the buffer pool manager's latch in a very specific scenario...
 *
 * TODO(P1): Add implementation.
 */
// void WritePageGuard::Drop() { UNIMPLEMENTED("TODO(P1): Add implementation."); }

void WritePageGuard::Drop() {
  if (!is_valid_) {
    return;
  }
  // std::cerr << "Dropping write page guard for page " << page_id_ 
  // << " with pin count before drop: " << frame_->pin_count_ << std::endl;

  // 배타적 락 해제
  // 왜? 이미 데이터 접근이 끝났으니까 제일 먼저 락 푸는 것.
  frame_->rwlatch_.unlock();
  
  // 핀 카운트 감소 및 교체자 상태 업데이트
  {
    std::scoped_lock<std::mutex> lock(*bpm_latch_);
    //Q. buffer pool lock이 학교 전체 문 걸어잠그는거고, frame lock이 학교안에 교실 문 걸어잠그는거라면, 비용 적은 frame lock에서 pin_count -1 하는게 맞지 않나?
    //A. ㅇㅇ 맞음 buffer pool lock이 비용이 더 비쌈. 근데 pin_count는 단순히 프레임 내부의 독립적인 값이 아니라 버퍼풀매니저 전체 상태랑 관련된 값임
    //   SetEvictable() 역시 버퍼풀 매니저 내부 전체 상태를 변경하는 것. 
    //   frame lock은 페이지 내부 데이터 보호 목적이고, 버퍼 풀 락은 버퍼 풀의 메타 데이터가 목적임. 
    if (frame_->pin_count_ > 0) {
      frame_->pin_count_--;
      if (frame_->pin_count_ == 0) {
        replacer_->SetEvictable(frame_->frame_id_, true);
      }
    }
  } //스코프를 이용해서 bpm lock을 최대한 적게 사용하자. 
  
  // 가드 무효화
  is_valid_ = false;

  // std::cerr << "Pin count after write::drop: " << frame_->pin_count_ << std::endl;
}

//RAII -> 페이지 가드가 {} 벗어날 때 자동으로 Drop() 함수 호출 
/** @brief The destructor for `WritePageGuard`. This destructor simply calls `Drop()`. */
WritePageGuard::~WritePageGuard() { Drop(); }

}  // namespace bustub
