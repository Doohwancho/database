//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// buffer_pool_manager_test.cpp
//
// Identification: test/buffer/buffer_pool_manager_test.cpp
//
// Copyright (c) 2015-2025, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <cstdio>
#include <filesystem>

#include "buffer/buffer_pool_manager.h"
#include "gtest/gtest.h"
#include "storage/page/page_guard.h"

namespace bustub {

static std::filesystem::path db_fname("test.bustub");

// The number of frames we give to the buffer pool.
const size_t FRAMES = 10;
// Note that this test assumes you are using the an LRU-K replacement policy.
const size_t K_DIST = 5;

void CopyString(char *dest, const std::string &src) {
  BUSTUB_ENSURE(src.length() + 1 <= BUSTUB_PAGE_SIZE, "CopyString src too long");
  snprintf(dest, BUSTUB_PAGE_SIZE, "%s", src.c_str());
}

TEST(BufferPoolManagerTest, VeryBasicTest) {
  // A very basic test.

  auto disk_manager = std::make_shared<DiskManager>(db_fname);
  auto bpm = std::make_shared<BufferPoolManager>(FRAMES, disk_manager.get(), K_DIST);

  const page_id_t pid = bpm->NewPage();
  const std::string str = "Hello, world!";

  // Check `WritePageGuard` basic functionality.
  {
    auto guard = bpm->WritePage(pid); 
    CopyString(guard.GetDataMut(), str); //여기서 페이지 데이터 수정 
    EXPECT_STREQ(guard.GetData(), str.c_str()); 
  } // 여기서 guard 소멸되면서 자동으로 락 풀리고 핀 카운트 감소

  // Check `ReadPageGuard` basic functionality.
  {
    const auto guard = bpm->ReadPage(pid);
    EXPECT_STREQ(guard.GetData(), str.c_str());
  }

  // Check `ReadPageGuard` basic functionality (again).
  {
    const auto guard = bpm->ReadPage(pid);
    EXPECT_STREQ(guard.GetData(), str.c_str());
  }

  ASSERT_TRUE(bpm->DeletePage(pid));
}

TEST(BufferPoolManagerTest, PagePinEasyTest) {
  auto disk_manager = std::make_shared<DiskManager>(db_fname);
  auto bpm = std::make_shared<BufferPoolManager>(2, disk_manager.get(), 5);

  const page_id_t pageid0 = bpm->NewPage();
  const page_id_t pageid1 = bpm->NewPage();

  const std::string str0 = "page0";
  const std::string str1 = "page1";
  const std::string str0updated = "page0updated";
  const std::string str1updated = "page1updated";

  //step1) 첫 번째 블록에서 pageid0, pageid1에 데이터를 씁니다.
  {
    auto page0_write_opt = bpm->CheckedWritePage(pageid0);
    ASSERT_TRUE(page0_write_opt.has_value());
    auto page0_write = std::move(page0_write_opt.value());
    CopyString(page0_write.GetDataMut(), str0);

    auto page1_write_opt = bpm->CheckedWritePage(pageid1);
    ASSERT_TRUE(page1_write_opt.has_value());
    auto page1_write = std::move(page1_write_opt.value());
    CopyString(page1_write.GetDataMut(), str1);

    ASSERT_EQ(1, bpm->GetPinCount(pageid0));
    ASSERT_EQ(1, bpm->GetPinCount(pageid1));

    const auto temp_page_id1 = bpm->NewPage();
    const auto temp_page1_opt = bpm->CheckedReadPage(temp_page_id1);
    ASSERT_FALSE(temp_page1_opt.has_value());

    const auto temp_page_id2 = bpm->NewPage();
    const auto temp_page2_opt = bpm->CheckedWritePage(temp_page_id2);
    ASSERT_FALSE(temp_page2_opt.has_value());

    ASSERT_EQ(1, bpm->GetPinCount(pageid0));
    page0_write.Drop();
    ASSERT_EQ(0, bpm->GetPinCount(pageid0));

    ASSERT_EQ(1, bpm->GetPinCount(pageid1));
    page1_write.Drop();
    ASSERT_EQ(0, bpm->GetPinCount(pageid0));
  } // 첫 번째 블록이 끝나면 page0_write와 page1_write가 소멸됩니다.

  //step2) 두 개의 새 페이지를 만들려고 시도합니다(버퍼 풀 크기는 2).
  {
    const auto temp_page_id1 = bpm->NewPage();
    const auto temp_page1_opt = bpm->CheckedReadPage(temp_page_id1);
    ASSERT_TRUE(temp_page1_opt.has_value());

    const auto temp_page_id2 = bpm->NewPage();
    const auto temp_page2_opt = bpm->CheckedWritePage(temp_page_id2);
    ASSERT_TRUE(temp_page2_opt.has_value());

    ASSERT_FALSE(bpm->GetPinCount(pageid0).has_value());
    ASSERT_FALSE(bpm->GetPinCount(pageid1).has_value());
  } //두 번째 블록에서 두 개의 새 페이지를 성공적으로 만듭니다.

  //step3) 세 번째 블록에서 pageid0에 다시 접근하려고 할 때 데이터가 유실됩니다.
  {
    auto page0_write_opt = bpm->CheckedWritePage(pageid0);
    ASSERT_TRUE(page0_write_opt.has_value());
    auto page0_write = std::move(page0_write_opt.value());
    EXPECT_STREQ(page0_write.GetData(), str0.c_str());  // error!) 
                                                        // page0_write.GetData()
                                                        //   Which is: ""
                                                        // str0.c_str()
                                                        //   Which is: "page0"
    CopyString(page0_write.GetDataMut(), str0updated);

    auto page1_write_opt = bpm->CheckedWritePage(pageid1);
    ASSERT_TRUE(page1_write_opt.has_value());
    auto page1_write = std::move(page1_write_opt.value());
    EXPECT_STREQ(page1_write.GetData(), str1.c_str());
    CopyString(page1_write.GetDataMut(), str1updated);

    ASSERT_EQ(1, bpm->GetPinCount(pageid0));
    ASSERT_EQ(1, bpm->GetPinCount(pageid1));
  }

  ASSERT_EQ(0, bpm->GetPinCount(pageid0));
  ASSERT_EQ(0, bpm->GetPinCount(pageid1));

  {
    auto page0_read_opt = bpm->CheckedReadPage(pageid0);
    ASSERT_TRUE(page0_read_opt.has_value());
    const auto page0_read = std::move(page0_read_opt.value());
    EXPECT_STREQ(page0_read.GetData(), str0updated.c_str());

    auto page1_read_opt = bpm->CheckedReadPage(pageid1);
    ASSERT_TRUE(page1_read_opt.has_value());
    const auto page1_read = std::move(page1_read_opt.value());
    EXPECT_STREQ(page1_read.GetData(), str1updated.c_str());

    ASSERT_EQ(1, bpm->GetPinCount(pageid0));
    ASSERT_EQ(1, bpm->GetPinCount(pageid1));
  }

  ASSERT_EQ(0, bpm->GetPinCount(pageid0));
  ASSERT_EQ(0, bpm->GetPinCount(pageid1));

  remove(db_fname);
  remove(disk_manager->GetLogFileName());
}

TEST(BufferPoolManagerTest, PagePinMediumTest) {
  auto disk_manager = std::make_shared<DiskManager>(db_fname);
  auto bpm = std::make_shared<BufferPoolManager>(FRAMES, disk_manager.get(), K_DIST);

  // Scenario: The buffer pool is empty. We should be able to create a new page.
  const auto pid0 = bpm->NewPage();
  auto page0 = bpm->WritePage(pid0);

  // Scenario: Once we have a page, we should be able to read and write content.
  const std::string hello = "Hello";
  CopyString(page0.GetDataMut(), hello);
  EXPECT_STREQ(page0.GetData(), hello.c_str());

  page0.Drop();

  // Create a vector of unique pointers to page guards, which prevents the guards from getting destructed.
  std::vector<WritePageGuard> pages;

  //step1) buffer pool 가득 채우기 
  // Scenario: We should be able to create new pages until we fill up the buffer pool.
  for (size_t i = 0; i < FRAMES; i++) {
    const auto pid = bpm->NewPage();
    auto page = bpm->WritePage(pid);
    pages.push_back(std::move(page));
  }

  // Scenario: All of the pin counts should be 1.
  for (const auto &page : pages) {
    const auto pid = page.GetPageId();
    EXPECT_EQ(1, bpm->GetPinCount(pid));
  }

  // Scenario: Once the buffer pool is full, we should not be able to create any new pages.
  for (size_t i = 0; i < FRAMES; i++) {
    const auto pid = bpm->NewPage();
    const auto fail = bpm->CheckedWritePage(pid);
    ASSERT_FALSE(fail.has_value());
  }

  //step2) 반절 언핀 후 새 페이지 할당 → LRU 확인
  // Scenario: Drop the first 5 pages to unpin them.
  for (size_t i = 0; i < FRAMES / 2; i++) {
    const auto pid = pages[0].GetPageId();
    EXPECT_EQ(1, bpm->GetPinCount(pid));
    pages.erase(pages.begin());
    EXPECT_EQ(0, bpm->GetPinCount(pid));
  }

  // Scenario: All of the pin counts of the pages we haven't dropped yet should still be 1.
  for (const auto &page : pages) {
    const auto pid = page.GetPageId();
    EXPECT_EQ(1, bpm->GetPinCount(pid));
  }

  // Scenario: After unpinning pages {1, 2, 3, 4, 5}, we should be able to create 4 new pages and bring them into
  // memory. Bringing those 4 pages into memory should evict the first 4 pages {1, 2, 3, 4} because of LRU.
  for (size_t i = 0; i < ((FRAMES / 2) - 1); i++) {
    const auto pid = bpm->NewPage();
    auto page = bpm->WritePage(pid);
    pages.push_back(std::move(page));
  }

  // Scenario: There should be one frame available, and we should be able to fetch the data we wrote a while ago.
  {
    //step3) 데이터 접근성 테스트
    const auto original_page = bpm->ReadPage(pid0);
    EXPECT_STREQ(original_page.GetData(), hello.c_str());
  }

  // Scenario: Once we unpin page 0 and then make a new page, all the buffer pages should now be pinned. Fetching page 0
  // again should fail.
  const auto last_pid = bpm->NewPage();
  const auto last_page = bpm->ReadPage(last_pid);

  const auto fail = bpm->CheckedReadPage(pid0);
  ASSERT_FALSE(fail.has_value());

  // Shutdown the disk manager and remove the temporary file we created.
  disk_manager->ShutDown();
  remove(db_fname);
}

// 멀티스레드 환경에서 읽기/쓰기 락 테스트 
TEST(BufferPoolManagerTest, PageAccessTest) {
  const size_t rounds = 50;

  auto disk_manager = std::make_shared<DiskManager>(db_fname);
  auto bpm = std::make_shared<BufferPoolManager>(1, disk_manager.get(), K_DIST);

  const auto pid = bpm->NewPage();
  char buf[BUSTUB_PAGE_SIZE];

  //step1) 쓰기 스레드 (다른 스레드에서 계속 쓰기)
  auto thread = std::thread([&]() {
    // The writer can keep writing to the same page.
    for (size_t i = 0; i < rounds; i++) {
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
      auto guard = bpm->WritePage(pid);
      CopyString(guard.GetDataMut(), std::to_string(i));
    }
  });


  for (size_t i = 0; i < rounds; i++) {
    // Wait for a bit before taking the latch, allowing the writer to write some stuff.
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    //step2) 읽기 스레드 (읽는 동안 데이터 불변 확인)
    // While we are reading, nobody should be able to modify the data.
    const auto guard = bpm->ReadPage(pid);

    // Save the data we observe.
    memcpy(buf, guard.GetData(), BUSTUB_PAGE_SIZE);

    // Sleep for a bit. If latching is working properly, nothing should be writing to the page.
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // Check that the data is unmodified.
    EXPECT_STREQ(guard.GetData(), buf);
  }

  thread.join();
}

//여러 스레드가 동시에 같은 페이지 경합할 때 성능/안정성 테스트
TEST(BufferPoolManagerTest, ContentionTest) {
  auto disk_manager = std::make_shared<DiskManager>(db_fname);
  auto bpm = std::make_shared<BufferPoolManager>(FRAMES, disk_manager.get(), K_DIST);

  const size_t rounds = 100000;

  const auto pid = bpm->NewPage();

  //4개 스레드가 동시에 같은 페이지에 10만 번 쓰기 경합
  auto thread1 = std::thread([&]() {
    for (size_t i = 0; i < rounds; i++) {
      auto guard = bpm->WritePage(pid);
      CopyString(guard.GetDataMut(), std::to_string(i));
    }
  });

  auto thread2 = std::thread([&]() {
    for (size_t i = 0; i < rounds; i++) {
      auto guard = bpm->WritePage(pid);
      CopyString(guard.GetDataMut(), std::to_string(i));
    }
  });

  auto thread3 = std::thread([&]() {
    for (size_t i = 0; i < rounds; i++) {
      auto guard = bpm->WritePage(pid);
      CopyString(guard.GetDataMut(), std::to_string(i));
    }
  });

  auto thread4 = std::thread([&]() {
    for (size_t i = 0; i < rounds; i++) {
      auto guard = bpm->WritePage(pid);
      CopyString(guard.GetDataMut(), std::to_string(i));
    }
  });

  thread3.join();
  thread2.join();
  thread4.join();
  thread1.join();
}

// 데드락 상황 테스트 (페이지 대기 중 다른 락 획득 가능한지)
// 관전 포인트: 버퍼 풀 전체 락(bpm_latch_)과 개별 페이지 락을 분리했는지 확인하는 테스트임. 전체 락을 오래 들고 있으면 데드락 발생함 ㅉㅉ
TEST(BufferPoolManagerTest, DeadlockTest) {
  auto disk_manager = std::make_shared<DiskManager>(db_fname);
  auto bpm = std::make_shared<BufferPoolManager>(FRAMES, disk_manager.get(), K_DIST);

  const auto pid0 = bpm->NewPage();
  const auto pid1 = bpm->NewPage();

  // 메인 스레드가 페이지0 락 보유한 채로 대기
  auto guard0 = bpm->WritePage(pid0);

  // A crude way of synchronizing threads, but works for this small case.
  std::atomic<bool> start = false;

  //자식 스레드가 페이지0 쓰기 요청해서 대기 중
  auto child = std::thread([&]() {
    // Acknowledge that we can begin the test.
    start.store(true);

    // Attempt to write to page 0.
    const auto guard0 = bpm->WritePage(pid0);
  });

  // Wait for the other thread to begin before we start the test.
  while (!start.load()) {
  }

  // lock 계속 걸리게 1초동안 내비둠. 그 사이에 다른 애 접근시킴 
  // Make the other thread wait for a bit.
  // This mimics the main thread doing some work while holding the write latch on page 0.
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  // If your latching mechanism is incorrect, the next line of code will deadlock.
  // Think about what might happen if you hold a certain "all-encompassing" latch for too long...


  // 메인 스레드가 페이지1 락 획득 (이게 됨?)
  // While holding page 0, take the latch on page 1.
  const auto guard1 = bpm->WritePage(pid1);

  // Let the child thread have the page 0 since we're done with it.
  guard0.Drop();

  child.join();
}

//핀된 페이지는 교체 대상에서 제외되는지 테스트
TEST(BufferPoolManagerTest, EvictableTest) {
  // Test if the evictable status of a frame is always correct.
  const size_t rounds = 1000;
  const size_t num_readers = 8;

  auto disk_manager = std::make_shared<DiskManager>(db_fname);
  // Only allocate 1 frame of memory to the buffer pool manager.
  auto bpm = std::make_shared<BufferPoolManager>(1, disk_manager.get(), K_DIST);

  for (size_t i = 0; i < rounds; i++) {
    std::mutex mutex;
    std::condition_variable cv;

    // This signal tells the readers that they can start reading after the main thread has already taken the read latch.
    bool signal = false;

    // 1프레임 버퍼풀에 페이지 로드하고 핀하기
    // This page will be loaded into the only available frame.
    const auto winner_pid = bpm->NewPage();
    // We will attempt to load this page into the occupied frame, and it should fail every time.
    const auto loser_pid = bpm->NewPage();

    std::vector<std::thread> readers;
    for (size_t j = 0; j < num_readers; j++) {
      readers.emplace_back([&]() {
        std::unique_lock<std::mutex> lock(mutex);

        // Wait until the main thread has taken a read latch on the page.
        while (!signal) {
          cv.wait(lock);
        }

        // 여러 스레드가 동시에 같은 페이지 읽기 접근
        // Read the page in shared mode.
        const auto read_guard = bpm->ReadPage(winner_pid);

        // 새 페이지 로드 시도 실패 확인 (모든 프레임 핀됨)
        // Since the only frame is pinned, no thread should be able to bring in a new page.
        ASSERT_FALSE(bpm->CheckedReadPage(loser_pid).has_value());
      });
    }

    std::unique_lock<std::mutex> lock(mutex);

    if (i % 2 == 0) {
      // Take the read latch on the page and pin it.
      auto read_guard = bpm->ReadPage(winner_pid);

      // Wake up all of the readers.
      signal = true;
      cv.notify_all();
      lock.unlock();

      // Allow other threads to read.
      read_guard.Drop();
    } else {
      // Take the read latch on the page and pin it.
      auto write_guard = bpm->WritePage(winner_pid);

      // Wake up all of the readers.
      signal = true;
      cv.notify_all();
      lock.unlock();

      // Allow other threads to read.
      write_guard.Drop();
    }

    for (size_t i = 0; i < num_readers; i++) {
      readers[i].join();
    }
  }
}

}  // namespace bustub
