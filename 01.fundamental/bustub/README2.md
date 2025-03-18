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

## 2. goal 
1. trie로 get(), put(), remove() 만드는데, 
2. multi-thread 환경에서 concurrency control도 하면서 
3. read()할 떄 중간에 put()이 껴들어서 원본데이터 오염시키면 안되니까 put() 도중에 lock을 걸까? 생각하다가도, 
4. 성능도 챙겨야 하니까, copy-on-write 패턴 써서 동시성 컨트롤 하면서 성능도 챙겨 



## 3. takeaway 

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


# project1 

## 1. how to run?
test task1
```
# LRU-K Replacer만 빌드 및 테스트
make lru_k_replacer_test -j$(nproc)
./test/lru_k_replacer_test
```

test task2
```
make buffer_pool_manager_test -j$(nproc)
./test/buffer_pool_manager_test
```

tset task3
```
make page_guard_test -j$(nproc)
./test/page_guard_test
```

## 2. task1

### 2-1. 어떤 파일 봐야함?
핵심 파일
- 구현 파일: `src/buffer/lru_k_replacer.cpp`
- 헤더 파일: `src/include/buffer/lru_k_replacer.h`
- 테스트 파일: `test/buffer/lru_k_replacer_test.cpp`

### 2-2. what and why?
project1은 버퍼 풀 구현하는 과제임.\

---\
Q. 버퍼 풀이 뭐임?

A. 데이터를 disk에 저장하는데, disk는 느리잖아?\
그래서 RAM에 올려서 쓰려는데,\
RAM으로 다 올리기엔 공간이 부족하니까,\
일부만 올리다가, 공간 부족하면, 안쓰는애(가장 오래 안쓴애) 다시 disk에 넣고, 공간 비우는걸 하는애가 버퍼풀임\
일종의 캐시 레이어

---\
Q. 그럼 LRU는 뭐임?

A. LRU(least recently used) - replacement policy == 젤 오래 안쓴놈 내쫓는다는거임 RAM->disk로.

한마디로 "가장 마지막에 접근한 페이지" <--- 이거 1가지만 보고 쫓아는건데,

근데 얘가 단점이 있음.

**한번 접근한애랑 여러번 접근한애랑 구분을 못함.**

예시)

예를 들어보자:

페이지 A: 1초에 접근, 2초에 접근, 3초에 접근, ... 98초에 접근, 99초에 접근 (자주 쓰는 놈)\
페이지 B: 100초에 한 번만 접근 (딱 한 번 쓴 놈)

만약 지금이 101초이고 누구 쫓아낼지 결정해야 한다면:

일반 LRU: B가 더 최근에 접근했으니까 A를 쫓아냄 (근데 이거 비효율적임. A는 자주 쓰이는데)\
LRU-K: A는 지속적으로 접근했지만 B는 한 번만 접근했으니까 B를 쫓아냄 (더 효율적)



---\
Q. LRU-K는 뭐임?

A. LRU-K에서 K는 "몇 번째 이전 접근까지 볼거냐"를 의미해. 예를 들어 K=2면:

각 페이지마다 최근 2번의 접근 시간을 기록함\
K-거리 = (현재시간 - K번째 이전 접근시간)\
즉, "현재시간 - 뒤에서 두 번째 접근시간"

예를 들어 프레임별로 접근 시간이 이렇다고 해보자 (K=2):

프레임 1: [10, 30, 50, 90] (10초, 30초, 50초, 90초에 접근)\
프레임 2: [20, 60, 80]\
현재 시간: 100초

K-거리 계산:

프레임 1: 100 - 50 = 50 (뒤에서 두 번째 접근은 50초)\
프레임 2: 100 - 60 = 40 (뒤에서 두 번째 접근은 60초)

K-거리가 더 큰 프레임 1을 쫓아냄. 왜냐? K-거리가 크다는 건 K번째 이전에 접근한지가 오래됐다는 뜻이니까.


중요한 특수 케이스:\
K번 미만으로 접근한 페이지들은 K-거리를 무한대로 간주해.\
왜냐? K번째 이전 접근이 없으니까.\
이런 페이지들은 K번 이상 접근한 페이지들보다 먼저 쫓아내게 됨.\
그리고 K번 미만 접근한 페이지들 중에서는 가장 처음 접근 시간이 오래된 놈부터 쫓아냄.



- 구현 파일: `src/buffer/lru_k_replacer.cpp`
- 헤더 파일: `src/include/buffer/lru_k_replacer.h`
이걸 여기에 구현하는 거임.


---\
Q. LRU-K 계산하는 부분에 동시성 처리 어떻게 하지?

mutex거는 부분 
1. Evict() 함수
2. RecordAccess() 함수
3. SetEvictable() 함수
4. Remove() 함수



## 3. task2
### 3-1. what 
Q. what is buffer pool manager?
A. 페이지 가져오고 관리하는 놈


Q. 이게 진짜 중요한 이유?
1. 디스크 읽기/쓰기는 개느림
2. 자주 쓰는 페이지는 메모리에 계속 있어야 함
3. 여러 트랜잭션이 동시에 페이지 접근할 수 있음


Q. read & write 시 진행과정?
A.
1. 먼저 요청하는 이 페이지가 버퍼풀에 있나 확인
2. 있으면 해당 페이지의 pin_count 증가하고 반환 (페이지 사용중이면 pin_count가 1이 되어 LRU-K replacement policy로 교체되지 않도록 함)
3. 없으면 빈 frame을 찾거나 task1에서 구현한 LRU-K 로 disk에 보낼 희생자 선택
4. disk에서 페이지 읽어오기
5. 페이지 ID <-> 프레임 ID 매핑 테이블 업데이트
6. 핀 카운트 = 1 설정 (페이지 사용중이면 pin_count가 1이 되어 LRU-K replacement policy로 교체되지 않도록 함)
7. 페이지 반환


Q. page와 frame?

A.
page는 disk에 있는 논리적 데이터 단위이고\
frame은 RAM에 있는 논리적 데이터 단위임.

disk가 용량이 훨씬 더 크고(500GiB), RAM은 고작 8GiB라,\
frame 갯수는 몇개 안됨 한 10개라고 치는데,\
page는 갯수가 100개가 될 수도 있음.

이 둘을 매핑해주는게 buffer pool manager에 `std::unordered_map<page_id_t, frame_id_t> page_table_;`

모든 page를 frame(RAM)에 못올리니까 어떤 page 쫓아낼지 정하는게 LRU_K replacement policy.\
`auto victim = replacer_->Evict();`


## 4. task3

### 4-1. what 

Q. page guard는 왜 씀?

A. 메모리 관리 + pin count set to 0 + setEvictable to false 자동화 하려고. (RAII)

개발자가 수동으로 set pin count to 0같은거 하다가 깜박하고 뺴먹으면 그 페이지 계속 못씀.

page_guard.cpp에 소멸자 보면, Drop()을 호출하고(RAII), Drop()을 보면, 

frame에 lock 해제하고, pin count 0, evictable to false 함.

