//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// trie_store.cpp
//
// Identification: src/primer/trie_store.cpp
//
// Copyright (c) 2015-2025, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "primer/trie_store.h"
#include "common/exception.h"

namespace bustub {

// GET - failed version
// Q. why it failed test?
// A. 일반적인 read under concurrent condition은 pass 했는데, mixedConcurrentTest는 fail 함. 
//    이 코드는 read 함수인데 root node에 mutex 락을 걸어버리기 때문에, 성능 효율도 안좋은데, 
//    다른 스레드가 root_를 바꿔버리면, get() 도중 원본데이터가 바뀌어서 망함
//    그래서 새 코드에선 local_trie = root_ 로 복사본 만들어서 락 풀어도 안전하게 함. 진짜 기본 중에 기본
// template <class T>
// auto TrieStore::Get(std::string_view key) -> std::optional<ValueGuard<T>> {
//   root_lock_.lock();
//   Trie *trie = &root_;
//   root_lock_.unlock();

//   const T *value = trie->Get<T>(key);
//   if (value) {
//     return {ValueGuard<T>(*trie, *value)};
//   }

//   return std::nullopt;
// }

  // GET - correct version 
  // Q. how it fixed problem? whats different from failed version?
  // A. 1. 기존엔 root_에 직접 락을 걸었다면, 이젠 Trie 복사본 만든거에 락검 
  //    2. 이전엔 수동으로 lock(), unlock()했는데, 이러면 문제가 실수로 unlock() 빼먹으면 데드락 오지게 발생함. 새 코드는 std::lock_guard<std::mutex> 써서 스코프 벗어나면 자동으로 락 풀리게 함. 그니까 예외 발생해도 락 자동으로 풀려서 안전함. 
  //       std::lock_guard는 RAII패턴씀: 생성자에서 .lock()걸고 소멸자에서 .unlock()함. 그래서 명시적으로 .lock(), .unlock() 호출 할 필요 없음. {} 벗어나서 local 변수 사라질 떄 자동으로 lock도 사라짐.

  // Q. read할 때도 원본 trie의 복사본을 만들어야 할까? 그냥 읽으면 안됨?
  // A. ㄴㄴ. 그러면 write할 때 원본 데이터 바꿔버려서 데이터 오염될 수 있음. 
  template <class T>
  auto TrieStore::Get(std::string_view key) -> std::optional<ValueGuard<T>> {
    Trie local_trie;
    { // Q. 왜 따로 {} block 안에 감싸지?
      // A. lock_guard에 감싸진 mutex가 {} 벗어나면 자동으로 해제되거든. 이 {} 안에 있으면 root_lock_이 걸리는거임. 
      std::lock_guard<std::mutex> lock(root_lock_); //std::lock_guard<std::mutex> 써서 스코프 벗어나면 자동으로 락 풀리게 함.  그니까 예외 발생해도 락 자동으로 풀려서 안전함. 
                                                    //근데 이게 tree_store.h에 있던 mutex락을 lock_guard로 감싸는거 같은데, 이게 local_tree에 적용되는 부분이 어디있지?
      local_trie = root_;  // 복사본 생성. 근데 deep copy는 아님. 그럼 Trie size가 1gib면 매 get()마다 1gib를 복사한다는거니까.. 너무 비효율 
                           // shallow copy. 실제 원본 trie를 복사해서 거기에 put(), remove() 하는건 Trie.Put(), Remove() 코드 도입부에 root = root_->Clone();으로 함. 
                           // root_->Clone();은 deepcopy가 아님. 


                           // Q. Trie local_trie = root_;  // 복사본 생성 이라는데, 저 = 연산자를 쓰면 새 local_trie 포인터에 기존 root_의 데이터를 복사해서 넣어줘? =는 그냥 포인터를 저 원본 데이터에 가르키게 하는거 아냐? 따로 operator= 선언을 하지 않은 이상? 근데 trie.h랑 trie.cpp를 봐도 operator=는 없는데?
                           // A. 컴파일러가 자동으로 만들어준다고 함. = 연산자 쓰면 내부 데이터 복사해서 넣어주나봄. 

                           // Q. 복사본 만들 때 deepcopy 하면 너무 heavy하지 않음? Trie size가 1GiB면 매 Get(), Put(), Remove() 마다 1GiB 를 deepcopy 하면 너무... 비효율적이잖아?
                           // A. copy-on-write 패턴 쓰는데, deepcopy 안하고 딱 root_note만 갈아낌. trie.cpp에 Clone() 보면,
                           // = operator를 컴파일러가 만들어주는데, deepcopy가 아니라 포인터 자체만 복사되고 참조카운터가 증가한다던데?
    }
  
    const T *value = local_trie.Get<T>(key);
    if (value) {
      return {ValueGuard<T>(local_trie, *value)};
    }
  
    return std::nullopt;
  }



// PUT - failed version
// Q. why it failed test?
// A. lock 범위가 불필요하게 너무 김. trie->Put() 할 때에도 write_lock.lock()을 잡는 중. 
//    근데 이 연산은 원본 Trie 안건드리고 새로운 trie에 Put() 하는거라 락 걸 필요 없음.
// template <class T>
// void TrieStore::Put(std::string_view key, T value) {
//   write_lock_.lock();

//   Trie *trie = &root_;
//   Trie new_trie = trie->Put(key, std::move(value));

//   root_lock_.lock();
//   root_ = new_trie;
//   root_lock_.unlock();

//   write_lock_.unlock();
// }

  // Put - correct version 
  // Q. how it fixed problem? whats different from failed version?  
  // A. 먼저 mutex를 lock_guard를 써서 {} 벗어나면 자동으로 락 풀리게 함. 그니까 예외 발생해도 락 자동으로 풀려서 안전함. 
  template <class T>
  void TrieStore::Put(std::string_view key, T value) {
    std::lock_guard<std::mutex> write_lock(write_lock_); //Q. 이렇게만 mutex였던 write_lock_을 lock_guard로 감싸는데, .lock(), .unlock()도 없는데 어떻게 코드가 처리되는거지?
    
    Trie local_trie;
    { //Q. 왜 {}로 감싸는거지?
      // A. lock_guard에 감싸진 mutex가 {} 벗어나면 자동으로 해제되거든. 이 {} 안에 있으면 root_lock_이 걸리는거임. 
      std::lock_guard<std::mutex> root_lock(root_lock_); //root_lock_에 락 걸어서 {} 스코프 내부에 원본데이터 복사 끝날 때 까지 lock을 안품 
      local_trie = root_; //원본 데이터를 1. 단 복사해 
    } // root_lock_ 자동 해제
    
    // COW 패턴이라 Put()이 락 없이 수행된다! 
    // 이 함수는 실제로 Trie 구조 따라 내려가면서 노드 생성하고 복사하는 무거운 연산임. 이거 하는 동안 락 잡고 있으면 다른 스레드들 다 ㅈ됨.이 함수는 실제로 Trie 구조 따라 내려가면서 노드 생성하고 복사하는 무거운 연산임. 이거 하는 동안 락 잡고 있으면 다른 스레드들 다 쳐짐.
    // 복사본에 작업하니까 안전함: local_trie = root_ 하고 락 풀어도 local_trie는 그냥 복사본이라 원본이랑 관계 없음. 다른 스레드가 root_를 바꿔도 이 복사본에는 영향 없음.
    Trie new_trie = local_trie.Put(key, std::move(value)); //복사한 Trie에 Put()을 해. 근데 Put()할 땐 어짜피 새로 만든 Trie에 하는거라 락 걸 필요 없음 
    
    { // 이 블록 안에서만 root_lock_ 유지
      std::lock_guard<std::mutex> root_lock(root_lock_); //새로 trie를 원본 trie에 덮어쓰기 위해 다시 root_lock_을 검
      root_ = new_trie; //원본 Trie에 덮어 써 
    } // root_lock_ 자동 해제
  }
  


// REMOVE - failed version
// Q. why it failed test?
// A. Remove()할 때 lock걸면 너무 오래 lock 거니까, 복사본 만든 다음, 복사본 remove 할 때 lock 안걸어도 됨. 그리고 원본 데이터 덮어쓸 때나 lock걸지. 
// void TrieStore::Remove(std::string_view key) {
//   write_lock_.lock();

//   Trie *trie = &root_;
//   Trie new_trie = trie->Remove(key);

//   root_lock_.lock();
//   root_ = new_trie;
//   root_lock_.unlock();

//   write_lock_.unlock();
// }

  // REMOVE - correct version 
  // goal: You will need to ensure there is only one writer at a time. Think of how you can achieve this.
  // Q. how it fixed problem? whats different from failed version?  
  void TrieStore::Remove(std::string_view key) {
    std::lock_guard<std::mutex> write_lock(write_lock_);
    
    Trie local_trie;
    {
      std::lock_guard<std::mutex> root_lock(root_lock_);
      local_trie = root_;
    }
    
    Trie new_trie = local_trie.Remove(key);
    
    { // 이 블록 안에서만 root_lock_ 유지
      std::lock_guard<std::mutex> root_lock(root_lock_);
      root_ = new_trie;
    } // root_lock_ 자동 해제
  }

// Below are explicit instantiation of template functions.

template auto TrieStore::Get(std::string_view key) -> std::optional<ValueGuard<uint32_t>>;
template void TrieStore::Put(std::string_view key, uint32_t value);

template auto TrieStore::Get(std::string_view key) -> std::optional<ValueGuard<std::string>>;
template void TrieStore::Put(std::string_view key, std::string value);

// If your solution cannot compile for non-copy tests, you can remove the below lines to get partial score.

using Integer = std::unique_ptr<uint32_t>;

template auto TrieStore::Get(std::string_view key) -> std::optional<ValueGuard<Integer>>;
template void TrieStore::Put(std::string_view key, Integer value);

template auto TrieStore::Get(std::string_view key) -> std::optional<ValueGuard<MoveBlocked>>;
template void TrieStore::Put(std::string_view key, MoveBlocked value);

}  // namespace bustub
