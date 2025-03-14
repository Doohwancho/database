//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// trie.h
//
// Identification: src/include/primer/trie.h
//
// Copyright (c) 2015-2025, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <algorithm>
#include <cstddef>
#include <future>  // NOLINT
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <iostream>  // cout, endl 사용을 위해 (for debugging, print문)
#include <functional>  // std::function 사용을 위해 (for debugging, print문)

/*
구조 

Trie에 각 노드는 
1. TrieNode
2. TrieNodeWithValue

이렇게 나뉜다. 

1. TrieNode는 map인데 key: char, value: chlidren_nodes이고, 
2. TrieNodeWithValue는 children_nodes가 있는 상황에서 std::shared_ptr<T> value_;도 있음. 데이터를 저장하는 변수가 따로 또 있음.
  - 아마 insert("hello"); 인 경우도 있는데, insert("hello", 3); 이런 경우도 있어서 이런 케이스를 위해서 TrieNodeWithValue를 만든 것 같음. 

*/


namespace bustub {
// Q. namespace bustub 라면, 이 안에있는 클래스를 사용할 때, bustub::Trie 이런식으로 사용해야 하나?
// A. 아님. namespace 밖에서 쓸 땐 bustub::Trie 이런식으로 사용해야 하는데, namespace 안에서 쓰는거면 그냥 Trie로 써도 됨. 편해서 씀. 

class MoveBlocked {
 //Q. 이 클래스는 어디에 쓰려고 만든거지? 딱히 Trie()나 TrieNode() 에서 참조하고 있지는 않은 것 같은데 
 //A. 테스트할 때 쓰이는 객체임. 움직이지 말아야 할 객체를 막을 때 쓰임. 
 // 이런식으로 테스트에서 쓸 수 있음
 // std::promise<int> p;
 // std::future<int> f = p.get_future();

 // 이렇게 MoveBlocked 객체 생성
 // MoveBlocked mb(std::move(f));

 // 이제 mb는 이동 생성자/대입 연산자가 블록되어 있어서
 // 다른 함수에 값으로 전달할 수 없음
 // 이런 제약 있는 객체가 Trie에서 제대로 처리되는지 테스트할 수 있음

 // 만약 이렇게 하면 컴파일 에러남
 // MoveBlocked mb2 = mb; // 에러: 복사 생성자 삭제됨
 public:
  explicit MoveBlocked(std::future<int> wait) : wait_(std::move(wait)) {}
  //Q. explicit이 뭐지? 왜 쓴거지?
  //A. 암시적 변환 막는 키워드임. 이거 없으면 컴파일러가 자동으로 타입 바꿔버릴 수 있는데 그거 방지하려고 씀. MoveBlocked(future<int>) 생성자를 호출할 때 반드시 명시적으로 호출하라는 뜻.
  //Q. std::future은 뭐지? 왜 쓴거지?
  //A. std::future는 비동기 연산의 결과를 받는 컨테이너임. 나중에 계산될 값을 담는 그릇이라고 생각하면 됨. 
  //   여기선 이동 생성자/대입 연산자 테스트하는데 쓰려고 만든거같음
  //Q. std::move()는 뭐지? 왜 쓴거지?
  //A. 객체를 이동시키는 함수임. lvalue를 rvalue로 바꿔서 복사 안하고 이동시킬 수 있게 해줌. 성능 때문에 쓰는거. 특히 큰 객체 복사하면 느리니까.

  MoveBlocked(const MoveBlocked &) = delete;
  //Q. 파라미터에 MoveBlocked & 이게 c로 치면 reference인가? const &MoveBlocked와 같은건가?
  //A. ㅇㅇ 레퍼런스 맞다. c++에는 &를 변수 뒤에 한칸 띄고 붙이나 봄 
  //Q. = delete;는 뭐지? 소멸자를 다른 방식으로 표현한건가? 만약 그렇다면 왜 ~MoveBlocked() 하지 않았지? RAII와 관련있나?
  //A. 함수 삭제한다는 뜻. 이 함수 쓰면 컴파일 에러남. 여기선 복사 생성자/대입 연산자 막으려고 쓴거.

  MoveBlocked(MoveBlocked &&that) noexcept {
    //Q. &&that에서 && 두번도 쓸 수 있나?
    //A. 쓸 수 있음. 이건 rvalue 레퍼런스임. C++11에서 도입된 문법. 임시 객체나 std::move로 이동된 객체 참조할 때 씀.
    //Q. noexcept 문법은 뭐지?
    //A. 이 함수가 예외 안 던진다고 컴파일러에게 알려주는거. 최적화랑 안전성 때문에 씀.
    if (!that.waited_) {
      that.wait_.get();
    }
    that.waited_ = waited_ = true;
  }

  auto operator=(const MoveBlocked &) -> MoveBlocked & = delete;
  // Q. operator= 에서 =가 메서드 명으로 될 수 있나?
  // A. ㅇㅇ. 연산자 오버로딩이라고 함.  C++에선 =, +, - 같은 연산자도 함수로 정의 가능.
  // Q. return type에 MoveBlocked &에서 reference type을 return type으로 할 수 있나?
  // A. ㅇㅇ. 연산자 오버로딩은 객체 자체를 반환하는게 아니라 참조만 반환한다고 한다. 
  // Q. = delete는 뭐지?
  // A. 함수 삭제한다는 뜻. 여기선 복사 생성자/대입 연산자 막으려고 쓴거.

  auto operator=(MoveBlocked &&that) noexcept -> MoveBlocked & {
    if (!that.waited_) {
      that.wait_.get();
    }
    that.waited_ = waited_ = true;
    return *this;
  }

  bool waited_{false};
  std::future<int> wait_;
};

class TrieNode {
 public:
  // Create a TrieNode with no children.
  TrieNode() = default;
  // Q. 이게 parameter가 없는 생성자인가?
  // A. ㅇㅇ 맞음. 기본 생성자라고 함. = default는 컴파일러한테 알아서 만들라는 뜻.

  explicit TrieNode(std::map<char, std::shared_ptr<const TrieNode>> children) : children_(std::move(children)) {}
  // Q. what is explicit?
  // A. c++에 syntax/에 적어둠. 생성자에 보통 explicit 붙이는게 좋은 practice 같더라 

  virtual ~TrieNode() = default;
  // Q. why use virtual on 소멸자?
  // A. 상속 관계에서 자식 클래스 소멸자 제대로 호출되게 하려고. 안 그러면 메모리 누수 생길 수 있음.

  virtual auto Clone() const -> std::unique_ptr<TrieNode> { 
    // 중요!) 이게 write-on-copy의 핵심!
    // Q. 이건 deepcopy임 shallow copy임?
    // A. deepcopy는 아님. 그렇게 구현해도 엄청 비효율이고. 그러면 Get(), Put(), Remove() 할 때마다 전체 Trie 카피하니까 비효율 
    //    그렇다고 shallow copy를 한다? -> Get() 도중에 Put() 해버리면 원본 데이터가 오염되버림 
    //    make_unique()로 이 문제를 해결?
    //    이 코드는 새 TrieNode 객체를 만들어서 생성자에 children_ 넣음.
    //    근데 TrieNode 생성자에 children_(std::move(children)) 이라, 기존 children_의 포인터로 데이터 접근 불가. 
    //    원본과 완전히 다른 객체임: 진짜 얕은 복사면 그냥 포인터만 복사해서 같은 객체 가리키는데, 이건 아님
    //    수정되는 경로만 새로 생성: 이게 핵심인데, "apple"을 수정하면 a->p->p->l->e 경로만 새로 만들고 나머지는 다 공유함
      return std::make_unique<TrieNode>(children_); 
  }
  // Q. 왜 이 함수는 virtual로 만들었지?
  // A. 다형성 때문임. 자식 클래스(TrieNodeWithValue)가 이 함수 오버라이드해서 자기만의 복제 로직 구현할 수 있게
  // Q. 왜 std::unique_ptr을 썼지? 여태껏은 shared_ptr 썼었잖아?
  // A. 소유권 명확하게 하려고. unique_ptr은 한 객체를 한 포인터만 가리킬 수 있음. 이거 쓰면 메모리 관리 더 명확해짐.

  std::map<char, std::shared_ptr<const TrieNode>> children_;
  // Q. TrieNode란?
  // A. map인데, key: char, value: children ptrs 

  bool is_value_node_{false};
};

template <class T>
class TrieNodeWithValue : public TrieNode {
 //Q. : public TrieNode 이게 상속인가? java로 치면 class TrieNodeWithValue extends TrieNode인가?
 //A. ㅇㅇ 상속 맞음. TrieNodeWithValue가 TrieNode 상속받는거.

 public:
  explicit TrieNodeWithValue(std::shared_ptr<T> value) : value_(std::move(value)) { this->is_value_node_ = true; }
  // Q. explicit이 뭐지?
  // A. c++에 syntax/에 적어둠. 생성자에 보통 explicit 붙이는게 좋은 practice 같더라 
  // Q. 함수명() : value_{} {} 에서 value_() 이 문법은 뭐지? 
  // A. 초기화 리스트라고 함. 생성자에서 멤버 변수 초기화하는 C++ 문법. 본문({ } 안)보다 먼저 실행됨. 
  // Q. std::move()가 뭐지? 왜 여기에 쓴거지?
  // A. c++에 syntax/에 적어둠. 객체를 다른 변수로 옮길 때 씀. 그냥 s2 = s1 (string)이면 전체 데이터 카피해서 느린데 s3 = std::move(s1); 하면 포인터만 바꾸는거라 빠름 

  // Create a trie node with children and a value.
  TrieNodeWithValue(std::map<char, std::shared_ptr<const TrieNode>> children, std::shared_ptr<T> value)
      : TrieNode(std::move(children)), value_(std::move(value)) {
    this->is_value_node_ = true;
  }
  // Q. 이 문법이 잘 이해가 안가네. 이 문법의 이름이 뭐고 c++ 어느 버전에서 나왔지?
  // A. 생성자에 TrieNodeWithValue() : TrieNode(std::move(children))  이건 상속받은 부모 클래스의 생성자를 호출하는 것. 

  auto Clone() const -> std::unique_ptr<TrieNode> override {
    return std::make_unique<TrieNodeWithValue<T>>(children_, value_);
  }
  // Q. 여기에선 std::make_unique 썼네? 이 문법은 std::unique_ptr를 복사할 때 쓰는 문법인가?

  std::shared_ptr<T> value_;
  // Q. 여기에 type T의 shared_ptr로 데이터 값 저장하는게 일반 TrieNode와 TrieNodeWithValue의 차이점인가?
  // Q. 왜 두개로 나눈거지? 만약에 "hello" 같은 string만 저장한다면 그냥 TrieNode 만 필요한데, 만약에 ("hello", 2) 같은 데이터 저장할 때 TrieNodeWithValue를 쓰게되는건가?
};

// A Trie is a data structure that maps strings to values of type T. All operations on a Trie should not
// modify the trie itself. It should reuse the existing nodes as much as possible, and create new nodes to
// represent the new trie.
//
// You are NOT allowed to remove any `const` in this project, or use `mutable` to bypass the const checks.
class Trie {
 private:
  // The root of the trie.
  std::shared_ptr<const TrieNode> root_{nullptr};
  // Q. root_ 뒤에 바로 {nullptr}를 붙이는게 c++에 initializer인가? 11년도에 나온 문법인가?
  // A. ㅇㅇ 멤버 변수 기본값 지정하는 C++11 문법임. 클래스 선언할 때 바로 초기화 가능.
  // Q. 왜 그냥 root라고 안하고 root_라고 한거지? 왜 _를 붙인거지? 그리고 붙인다면 보통 앞에 붙이지 않나? 왜 뒤에 붙였지?
  // A. 멤버 변수는 밑줄 붙이는게 흔한 코딩 스타일임. 로컬 변수랑 구분하려고 그럼. Google 스타일은 끝에 밑줄 붙임.
  // Q. shared_ptr?
  // A. shared pointer는 smart pointer중에 하나로, 참조 안되면 자동으로 메모리 해제해줌 

  // Create a new trie with the given root.
  explicit Trie(std::shared_ptr<const TrieNode> root) : root_(std::move(root)) {}
  // Q. explicit?
  // A. syntax/ 에 적어둠 
  // Q. std::move()?
  // A. syntax/ 에 적어둠 

 public:
  // Create an empty trie.
  Trie() = default;
  // Q. default? is it NULL? 이게 parameter가 없는 생성자인가? 이렇게 객체 생성하면 private에 있는 변수 root_도 자동생성 되나?

  template <class T>
  auto Get(std::string_view key) const -> const T *;
  // Q. generic(template for c++) 선언을 매 메서드 위에서 반복적으로 해줘야 하나? 그냥 위에 한번 선언한걸로 T 계속 쓰면 안되나?
  // A. 이건 멤버 함수 템플릿이라 그럼. 클래스 자체는 템플릿 아니고 특정 메서드만 템플릿으로 만들 때 이렇게 함.
  // Q. Get() const 에서 const는 마치 c에서 get(const char c); 이거랑 같은건가? read only? 원본데이터 값을 수정할 수 없는?
  // A. ㅇㅇ
  // Q. auto?
  // A. 타입 T가 generic이니까 컴파일러가 알아서 return type을 auto로 받는다는 뜻 
  // Q. I know std::string, but whats std::string_view? 
  // A. std::string_view는 C++17에 추가된 타입으로, 문자열 소유 안 하고 그냥 참조만 함. 복사 없이 문자열 참조해서 성능 좋아짐.
  // Q. 왜 return type이 T*이지? 해당 타입의 포인터? get은 보통 값을 반환하지 포인터를 반환하는게 아니지 않나? 포인터를 반환하면 다시 *data 인가?로 한번 더 거쳐야 하잖아?
  // A. 찾으려는 값 있으면 포인터 반환하고, 없으면 nullptr 반환하려고. 포인터로 주면 값 있는지 없는지 쉽게 체크 가능.

  template <class T>
  auto Put(std::string_view key, T value) const -> Trie;
  // Q. put의 return type이 왜 boolean이 아니지? 값 넣기에 성공했으면 true, 아니면 false여야 하지 않아?
  // A. 이 Trie는 불변(immutable) 구조라서 새 Trie 반환함. 원본 수정 안 하고 새 버전 만들어서 줌. 함수형 프로그래밍 스타일임.
  // A. trie 수정할 때(write 할 때) 새 trie를 return 하고, 기존 trie는 multi-thread reads를 위해 남겨둠. 원본데이터가 변경 안되는거라 concurrency 문제가 없음 

  auto Remove(std::string_view key) const -> Trie;

  // Get the root of the trie, should only be used in test cases.
  auto GetRoot() const -> std::shared_ptr<const TrieNode> { return root_; }

  void PrintStructure() const;
};

}  // namespace bustub
  