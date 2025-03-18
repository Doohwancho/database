//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// buffer_pool_manager.cpp
//
// Identification: src/buffer/buffer_pool_manager.cpp
//
// Copyright (c) 2015-2025, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/buffer_pool_manager.h"

namespace bustub {

/**
 * @brief The constructor for a `FrameHeader` that initializes all fields to default values.
 *
 * See the documentation for `FrameHeader` in "buffer/buffer_pool_manager.h" for more information.
 *
 * @param frame_id The frame ID / index of the frame we are creating a header for.
 */
FrameHeader::FrameHeader(frame_id_t frame_id) : frame_id_(frame_id), data_(BUSTUB_PAGE_SIZE, 0) { Reset(); }

/**
 * @brief Get a raw const pointer to the frame's data.
 *
 * @return const char* A pointer to immutable data that the frame stores.
 */
auto FrameHeader::GetData() const -> const char * { return data_.data(); }

/**
 * @brief Get a raw mutable pointer to the frame's data.
 *
 * @return char* A pointer to mutable data that the frame stores.
 */
auto FrameHeader::GetDataMut() -> char * { return data_.data(); }

/**
 * @brief Resets a `FrameHeader`'s member fields.
 */
void FrameHeader::Reset() {
  std::fill(data_.begin(), data_.end(), 0);
  pin_count_.store(0);
  is_dirty_ = false;
}

/**
 * @brief Creates a new `BufferPoolManager` instance and initializes all fields.
 *
 * See the documentation for `BufferPoolManager` in "buffer/buffer_pool_manager.h" for more information.
 *
 * ### Implementation
 *
 * We have implemented the constructor for you in a way that makes sense with our reference solution. You are free to
 * change anything you would like here if it doesn't fit with you implementation.
 *
 * Be warned, though! If you stray too far away from our guidance, it will be much harder for us to help you. Our
 * recommendation would be to first implement the buffer pool manager using the stepping stones we have provided.
 *
 * Once you have a fully working solution (all Gradescope test cases pass), then you can try more interesting things!
 *
 * @param num_frames The size of the buffer pool.
 * @param disk_manager The disk manager.
 * @param k_dist The backward k-distance for the LRU-K replacer.
 * @param log_manager The log manager. Please ignore this for P1.
 */
BufferPoolManager::BufferPoolManager(size_t num_frames, DiskManager *disk_manager, size_t k_dist,
                                     LogManager *log_manager)
    : num_frames_(num_frames),
      next_page_id_(0),
      bpm_latch_(std::make_shared<std::mutex>()),
      replacer_(std::make_shared<LRUKReplacer>(num_frames, k_dist)),
      disk_scheduler_(std::make_shared<DiskScheduler>(disk_manager)),
      log_manager_(log_manager) {
  // Not strictly necessary...
  std::scoped_lock latch(*bpm_latch_);

  // Initialize the monotonically increasing counter at 0.
  next_page_id_.store(0);

  // Allocate all of the in-memory frames up front.
  frames_.reserve(num_frames_);

  // The page table should have exactly `num_frames_` slots, corresponding to exactly `num_frames_` frames.
  page_table_.reserve(num_frames_);

  // Initialize all of the frame headers, and fill the free frame list with all possible frame IDs (since all frames are
  // initially free).
  for (size_t i = 0; i < num_frames_; i++) {
    frames_.push_back(std::make_shared<FrameHeader>(i));
    free_frames_.push_back(static_cast<int>(i));
  }
}

/**
 * @brief Destroys the `BufferPoolManager`, freeing up all memory that the buffer pool was using.
 */
BufferPoolManager::~BufferPoolManager() = default;

/**
 * @brief Returns the number of frames that this buffer pool manages.
 */
auto BufferPoolManager::Size() const -> size_t { return num_frames_; }

/**
 * @brief Allocates a new page on disk. (새 페이지를 할당하고 그 ID를 반환합니다.)
 *
 * ### Implementation
 *
 * You will maintain a thread-safe, monotonically increasing counter in the form of a `std::atomic<page_id_t>`.
 * See the documentation on [atomics](https://en.cppreference.com/w/cpp/atomic/atomic) for more information.
 *
 * TODO(P1): Add implementation.
 *
 * @return The page ID of the newly allocated page.
 */
auto BufferPoolManager::NewPage() -> page_id_t { 
  std::scoped_lock latch(*bpm_latch_);
  
  // 사용 가능한 프레임 찾기
  frame_id_t frame_id;
  
  // 사용 가능한 프레임이 있는지 확인
  if (!free_frames_.empty()) {
    // 자유 프레임 목록에서 프레임 가져오기
    frame_id = free_frames_.front();
    free_frames_.pop_front();
  } else {
    // 교체해야 할 프레임 찾기
    if (replacer_->Size() == 0) {
      // 모든 프레임이 고정되어 있으면 nullptr 반환
      return INVALID_PAGE_ID;
    }
    
    // LRU-K 교체자에서 희생자 찾기
    auto victim = replacer_->Evict();
    if (!victim.has_value()) {
      return INVALID_PAGE_ID;
    }
    
    frame_id = victim.value();
    auto frame = frames_[frame_id];
    
    // 희생 프레임에 있던 페이지가 더러우면 디스크에 쓰기
    page_id_t old_page_id = INVALID_PAGE_ID;
    for (const auto &entry : page_table_) {
      if (entry.second == frame_id) {
        old_page_id = entry.first;
        break;
      }
    }
    
    if (old_page_id != INVALID_PAGE_ID) {
      if (frame->is_dirty_) {
        // 더러운 페이지를 디스크에 쓰기 - 완료될 때까지 대기
        auto promise = disk_scheduler_->CreatePromise();
        auto future = promise.get_future();
        disk_scheduler_->Schedule({true, frame->GetDataMut(), old_page_id, std::move(promise)});
        future.wait(); // 쓰기가 완료될 때까지 대기
      }
      // 페이지 테이블에서 제거
      page_table_.erase(old_page_id);
    }
  }
  
  // 새 페이지 ID 할당
  page_id_t new_page_id = next_page_id_++;
  
  // 프레임 초기화
  auto frame = frames_[frame_id];
  frame->Reset();
  
  // 페이지 테이블 업데이트
  page_table_[new_page_id] = frame_id;
  
  // LRU-K 교체자에 접근 기록 추가
  replacer_->RecordAccess(frame_id);
  replacer_->SetEvictable(frame_id, false);  // 페이지가 핀되었으므로 교체 불가능으로 설정
  
  // 핀 카운트 증가 (여기서 빠트린 부분)
  // frame->pin_count_++; //핀카운트 여기서 증가 안시키고, CheckedReadPage()나 CheckedWritePage()에서 증가시키자. 
  
  return new_page_id;
}

/**
 * 페이지 삭제한다. (both on disk & in memory)
 * 근데 해당 페이지가 pinned 되있으면 does nothing and return false
 * 
 * @brief Removes a page from the database, both on disk and in memory. 
 *
 * If the page is pinned in the buffer pool, this function does nothing and returns `false`. Otherwise, this function
 * removes the page from both disk and memory (if it is still in the buffer pool), returning `true`.
 *
 * ### Implementation
 *
 * Think about all of the places a page or a page's metadata could be, and use that to guide you on implementing this
 * function. You will probably want to implement this function _after_ you have implemented `CheckedReadPage` and
 * `CheckedWritePage`.
 *
 * Ideally, we would want to ensure that all space on disk is used efficiently. That would mean the space that deleted
 * pages on disk used to occupy should somehow be made available to new pages allocated by `NewPage`.
 *
 * If you would like to attempt this, you are free to do so. However, for this implementation, you are allowed to
 * assume you will not run out of disk space and simply keep allocating disk space upwards in `NewPage`.
 *
 * For (nonexistent) style points, you can still call `DeallocatePage` in case you want to implement something slightly
 * more space-efficient in the future.
 *
 * TODO(P1): Add implementation.
 *
 * @param page_id The page ID of the page we want to delete.
 * @return `false` if the page exists but could not be deleted, `true` if the page didn't exist or deletion succeeded.
 */
auto BufferPoolManager::DeletePage(page_id_t page_id) -> bool {
  // debug) pin count 먼저 확인해보기 
  auto pin_count_opt = GetPinCount(page_id);
  if (pin_count_opt.has_value()) {
    std::cerr << "Trying to delete page " << page_id << " with pin count: " 
              << pin_count_opt.value() << std::endl;
  } else {
    std::cerr << "Trying to delete page " << page_id << " which is not in buffer pool" << std::endl;
  }


  std::scoped_lock latch(*bpm_latch_);
  
  // 페이지가 페이지 테이블에 없으면 true 반환
  if (page_table_.find(page_id) == page_table_.end()) {
    return true;
  }
  
  frame_id_t frame_id = page_table_[page_id];
  auto frame = frames_[frame_id];
  
  // 페이지가 핀되어 있으면 삭제할 수 없음
  if (frame->pin_count_ > 0) {
    return false;
  }
  
  // 페이지 테이블에서 제거
  page_table_.erase(page_id);
  
  // 교체자에서 제거
  replacer_->Remove(frame_id);
  
  // 프레임 초기화
  frame->Reset();
  
  // 자유 프레임 목록에 추가
  free_frames_.push_back(frame_id);
  
  // 디스크에서 페이지 삭제
  disk_scheduler_->DeallocatePage(page_id);
  
  return true;
}

/**
 * 1. 페이지가 버퍼 풀에 있는지 확인
 * 2. 없으면 새 프레임 할당 (자유 프레임 또는 희생자 선택)
 * 3. 디스크에서 페이지 데이터 읽어옴 (동기 I/O)
 * 4. 프레임에 핀 카운트 증가시키고 exclusive_lock 획득
 * 5. WritePageGuard 생성해서 반환
 * 
 * @brief Acquires an optional write-locked guard over a page of data. The user can specify an `AccessType` if needed.
 *
 * If it is not possible to bring the page of data into memory, this function will return a `std::nullopt`.
 *
 * Page data can _only_ be accessed via page guards. Users of this `BufferPoolManager` are expected to acquire either a
 * `ReadPageGuard` or a `WritePageGuard` depending on the mode in which they would like to access the data, which
 * ensures that any access of data is thread-safe.
 *
 * There can only be 1 `WritePageGuard` reading/writing a page at a time. This allows data access to be both immutable
 * and mutable, meaning the thread that owns the `WritePageGuard` is allowed to manipulate the page's data however they
 * want. If a user wants to have multiple threads reading the page at the same time, they must acquire a `ReadPageGuard`
 * with `CheckedReadPage` instead.
 *
 * ### Implementation
 *
 * There are 3 main cases that you will have to implement. The first two are relatively simple: one is when there is
 * plenty of available memory, and the other is when we don't actually need to perform any additional I/O. Think about
 * what exactly these two cases entail.
 *
 * The third case is the trickiest, and it is when we do not have any _easily_ available memory at our disposal. The
 * buffer pool is tasked with finding memory that it can use to bring in a page of memory, using the replacement
 * algorithm you implemented previously to find candidate frames for eviction.
 *
 * Once the buffer pool has identified a frame for eviction, several I/O operations may be necessary to bring in the
 * page of data we want into the frame.
 *
 * There is likely going to be a lot of shared code with `CheckedReadPage`, so you may find creating helper functions
 * useful.
 *
 * These two functions are the crux of this project, so we won't give you more hints than this. Good luck!
 *
 * TODO(P1): Add implementation.
 *
 * @param page_id The ID of the page we want to write to.
 * @param access_type The type of page access.
 * @return std::optional<WritePageGuard> An optional latch guard where if there are no more free frames (out of memory)
 * returns `std::nullopt`, otherwise returns a `WritePageGuard` ensuring exclusive and mutable access to a page's data.
 */
auto BufferPoolManager::CheckedWritePage(page_id_t page_id, AccessType access_type) -> std::optional<WritePageGuard> {
  frame_id_t frame_id;
  bool is_in_buffer_pool;
  std::shared_ptr<FrameHeader> frame;
  
  {
    // 버퍼 풀 잠금을 최소한으로 유지
    std::scoped_lock latch(*bpm_latch_);
    
    is_in_buffer_pool = page_table_.find(page_id) != page_table_.end();
    
    if (is_in_buffer_pool) {
      // 페이지가 이미 버퍼 풀에 있는 경우
      frame_id = page_table_[page_id];
    } else {
      // 페이지가 버퍼 풀에 없는 경우, 프레임 할당 필요
      if (!free_frames_.empty()) {
        // 자유 프레임 사용
        frame_id = free_frames_.front();
        free_frames_.pop_front();
      } else {
        // 교체자에서 희생자 찾기
        auto victim = replacer_->Evict();
        if (!victim.has_value()) {
          // 모든 페이지가 핀되어 있으면 교체할 수 없음
          return std::nullopt;
        }
        
        frame_id = victim.value();
        auto victim_frame = frames_[frame_id];
        
        // 희생 프레임에 있던 페이지가 더러우면 디스크에 쓰기
        page_id_t old_page_id = INVALID_PAGE_ID;
        for (const auto &entry : page_table_) {
          if (entry.second == frame_id) {
            old_page_id = entry.first;
            break;
          }
        }
        
        if (old_page_id != INVALID_PAGE_ID) {
          if (victim_frame->is_dirty_) {
            // 더러운 페이지를 디스크에 쓰기
            auto promise = disk_scheduler_->CreatePromise();
            auto future = promise.get_future();
            disk_scheduler_->Schedule({true, victim_frame->GetDataMut(), old_page_id, std::move(promise)});
            future.wait(); // 쓰기가 완료될 때까지 대기
          }
          
          // 페이지 테이블에서 제거
          page_table_.erase(old_page_id);
        }
      }
      
      // 새로운 프레임 초기화
      auto new_frame = frames_[frame_id];
      new_frame->Reset();
      
      // 페이지 테이블 업데이트
      page_table_[page_id] = frame_id;
    }
    
    // 프레임 획득
    frame = frames_[frame_id];
    
    // 핀 카운트 증가
    frame->pin_count_++;
    
    // LRU-K 교체자에 접근 기록 추가
    replacer_->RecordAccess(frame_id, access_type);
    replacer_->SetEvictable(frame_id, false);  // 페이지가 핀되었으므로 교체 불가능으로 설정
  }
  
  // 버퍼 풀 잠금 밖에서 디스크 I/O 수행
  if (!is_in_buffer_pool) {
    auto promise = disk_scheduler_->CreatePromise();
    auto future = promise.get_future();
    disk_scheduler_->Schedule({false, frame->GetDataMut(), page_id, std::move(promise)});
    future.wait();  // 읽기가 완료될 때까지 대기
  }
  
  // 쓰기 락 획득 - 차단형으로 변경
  frame->rwlatch_.lock();  // try_lock() 대신 lock() 사용
  
  // WritePageGuard 반환
  return WritePageGuard(page_id, frame, replacer_, bpm_latch_, disk_scheduler_);
}

/**
 * 1. 페이지가 버퍼 풀에 있는지 확인
 * 2. 없으면 새 프레임 할당 (자유 프레임 또는 희생자 선택)
 * 3. 디스크에서 페이지 데이터 읽어옴 (동기 I/O)
 * 4. 프레임에 핀 카운트 증가시키고 shared_lock 획득
 * 5. ReadPageGuard 생성해서 반환
 * 
 * @brief Acquires an optional read-locked guard over a page of data. The user can specify an `AccessType` if needed.
 *
 * If it is not possible to bring the page of data into memory, this function will return a `std::nullopt`.
 *
 * Page data can _only_ be accessed via page guards. Users of this `BufferPoolManager` are expected to acquire either a
 * `ReadPageGuard` or a `WritePageGuard` depending on the mode in which they would like to access the data, which
 * ensures that any access of data is thread-safe.
 *
 * There can be any number of `ReadPageGuard`s reading the same page of data at a time across different threads.
 * However, all data access must be immutable. If a user wants to mutate the page's data, they must acquire a
 * `WritePageGuard` with `CheckedWritePage` instead.
 *
 * ### Implementation
 *
 * See the implementation details of `CheckedWritePage`.
 *
 * TODO(P1): Add implementation.
 *
 * @param page_id The ID of the page we want to read.
 * @param access_type The type of page access.
 * @return std::optional<ReadPageGuard> An optional latch guard where if there are no more free frames (out of memory)
 * returns `std::nullopt`, otherwise returns a `ReadPageGuard` ensuring shared and read-only access to a page's data.
 */
auto BufferPoolManager::CheckedReadPage(page_id_t page_id, AccessType access_type) -> std::optional<ReadPageGuard> {
  std::scoped_lock latch(*bpm_latch_);
  
  frame_id_t frame_id;
  bool is_in_buffer_pool = page_table_.find(page_id) != page_table_.end();
  
  if (is_in_buffer_pool) {
    // 페이지가 이미 버퍼 풀에 있는 경우
    frame_id = page_table_[page_id];
  } else {
    // 페이지가 버퍼 풀에 없는 경우, 프레임 할당 필요
    if (!free_frames_.empty()) {
      // 자유 프레임 사용
      frame_id = free_frames_.front();
      free_frames_.pop_front();
    } else {
      // 교체자에서 희생자 찾기
      auto victim = replacer_->Evict();
      if (!victim.has_value()) {
        // 모든 페이지가 핀되어 있으면 교체할 수 없음
        return std::nullopt;
      }
      
      frame_id = victim.value();
      auto frame = frames_[frame_id];
      
      // 희생 프레임에 있던 페이지가 더러우면 디스크에 쓰기
      page_id_t old_page_id = INVALID_PAGE_ID;
      for (const auto &entry : page_table_) {
        if (entry.second == frame_id) {
          old_page_id = entry.first;
          break;
        }
      }

      if (old_page_id != INVALID_PAGE_ID) {
        if (frame->is_dirty_) {
          // Use a promise to wait for the write to complete
          auto promise = disk_scheduler_->CreatePromise();
          auto future = promise.get_future();
          disk_scheduler_->Schedule({true, frame->GetDataMut(), old_page_id, std::move(promise)});
          future.wait(); // Wait for the write to complete
        }
        // 페이지 테이블에서 제거
        page_table_.erase(old_page_id);
      }
    }
    
    // 디스크에서 페이지 읽기
    auto frame = frames_[frame_id];
    frame->Reset();
    
    // 디스크에서 데이터 읽기
    //before
    // disk_scheduler_->Schedule({false, frame->GetDataMut(), page_id, {}});
    //after
    // 디스크에서 데이터 읽기 - 완료될 때까지 대기
    auto promise = disk_scheduler_->CreatePromise();
    auto future = promise.get_future();
    disk_scheduler_->Schedule({false, frame->GetDataMut(), page_id, std::move(promise)});
    future.wait(); // 읽기가 완료될 때까지 대기
    
    // 페이지 테이블 업데이트
    page_table_[page_id] = frame_id;
  }
  
  // 프레임 획득 및 핀 카운트 증가
  auto frame = frames_[frame_id];
  frame->pin_count_++;

  // 읽기 락 획득
  frame->rwlatch_.lock_shared();
  
  // LRU-K 교체자에 접근 기록 추가
  replacer_->RecordAccess(frame_id, access_type);
  replacer_->SetEvictable(frame_id, false);  // 페이지가 핀되었으므로 교체 불가능으로 설정
  
  // ReadPageGuard 반환
  return ReadPageGuard(page_id, frame, replacer_, bpm_latch_, disk_scheduler_);
  // UNIMPLEMENTED("TODO(P1): Add implementation.");
}

/**
 * @brief A wrapper around `CheckedWritePage` that unwraps the inner value if it exists.
 *
 * If `CheckedWritePage` returns a `std::nullopt`, **this function aborts the entire process.**
 *
 * This function should **only** be used for testing and ergonomic's sake. If it is at all possible that the buffer pool
 * manager might run out of memory, then use `CheckedPageWrite` to allow you to handle that case.
 *
 * See the documentation for `CheckedPageWrite` for more information about implementation.
 *
 * @param page_id The ID of the page we want to read.
 * @param access_type The type of page access.
 * @return WritePageGuard A page guard ensuring exclusive and mutable access to a page's data.
 */
auto BufferPoolManager::WritePage(page_id_t page_id, AccessType access_type) -> WritePageGuard {
  auto guard_opt = CheckedWritePage(page_id, access_type);

  if (!guard_opt.has_value()) {
    fmt::println(stderr, "\n`CheckedWritePage` failed to bring in page {}\n", page_id);
    std::abort();
  }

  return std::move(guard_opt).value();
}

/**
 * @brief A wrapper around `CheckedReadPage` that unwraps the inner value if it exists.
 *
 * If `CheckedReadPage` returns a `std::nullopt`, **this function aborts the entire process.**
 *
 * This function should **only** be used for testing and ergonomic's sake. If it is at all possible that the buffer pool
 * manager might run out of memory, then use `CheckedPageWrite` to allow you to handle that case.
 *
 * See the documentation for `CheckedPageRead` for more information about implementation.
 *
 * @param page_id The ID of the page we want to read.
 * @param access_type The type of page access.
 * @return ReadPageGuard A page guard ensuring shared and read-only access to a page's data.
 */
auto BufferPoolManager::ReadPage(page_id_t page_id, AccessType access_type) -> ReadPageGuard {
  auto guard_opt = CheckedReadPage(page_id, access_type);

  if (!guard_opt.has_value()) {
    fmt::println(stderr, "\n`CheckedReadPage` failed to bring in page {}\n", page_id);
    std::abort();
  }

  return std::move(guard_opt).value();
}

/**
 * 페이지를 디스크에 쓰기 (안전하지 않은 버전)
 * 
 * @brief Flushes a page's data out to disk unsafely.
 *
 * This function will write out a page's data to disk if it has been modified. If the given page is not in memory, this
 * function will return `false`.
 *
 * You should not take a lock on the page in this function.
 * This means that you should carefully consider when to toggle the `is_dirty_` bit.
 *
 * ### Implementation
 *
 * You should probably leave implementing this function until after you have completed `CheckedReadPage` and
 * `CheckedWritePage`, as it will likely be much easier to understand what to do.
 *
 * TODO(P1): Add implementation
 *
 * @param page_id The page ID of the page to be flushed.
 * @return `false` if the page could not be found in the page table, otherwise `true`.
 */
auto BufferPoolManager::FlushPageUnsafe(page_id_t page_id) -> bool { 
  std::scoped_lock latch(*bpm_latch_);
  
  // 페이지가 버퍼 풀에 없으면 false 반환
  if (page_table_.find(page_id) == page_table_.end()) {
    return false;
  }
  
  frame_id_t frame_id = page_table_[page_id];
  auto frame = frames_[frame_id];
  
  // 페이지가 더러우면 디스크에 쓰기
  if (frame->is_dirty_) {
    disk_scheduler_->Schedule({true, frame->GetDataMut(), page_id, {}});
    frame->is_dirty_ = false;
  }
  
  return true;
  // UNIMPLEMENTED("TODO(P1): Add implementation."); 
}

/**
 * 페이지를 디스크에 쓰기 (안전한 버전)
 * 
 * @brief Flushes a page's data out to disk safely.
 *
 * This function will write out a page's data to disk if it has been modified. If the given page is not in memory, this
 * function will return `false`.
 *
 * You should take a lock on the page in this function to ensure that a consistent state is flushed to disk.
 *
 * ### Implementation
 *
 * You should probably leave implementing this function until after you have completed `CheckedReadPage`,
 * `CheckedWritePage`, and `Flush` in the page guards, as it will likely be much easier to understand what to do.
 *
 * TODO(P1): Add implementation
 *
 * @param page_id The page ID of the page to be flushed.
 * @return `false` if the page could not be found in the page table, otherwise `true`.
 */
auto BufferPoolManager::FlushPage(page_id_t page_id) -> bool { 
  std::scoped_lock latch(*bpm_latch_);
  
  // 페이지가 버퍼 풀에 없으면 false 반환
  if (page_table_.find(page_id) == page_table_.end()) {
    return false;
  }
  
  frame_id_t frame_id = page_table_[page_id];
  auto frame = frames_[frame_id];
  
  // 쓰기 락 획득
  std::scoped_lock frame_latch(frame->rwlatch_);
  
  // 페이지가 더러우면 디스크에 쓰기
  if (frame->is_dirty_) {
    auto promise = disk_scheduler_->CreatePromise();
    auto future = promise.get_future();
    disk_scheduler_->Schedule({true, frame->GetDataMut(), page_id, std::move(promise)});
    future.wait(); // 쓰기가 완료될 때까지 대기
    frame->is_dirty_ = false;
  }
  
  return true;
}

/** 
 * 모든 페이지를 디스크에 쓰기 (안전하지 않은 버전)
 * 
 * @brief Flushes all page data that is in memory to disk unsafely.
 *
 * You should not take locks on the pages in this function.
 * This means that you should carefully consider when to toggle the `is_dirty_` bit.
 *
 * ### Implementation
 *
 * You should probably leave implementing this function until after you have completed `CheckedReadPage`,
 * `CheckedWritePage`, and `FlushPage`, as it will likely be much easier to understand what to do.
 *
 * TODO(P1): Add implementation
 */
void BufferPoolManager::FlushAllPagesUnsafe() { 
  std::scoped_lock latch(*bpm_latch_);
  
  for (const auto &entry : page_table_) {
    page_id_t page_id = entry.first;
    frame_id_t frame_id = entry.second;
    auto frame = frames_[frame_id];
    
    // 페이지가 더러우면 디스크에 쓰기
    if (frame->is_dirty_) {
      auto promise = disk_scheduler_->CreatePromise();
      auto future = promise.get_future();
      disk_scheduler_->Schedule({true, frame->GetDataMut(), page_id, std::move(promise)});
      future.wait(); // 쓰기가 완료될 때까지 대기
      frame->is_dirty_ = false;
    }
  }
}

/**
 * 모든 페이지를 디스크에 쓰기 (안전한 버전)
 * 
 * @brief Flushes all page data that is in memory to disk safely.
 *
 * You should take locks on the pages in this function to ensure that a consistent state is flushed to disk.
 *
 * ### Implementation
 *
 * You should probably leave implementing this function until after you have completed `CheckedReadPage`,
 * `CheckedWritePage`, and `FlushPage`, as it will likely be much easier to understand what to do.
 *
 * TODO(P1): Add implementation
 */
void BufferPoolManager::FlushAllPages() { 
  std::scoped_lock latch(*bpm_latch_);
  
  for (const auto &entry : page_table_) {
    page_id_t page_id = entry.first;
    frame_id_t frame_id = entry.second;
    auto frame = frames_[frame_id];
    
    // 쓰기 락 획득
    std::scoped_lock frame_latch(frame->rwlatch_);
    
    // 페이지가 더러우면 디스크에 쓰기
    if (frame->is_dirty_) {
      auto promise = disk_scheduler_->CreatePromise();
      auto future = promise.get_future();
      disk_scheduler_->Schedule({true, frame->GetDataMut(), page_id, std::move(promise)});
      future.wait(); // 쓰기가 완료될 때까지 대기
      frame->is_dirty_ = false;
    }
  }
}

/**
 * 페이지의 핀 카운트를 반환합니다.
 * 
 * @brief Retrieves the pin count of a page. If the page does not exist in memory, return `std::nullopt`.
 *
 * This function is thread safe. Callers may invoke this function in a multi-threaded environment where multiple threads
 * access the same page.
 *
 * This function is intended for testing purposes. If this function is implemented incorrectly, it will definitely cause
 * problems with the test suite and autograder.
 *
 * # Implementation
 *
 * We will use this function to test if your buffer pool manager is managing pin counts correctly. Since the
 * `pin_count_` field in `FrameHeader` is an atomic type, you do not need to take the latch on the frame that holds the
 * page we want to look at. Instead, you can simply use an atomic `load` to safely load the value stored. You will still
 * need to take the buffer pool latch, however.
 *
 * Again, if you are unfamiliar with atomic types, see the official C++ docs
 * [here](https://en.cppreference.com/w/cpp/atomic/atomic).
 *
 * TODO(P1): Add implementation
 *
 * @param page_id The page ID of the page we want to get the pin count of.
 * @return std::optional<size_t> The pin count if the page exists, otherwise `std::nullopt`.
 */
auto BufferPoolManager::GetPinCount(page_id_t page_id) -> std::optional<size_t> {
  std::scoped_lock latch(*bpm_latch_);
  
  // 페이지가 버퍼 풀에 없으면 std::nullopt 반환
  if (page_table_.find(page_id) == page_table_.end()) {
    return std::nullopt;
  }
  
  frame_id_t frame_id = page_table_[page_id];
  auto frame = frames_[frame_id];
  
  // 핀 카운트 반환
  return frame->pin_count_.load();
  // UNIMPLEMENTED("TODO(P1): Add implementation.");
}

}  // namespace bustub
