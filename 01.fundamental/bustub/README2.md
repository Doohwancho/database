# project0 

## 1. how to run?
```
cd build

make trie_test trie_store_test -j$(nproc)
make trie_noncopy_test trie_store_noncopy_test -j$(nproc)

./test/trie_test
./test/trie_noncopy_test
./test/trie_store_test
./test/trie_store_noncopy_test
```

## 2. takeaway 

1. 각종 c++ 문법들 (computer-science/c++/syntax) 참고
2. trie 자료구조 
3. copy-on-write 패턴 (computer-science/c++/concept/copy-on-write) 참고
    - trie.h 를 먼저 봐서 이 자료구조가 어떻게 구성되었는지 먼저 보고 
    - trie.cpp에 Put()을 보면 됨 
4. concurrency control할 때 lock 최소화로 잡는 테크닉 
    - COW 패턴이라 Put()이나 Remove()에 lock()을 안검
    - 먼저 root_node만 Trie local_trie;에 새로 만든 후,
    - local_trie.Put()함. 이 때 트리 돌면서 나머지는 원본데이터 고대로 가져오는데, 바뀌는 부분만 새롭게 노드를 만들음. 
    - 그러면 실질적으로 만들어지는 부분은 local_trie(new head)랑 바뀌는 노드들
    - 바뀌는 부분을 새로 만드는거니까 여기에까지 lock 걸 필요 없음 
    - Put()이 끝났으면 root_ = new_trie 해서 덮어씀. 
