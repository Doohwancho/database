//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// trie.cpp
//
// Identification: src/primer/trie.cpp
//
// Copyright (c) 2015-2025, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "primer/trie.h"
#include <string_view>
#include "common/exception.h"

namespace bustub {

/*
NOTE:
takeaway 

1. Copy-on-Write -> immutability -> write시 multi-thread read해도 서로 간섭 안함
  - write할 때 이전 버전은 multi-thread-reads 를 위해 남겨두고 새로운 트리 노드 노드 만들어서 write 하고 덮어쓰는식
  - 일종의 transaction 구현하는 것 
  - std::move도 씀 
2. type safety
  - dynamic_cast 씀 
3. shared_ptr -> 포인터 free() 자동으로 해줘서 메모리 관리 
*/


/**
 * @brief Get the value associated with the given key.
 * 1. If the key is not in the trie, return nullptr.
 * 2. If the key is in the trie but the type is mismatched, return nullptr.
 * 3. Otherwise, return the value.
 */
template <class T>
auto Trie::Get(std::string_view key) const -> const T * {
  // You should walk through the trie to find the node corresponding to the key. If the node doesn't exist, return
  // nullptr. After you find the node, you should use `dynamic_cast` to cast it to `const TrieNodeWithValue<T> *`. If
  // dynamic_cast returns `nullptr`, it means the type of the value is mismatched, and you should return nullptr.
  // Otherwise, return the value.

  // throw NotImplementedException("Trie::Get is not implemented.");

  // You should walk through the trie to find the node corresponding to the key. If the node doesn't exist, return
  // nullptr. After you find the node, you should use `dynamic_cast` to cast it to `const TrieNodeWithValue<T> *`. If
  // dynamic_cast returns `nullptr`, it means the type of the value is mismatched, and you should return nullptr.
  // Otherwise, return the value.


  std::shared_ptr<const TrieNode> root = root_;
  //Q. root_ 어케씀?
  //A. 이 함수가 Trie::Get()이 Trie클래스의 멤버함수이기 때문에, Trie 클래스의 private 멤버변수 바로 쓸 수 있다. 

  if (root == nullptr) {
    return nullptr;
  }

  // if (key.empty()) { //Q. 이게 empty string인 ""을 나타내는거야? 아니면 NULL이나 nullptr을 가르키는거야?
  //   auto iter = root->children_.find('\0');
  //   root = iter->second; //Q. input이 ""일 때, '\0'을 반환하는게 맞는걸까?
  // }

  // 빈 문자열 처리
  if (key.empty()) {
    auto iter = root->children_.find('\0');
    if (iter == root->children_.end()) {
      return nullptr;  // '\0' 키가 없으면 nullptr 반환
    }
    root = iter->second;
    
    if (!root->is_value_node_) {
      return nullptr;
    }
    
    auto value = dynamic_cast<const TrieNodeWithValue<T> *>(root.get());
    if (value) {
      return value->value_.get();
    }
    return nullptr;
  }

  for (auto index : key) {
    auto iter = root->children_.find(index);
    if (iter == root->children_.end()) {
      return nullptr;
    }
    root = iter->second;
  }

  if (!root->is_value_node_) {
    return nullptr;
  }

  auto value = dynamic_cast<const TrieNodeWithValue<T> *>(root.get()); //Q. 이거 왜 했지? 그냥 파라미터 없는 .get()은 그 자체를 반환하는데, 이걸 TireNodeWithValue로 변환해서 만약 값이 있으면 반환해주는건가?
  if (value) {
    return value->value_.get();
  }

  return nullptr;
}

/**
 * @brief Put a new key-value pair into the trie. If the key already exists, overwrite the value.
 * @return the new trie.
 */
/**
 * 핵심 
 * 1. 경로 복사해서 새로 만듦 
 * 2. 기존 자식들도 가져옴 
 * 3. 마지막에 값 노드 넣음 
 */
template <class T>
auto Trie::Put(std::string_view key, T value) const -> Trie {
  //step1) copy-on-write(COW) 패턴 하려면 먼저 shallow copy를 해야겠지? 그 다음에 수정할 노드만 deepcopy 하는거니까 일단 Clone()으로 shallow_copy 한다. 
  std::shared_ptr<TrieNode> root;
  if (root_ == nullptr) {
    root = std::make_shared<TrieNode>();
  } else {
    root = root_->Clone(); //COW의 시작! 먼저 shallow copy of root node로부터 시작.
  }

  // step2) 오해! trie 순회가 아니라 문자열 순회하면서 그에 해당하는 trie 경로를 찾거나 만듬 ("hello"면 "h","e","l","l","e" 노드 순으로 가는 것)
  // root->children_은 map이니까, 
  //root노드 대체할 새 root_node 생성 
  std::shared_ptr<TrieNode> parent = root;

  auto it = key.begin(); //문자열의 첫 char부터 시작. ex. "hello" -> "h"
  while (it != key.end() && std::next(it) != key.end()) { //문자열 끝까지 반복해서 map을 찾음. children_은 std::map<char, std::shared_ptr<const TrieNode>> children_; 이기 때문에, key: char 
    auto iter = parent->children_.find(*it); 
    std::shared_ptr<TrieNode> node; //새 node를 만들어서

    if (iter != parent->children_.end()) { //문자열 끝까지 반복 (마지막 글자는 특별처리)
      node = iter->second->Clone(); // 기존 노드 복제. 예를 들어, "apple"과 "app"이 저장된 Trie에 "applet"을 Put 한다면:
                                    // "a", "p", "p" 경로는 이미 존재하므로 해당 노드들을 Clone() (원본꺼 shallow copy로 그대로 씀)
                                    // "l", "e", "t" 노드는 새로 생성
                                    // "apple"의 "l", "e" 부분은 원래 트리에 그대로 남음
    } else {
      node = std::make_shared<TrieNode>(); //새 노드 생성. 이 부분은 복사 안함 
    }

    parent->children_[*it] = node; //수정한 노드의 "자식들"은 **원본 노드에서 그대로 가져옴**
    parent = node;
    it++; //다음글자로 이동 
  }

  // step3) 만약 그냥 쌩 string이 아니라 value까지 있으면? 
  std::shared_ptr<T> pvalue = std::make_shared<T>(std::move(value));
  //원래 value를 std::move()로 deference 하고 pvalue에 할당 

  auto iter = parent->children_.find(*it);

  // string의 마지막 문자열 처리 
  if (key.empty() || iter == parent->children_.end()) {
    parent->children_[*it] = std::make_shared<TrieNodeWithValue<T>>(pvalue);
  } else {
    auto children = iter->second;
    parent->children_[*it] = std::make_shared<TrieNodeWithValue<T>>(children->children_, pvalue);
  }

  // step4) 새로운 Trie 반환 
  std::shared_ptr<Trie> new_trie = std::make_shared<Trie>();
  new_trie->root_ = root;
  return *new_trie;
}


/********
 * Debug 목적을 위한 Trie의 전체 node 돌면서 print 하는 함수 
 * 
 * 각 노드에서 마지막 letter면 ValueNode라고 표시되고, 값이 없고 지나가는 Node면 RegularNode라고 표시된다.
 * example)
    first put! - 'test', 2333
    Trie Structure:
      Node: RegularNode, Children Count: 1
        Child 't':       Node: RegularNode, Children Count: 1
            Child 'e':           Node: RegularNode, Children Count: 1
                Child 's':               Node: RegularNode, Children Count: 1
                    Child 't':                   Node: ValueNode, Children Count: 0
    second put! - 'te', 23
    Trie Structure:
      Node: RegularNode, Children Count: 1
        Child 't':       Node: RegularNode, Children Count: 1
            Child 'e':           Node: ValueNode, Children Count: 1
                Child 's':               Node: RegularNode, Children Count: 1
                    Child 't':                   Node: ValueNode, Children Count: 0
    third put! - 'tes', 233
    Trie Structure:
      Node: RegularNode, Children Count: 1
        Child 't':       Node: RegularNode, Children Count: 1
            Child 'e':           Node: ValueNode, Children Count: 1
                Child 's':               Node: ValueNode, Children Count: 1
                    Child 't':                   Node: ValueNode, Children Count: 0
    After removing 'tes':
    Node: RegularNode, Children Count: 1
      Child 't':     Node: RegularNode, Children Count: 1
          Child 'e':         Node: ValueNode, Children Count: 1
              Child 's':             Node: RegularNode, Children Count: 1
                  Child 't':                 Node: ValueNode, Children Count: 0
 */

void Trie::PrintStructure() const {
  std::cout << "Trie Structure:" << std::endl;
  if (root_ == nullptr) {
    std::cout << "  Empty trie" << std::endl;
    return;
  }
  
  std::function<void(const std::shared_ptr<const TrieNode>&, std::string)> print_node;
  print_node = [&](const std::shared_ptr<const TrieNode>& node, std::string indent) {
    std::cout << indent << "Node: " << (node->is_value_node_ ? "ValueNode" : "RegularNode") 
              << ", Children Count: " << node->children_.size() << std::endl;
    
    for (const auto& [ch, child] : node->children_) {
      std::string char_repr;
      if (ch == '\0') {
        char_repr = "\\0";
      } else {
        char_repr = ch;
      }
      std::cout << indent << "  Child '" << char_repr << "': ";
      print_node(child, indent + "    ");
    }
  };
  
  print_node(root_, "  ");
}

void PrintTrieStructure(const std::shared_ptr<const TrieNode>& node, std::string prefix = "", std::string indent = "") {
  if (node == nullptr) {
    std::cerr << indent << "nullptr" << std::endl;
    return;
  }

  std::cerr << indent << "Node: " << (node->is_value_node_ ? "ValueNode" : "RegularNode") 
            << ", Children Count: " << node->children_.size() << std::endl;

  for (const auto& [ch, child] : node->children_) {
    std::string char_repr;
    if (ch == '\0') {
      char_repr = "\\0";
    } else {
      char_repr = ch;
    }
    std::cerr << indent << "  Child '" << char_repr << "': ";
    PrintTrieStructure(child, prefix + ch, indent + "    ");
  }
}

auto Trie::Remove(std::string_view key) const -> Trie {
  // 비어있는 트리는 그대로 반환
  if (root_ == nullptr) {
    return *this;
  }

  // 루트 노드 복제
  std::shared_ptr<TrieNode> root = root_->Clone();
  
  // 빈 문자열 처리
  if (key.empty()) {
    auto iter = root->children_.find('\0');
    if (iter != root->children_.end()) {
      root->children_.erase('\0');
    }
    
    // 루트가 비었는지 확인
    if (root->children_.empty()) {
      return Trie(); // 빈 트리 반환
    }
    
    // 새 트리 생성
    std::shared_ptr<const TrieNode> const_root = root;
    return Trie(const_root);
  }

  // 키를 따라 경로 구축
  std::vector<std::pair<std::shared_ptr<TrieNode>, char>> path;
  std::shared_ptr<TrieNode> current = root;
  
  // 키의 모든 문자(마지막 제외) 처리
  for (size_t i = 0; i < key.size() - 1; i++) {
    char ch = key[i];
    auto iter = current->children_.find(ch);
    if (iter == current->children_.end()) {
      // 키가 존재하지 않음
      return *this;
    }
    
    path.push_back({current, ch});
    current = iter->second->Clone();
    path.back().first->children_[ch] = current;
  }
  
  // 마지막 문자 처리
  if (!key.empty()) {
    char last_char = key.back();
    auto iter = current->children_.find(last_char);
    if (iter == current->children_.end()) {
      // 키가 존재하지 않음
      return *this;
    }
    
    std::shared_ptr<const TrieNode> last_node = iter->second;
    
    // *** 핵심 변경 부분 ***
    // 값 노드인지 확인 (자식 유무와 상관없이)
    if (last_node->is_value_node_) {
      if (last_node->children_.empty()) {
        // 자식이 없으면 완전히 제거
        current->children_.erase(last_char);
        
        // 상위 노드들 검사해서 빈 노드 제거 - 단, 값 노드는 유지
        for (int i = static_cast<int>(path.size()) - 1; i >= 0; i--) {
          auto [node, ch] = path[i];
          auto child_iter = node->children_.find(ch);
          
          // 이미 삭제된 자식은 건너뜀
          if (child_iter == node->children_.end()) {
            continue;
          }
          
          // 자식 노드가 비어 있고 값 노드가 아닌 경우에만 제거
          if (child_iter->second->children_.empty() && !child_iter->second->is_value_node_) {
            node->children_.erase(ch);
          } else {
            break;  // 비어있지 않거나 값 노드인 경우 중단
          }
        }
      } else {
        // 자식이 있는 경우 일반 노드로 변환
        std::shared_ptr<TrieNode> regular_node = std::make_shared<TrieNode>(last_node->children_);
        current->children_[last_char] = regular_node;
      }
    } else {
      // 이미 값 노드가 아님 - 변경 없음
      return *this;
    }
  }
  
  // 루트가 비어있으면 빈 트리 반환
  if (root->children_.empty()) {
    return Trie(); // 빈 트리 반환
  }
  
  // 수정된 트리 반환
  std::shared_ptr<const TrieNode> const_root = root;
  return Trie(const_root);
}


// Below are explicit instantiation of template functions.
//
// Generally people would write the implementation of template classes and functions in the header file. However, we
// separate the implementation into a .cpp file to make things clearer. In order to make the compiler know the
// implementation of the template functions, we need to explicitly instantiate them here, so that they can be picked up
// by the linker.

template auto Trie::Put(std::string_view key, uint32_t value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const uint32_t *;

template auto Trie::Put(std::string_view key, uint64_t value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const uint64_t *;

template auto Trie::Put(std::string_view key, std::string value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const std::string *;

// If your solution cannot compile for non-copy tests, you can remove the below lines to get partial score.

using Integer = std::unique_ptr<uint32_t>;

template auto Trie::Put(std::string_view key, Integer value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const Integer *;

template auto Trie::Put(std::string_view key, MoveBlocked value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const MoveBlocked *;

}  // namespace bustub
