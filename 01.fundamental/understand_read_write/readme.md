# A. summary 
## 1. read optimization의 핵심 
결국 caching을 통해 disk i/o 최소화 하자는 게 핵심.

버퍼풀(cache) 사이즈 최대한으로 늘리고,\
버퍼풀 내에 불필요한 데이터가 안올라오게 해서 cache hit rate 99%+ 찍는걸 목표하고,\
disk 읽을 때 pk말고 커버링 인덱스 말고 그냥 인덱스 기준으로 읽으면 물리적으로 붙어있는 애들 통으로 가져오는게 아니라, random i/o 하는데, 이것도 줄이고. 

## 2. write optimization의 핵심
느린 random i/o write를 어떻게서든 피하고,\
작업을 지연/병합(batch)하여 최대한 sequential i/o로 바꾸는 게 핵심 

1. mvcc에 redo-log (disk random i/o write -> disk sequantial i/o write 후, 나중에 함)
2. change buffer (인덱스용 b+tree의 sk random i/o read 대신, ram에 쓰고 나중에 disk random i/o write함)
...가 핵심 

주요 병목지점은 disk random i/o 이고,\
이걸 지금 당장 피하기 위해서 redo-log로 disk에 sequential write하고(그래도 disk random i/o write보다 훨씬 빠름),\
나중에 쌓이면 redo-sequantial-log-on-disk -> 원래 데이터가 있는 페이지에 random i/o 하는 것 

change buffer는 업데이트하는 필드가 인덱스 걸려있는데,\
버퍼풀에 해당 인덱스의 페이지가 안올라온 경우,\
disk random i/o해서 불러와서 쓰지 말고,\
일단 RAM에 써놓고 썼다 치고 나중에 여유생기면 disk random i/o로 쓰는 것.




# B. how read works in b+tree based RDB?
![mysql](./mysql_architecture.png)

## --- 1. application layer ---
1. step1. sql query -> connection pool 
    - 백엔드에서 쿼리 요청하는데 스레드 풀에 담긴 스레드 하나가 rdb에서 연결 담당하는 스레드풀에 rds-connection용 스레드랑 통신함. 쿼리 넘겨줌. 
    - PMM metrics에 `com_set_options`이 idle thread in thread pool에 요청 날리는 것. 아니면 부하 더 많아져서 thread 늘려야 한다는 요청 날리는거나.
## --- 2. query processing layer --- 
2. step2. query caching
    - 쿼리가 캐싱되어있는지 확인하고 캐싱되있으면 값 바로 반환 
    - mysql 5.7까지만 했었고, mysql 8.0 이후로는 삭제됨 
    - 테이블 데이터가 단 한 줄이라도 변경되면 그 테이블과 관련된 캐시를 모두 삭제(invalidation)해야 했기 때문입니다. 이로 인해 쓰기 작업이 빈번한 환경에서는 캐시를 유지하는 비용이 더 커져 성능 저하의 원인이 되었습니다.
    - 쿼리 캐싱은 옛날방식. sql 쿼리 +  그 결과를 메모리에 그대로 저장하는건데, 테이블에 write하면 얘도 삭제했어야 했어서 비효율이어서 없어졌잖아?
    - InnoDB bufferpool(현대 방식): 쿼리 결과가 아닌, 디스크의 데이터와 인덱스 페이지(Page) 자체를 메모리에 캐싱합니다. 어떤 쿼리가 들어오든 필요한 데이터/인덱스 페이지가 버퍼 풀에 있다면 디스크를 읽지 않고 메모리에서 바로 처리합니다. 이것이 훨씬 효율적이고 재사용성이 높습니다.
    - db read optimize를 할 때 개발자나 DBA가 가장 중요하게 여기는 설정은 바로 이 innodb_buffer_pool_size입니다. 보통 RDS RAM 메모리의 50~70%까지 설정하여 디스크 I/O를 최소화하는 것을 목표로 합니다.
3. step3. parser
    - 파서가 쿼리를 파싱함 하여 select, from, where의 계층적 tree로 만듬. 
    - sql 문법 에러 있으면 여기서 바로 return 
4. step4. query optimizer & execution plan  
    - 옵티마이저한테 쿼리를 넘겨주면, 어떤 방식,순서로 쿼리해야 가장 효과적인지 roadmap을 짜서 cost가 가장 낮은 애를 고름 
        - 그 기준은 index를 어떤걸 써서 쿼리할지,(hash index, unique index, text based index) 
            - type이 ALL(풀 테이블 스캔)이나 index(풀 인덱스 스캔)가 아닌 ref, eq_ref, const 등으로 나오는지 확인합니다.
            - Extra 필드에 Using filesort, Using temporary가 뜨지 않도록 쿼리나 인덱스를 수정합니다.
        - join할 때 테이블 join 순서(사이즈가 큰 테이블이 작은 테이블을 join거는 쪽으로)
        - 어떤 join할지(nested join, sort merge join, hash join)
        - 경우의수 4 * 3 * 3 = 36가지라고 치면, 그중에서 제일 cost 낮은 쿼리를 고름 
    - cardinality 예측해서 full scan vs 인덱스 탈지 정한다. 
        - 옵티마이저가 가장 중요하게 보는 통계 정보는 카디널리티(Cardinality), 즉 특정 컬럼에 얼마나 다양한 값이 분포하는가입니다. 
        - WHERE name = '김철수' 조건이 있을 때, 옵티마이저는 "과연 '김철수'라는 이름을 가진 데이터가 몇 건이나 될까?"를 예측해야 합니다. 이 예측된 행 수를 기반으로 인덱스를 탈지, 풀 스캔을 할지 결정하기 때문입니다.
        - ex) 성별 M,F 은 카디널리티가 50%니까, 걍 풀스캔하자!  vs email='hello@gmail.com'은 1개뿐이니까, 인덱스 타자! 
    - cardinality 예측의 한계 
        - 이 예측을 위해 옵티마이저는 **히스토그램(Histogram)** 을 사용합니다. 히스토그램은 컬럼의 데이터 분포를 막대그래프처럼 구간별로 나누어 저장한 정보입니다.
        - 근데 이 histogram이 전체 데이터 기반으로 만들어지지 않고, 몇개만 샘플 떠서 만들어짐. 
        - 따라서 샘플링된 데이터가 실제 데이터 분포랑 다른 경우, 옵티마이저의 카티널리티 예측이 크게 빗나갈 수 있음. 예를 들어, 옵티마이저는 10건을 예상하고 인덱스를 사용했지만 실제로는 100만 건이 조회되어 오히려 풀 스캔보다 훨씬 느려지는 최악의 실행 계획이 선택될 수 있습니다.
        - ex. '도시'컬럼에 인덱스가 걸려있는데, 옵티마이저는 도시가 다양하다고 생각해서 카디널리티가 높다고 착각하겠지만, 서울90% -> 깜짝?!
        -      그러면 '도시'컬럼을 인덱스 타는데, 사실 full scan이 훨씬 빠르지. 
    - join 순서 
        - 일반적으로 join할 떄 hash join, merge sort join말고 nested loop join(O(N^2))하는데, 반복문 횟수를 최대한 줄이고 싶어함 옵티마이저가 
        - 그래서 driving table쪽이 최대한 row가 필터링 되서 적은 쪽이 좋음. 왜냐면 join 거는 테이블에 where id = pk같은거 하면, 매 row마다 O(logN)이니까, 뒤에꺼가 더 빠름.
        - 예를들어, 회원(1억 row) join 우수회원(100 row) 한다고 했을 때, 
        - 우수회원 join 회원이면, loop 100번동안, 회원 테이블에 pk 기준으로 찾는건데, 이게 loop 1억번동안, 우수회원 테이블 fullscan(or pk 기준으로 찾기)보다 빠름. 
    - hint
        - query에 hint있으면 참고하는 편이나, 현대 optimizer가 굉장히 발달했기 때문에, hint는 엄청 확실하고 엄청 성능 요하고 엄청 안바뀌는 금융코드 아니면 박지마
## --- 3. storage engine layer ----
3. step5. buffer pool 
    - InnoDB는 가장 먼저 버퍼풀을 확인해서, 만약에 페이지가 버퍼풀에 캐싱되어 있었다면, disk i/o없이 순수 메모리 내에서 b+tree 탐색이 일어남. 
    - cache hit!
        - 버퍼 힛! 이게 젤 빠른 시나리오. 
        - buffer pool의 cache hit 99%이상 목표해야 하고, 낮아지면 full scans이 버퍼풀 오염시키는지 확인해야 함. 
    - 버퍼풀 안에 원본 데이터 있는 b+tree랑 인덱스 데이터 담긴 b+tree가 있음 
        - Q. 만약 인덱스 탄다고 하면, 인덱스 컬럼이 있는 b+tree로 leafnode에 실제 데이터의 위치를 파악하고, 여기서 헤깔리는부분인데, 이 실제 데이터의 위치를 가지고 바로  clustering index(b+tree인데 pk가 키고, leaf node에 실제 데이터가 있는 것)
        - A. index건 컬럼이 든 b+tree의 leaf node에 목적하는 데이터의 pk가 들어있는데, 이걸 가지고 다시 clustering index b+tree에 서치해서 leaf node에서 원본 데이터 가져감 
    - Adaptive Hash Index, AHI
        - 인덱스가 담긴 b+tree도 O(logN)인데, 이걸 더 빠르게 하기 위해 존재하는 인덱스 전용 자료구조 
        - 특정 페이지가 자주 검색되는걸 발견하면,(ex. 동일한 where조건 쿼리가 자주 실행됨) -> 그 페이지에 대한 해시 인덱스를 버퍼 풀 안에 자동 생성
        - 이 해시 인덱스는 (인덱스 키 값, 데이터 페이지 메모리 주소) 쌍으로 구성됨 
        - 이제 자주 반복되는 쿼리가 인덱스 타면, O(logN)으로 2번 하는게 아니라, O(1)으로 찾음
        - 원래 저 해시테이블이 없었으면, 인덱스가 담긴 b+tree한번 타고 리프노드에서pk값 얻은 다음에, 원본데이터가 있는 b+tree 한번 더 타서 O(logN) 2번 했을텐데, 자주 쿼리되어 AHI에 캐싱되었다면, O(1)으로 바로 페이지의 주소를 얻는 거니까, 원본 데이터의 b+Tree 한번만 타면 끝 
    - 페이지와 random I/O
        - 원본 b+tree(pk : 원본 데이터가 담긴 페이지의 주소)랑, name필드에 건 인덱스용 b+tree(알파벳 순 정렬된 name : pk) 이 두 b+tree안에 leaf node들이 16kb 페이지 단위로 끊어서 페이지에 저장됨. 
        - 이 페이지를 버퍼풀에 로드해와서 페이지 안에 clutering index, 인덱스용 b+tree를 쓰는 것.
        - 인덱스용 b+tree에 값들은 정렬되서 비슷한 애들끼리 있겠지 이렇게. 
            - 101호: ('Gildong', 본관 105호) 
            - 102호: ('Gimin', 본관 32호)
            - 103호: ('Gisoo', 본관 512호)
        - 근데 이 pk값을 가지고 clutering index의 leafnode에서 원본데이터가 위치한 페이지의 주소값을 보면, 죄다 다른곳에 위치하겠지. 
            - ...
            - 32호: (id: 32, name: 'Gimin', age: 25, ...)
            - ...
            - 105호: (id: 105, name: 'Gildong', age: 30, ...)
            - ...
            - 512호: (id: 512, name: 'Gisoo', age: 40, ...)
            - ...
        - 이 물리적으로 떨어져있는 데이터를 찾는게 random I/O이고, 이게 인덱스 타는 쿼리의 성능저하의 주요 원인(pk fullscan 대비)
        - 그래서 이 random I/O 피하려고, 
            1. pk기반 풀스캔 하거나
                - 테이블 사이즈가 작을 때, 물리적으로 모여있으니까 아싸리 다 가져오는게 더 빠른 경우,
                - 카디널리티가 낮은 컬럼에 index용 b+tree타고 원본데이터용 b+Tree 타고 random i/o까지 하느니, 물리적으로 모여있는애 순차read 하겠다
            2. pk 기반 범위검색, range scan 하거나, (WHERE id BETWEEN 1000 AND 1100과)
            3. covering index를 하거나 (보조 인덱스 b+tree만 읽고 작업 끝냄. clustering index까지 안감 + random i/o 안해서 빠름)
    - 버퍼풀 오염?
        - 자주 쿼리되던 인덱스용 b+tree와 clustering index가 버퍼풀에 로드되서 disk i/o 안하고 메모리 내에서만 read하는데, 
        - 갑자기 뭐 유저 테이블 100만 rows fulls can한다고 하면, 얘를 다 disk i/o 해와서 버퍼풀 메모리에 올리는데, 
        - 버퍼풀 메모리가 한정되있다보니 LRU같은 걸로 원래 쓰이던 애들 밀어내고 딱 한번만 쓰고 말 유저 테이블 풀스캔 데이터를 버퍼풀 메모리에 올려버림 
        - 그러면 또 자주 쿼리되는거 요청 오면 cache miss 떠서, 다시 disk i/o해서 불러오고 메모리 로드해야 함. 
        - 근데 맨 처음엔 disk i/o해서 불러오는게 인덱스 b+tree랑 원본 b+tree인데, 얘 타서 페이지 주소값 얻어도, 얘도 아까 100만건 full scan으로 밀려났기 문에, 다시 disk i/o 해서 가져와서 버퍼풀에 올려야 함. 
        - 이래서 잘 안쓰는 쿼리 full scan이면 안좋나보다. 
## --- 4. disk layer --- 
4. step6. buffer pool cache miss -> disk i/o
    - buffer pool에 cache miss나면, InnoDB는 디스크로부터 해당 page 읽어오라고 파일 시스템에 요청함
    - os는 먼저 파일 시스템 캐시를 확인하고, 없으면 물리디스크에서 접근함. 
    - hardware layer에서 SDD/HDD가 디스크의 특정 sector에 접근하여 page가 포함된 block을 읽어 RAM에 전송함. 
    - 읽어온 페이지는 버퍼 풀의 빈 공간(Frame)에 적재되고, LRU 리스트 같은 자료구조가 업데이트됩니다. 이제 해당 페이지는 버퍼 풀에 있으므로, 중단됐던 메모리 내 탐색 과정을 재개합니다. 
5. step7. MVCC (multi version concurrency control) & Locking
    - what 
        - Step 5와 Step 6을 통해 InnoDB가 물리적으로 데이터가 담긴 페이지를 찾아 메모리(버퍼 풀)에 올리는 데 성공했다고 가정해 봅시다.
        - 이제 Step 7에서 할 일은 그 페이지에 있는 데이터를 클라이언트에게 바로 반환하는 것이 아니라, **"현재 트랜잭션이 이 데이터를 보는 것이 허용되는가?"** 를 판단하는 것입니다. 이 가시성(Visibility) 판단 과정에서 MVCC와 격리 수준(Isolation Level)이 핵심적인 역할을 합니다. 
    - MVCC란? 
        - "읽기는 쓰기를 막지 않고, 쓰기는 읽기를 막지 않는다"
        - 원래같았면 write 도중 같은 row를 read하려는 다른 트랜젝션이 있을 때, beta lock으로 막았겠지만, mvcc에 undo-log에 먼저 원본 데이터를 저장한 후에, write lock 걸고 값을 수정하기 때문에, 동시간 같은 row에 read 요청 오면, 저 원본 데이터를 읽어감. lock을 안기다려도 되서 concurrency 상황에서 빠름. 
    - Isolation level 
        - 격리 수준에 따라, undo log만드는 snapshot 생성 시점이 달라진다. 
        - types
            1. READ UNCOMMITTED
                - 스냅샷 생성안함 
                - MVCC 안쓰고, 따라서 undo-log도 안씀. 
                - 다른 트랜젝션이 아직 commit하지도 않았는데 냅다 읽어서 반환해버림 
                - dirty read, non-repeatable read, phantom read 다 발생함 
                - dirty read 예시 
                    - ex. 1. update 계좌잔고 천만원 2. read from 계좌잔고 천만원 3. update중 에러발생! rollback! 4. read는 여전히 천만원 와!
            2. READ COMMITTED
                - 매 `select` 구문마다 새로운 snapshot을 생성함 
                - 쿼리를 실행할 때마다 그 시점에 커밋된 데이터들로 새로운 스냅샷을 만들기 때문에, 한 트랜잭션 내에서도 조회 시점마다 다른 결과를 볼 수 있습니다.
                - non-repeatable read, phantom read 다 발생
                - non-repeatable read 예시 
                    1. 사용자 1이 남은 티켓 read해서 1장남은걸 확인 (select 쓴 순간에 undo-log에 스냅샷 뜸. 1장이라고 나옴)
                    2. 사용자 2가 그 사이에 티켓 1장 구매해서 0장됨 
                    3. 사용자 1의 트랜젝션은 아직 안끝났었음. 다른 일 처리하다가 다시 티켓 read 했는데 0으로 되있네? (select 쓴 순간에 undo-log에 새로운 스냅샷 뜸. 0장이라고 뜸.)
                    - 이 사단이 난건, 하나에 트랜젝션 사이에 같은 row 2번 read했는데 값이 다르네???
                    - 왜냐면 read 쿼리 2번 날릴 때, select 2번 있는데, select 기준으로 undo-log에 snapshot 뜨기 때문
                    - 만약에 isolation 수준이 repeatable read였다면, 스냅샷 뜨는 기준이 select가 아니라 transaction이라, 3번단계에서 1이라고 반환했을 것. 
                    - Q. 그러면 오히려 안좋은거 아닌가? 실제론 0개인데 예전거인 1개라고 하면, 실제로 구입할 때 에러날거아냐?
                    - A. 맞음. 근데 그보다 '트랜젝션 동안에 논리적 정합성'을 더 중요시 해서 transaction 초반에 값이랑 같은값을 반환하는거고, 방금 언급한 문제는 `select ... for update`로 해결함. 
                    -    핵심은 요청 건, 트랜젝션 시작 시점에서 원본데이터를 가지고 일관성 있게 데이터 뚝딱뚝딱해서 반환한다는거임. 그 사이에 write로 값이 바뀌면 어지러워지니까. 
                    -    근데 이렇게 티케팅같이, 읽는 도중에 반드시 다른 write가 개입안하게 하려면 `select ... for update`로 테이블 전체 락거는게 아니라, 딱 읽는 row만 lock 걸어서 확실하게 트랜젝션 시작 ~ 끝까지 값이 오염 안되는것 보장하는 것.
                    -    일반적인 글읽기 같은건 `select`로 처리하는데, 재고 값 read해서 +N 하는건 `select ... for update`로 하는게 좋지. 
            3. REPEATABLE READ
                - 트랜젝션의 첫 `select`시 한번만 snapshot을 생성한다
                - 트랜잭션이 시작되고 처음으로 데이터를 읽는 순간에 스냅샷을 만들고, 트랜잭션이 끝날 때까지 그 스냅샷 하나만 계속 재사용합니다. 이로 인해 반복해서 읽어도 항상 동일한 결과가 보장됩니다
                - phantom read 발생
            4. SERIALIZABLE
                - 읽는 모든 데이터에 공유 락(Shared Lock)을 설정합니다.
                - 기본적으로 REPEATABLE READ처럼 동작하지만, SELECT로 읽는 모든 데이터에 공유 락을 걸어버립니다. 다른 트랜잭션은 이 데이터가 락에 걸려있어 수정할 수 없으므로, 데이터 정합성이 가장 강력하게 보장됩니다. MVCC의 장점인 '논블로킹 읽기'가 사실상 불가능해집니다.
                - 데이터 정합성 문제는 없는데, concurrency read & write 성능이 lock 떄문에 엄청 떨어져서 비추 
        - errors related to '데이터 정합성' 
            1. dirty read: 커밋도 안된 데이터 읽어버림
            2. non-repeatable read: 한 트랜젝션 내에 같은 row 두번 읽었는데 결과가 다름
            3. phantom read: 한 트랜젝션 내에 같은 범위로 2번 쿼리했는데, 없던 '유령' row가 생겨남 


## --- 5. storage engine layer again --- 
5. step8. transaction manager & lock manager for concurrency & transaction
    - 디스크에서 가져온 page안 데이터를 읽을 때, 같은 row에 대해서 write transaction, read transaction이 동시에 일어날 때, 서로 blocking 하면 안된다! -> MVCC 
    - MVCC(multi version concurrency control): 
        - `SELECT` 쿼리는 현재 트랜잭션의 격리 수준(Isolation Level)에 맞는 버전의 데이터를 읽어야 합니다. 만약 현재 페이지의 데이터가 다른 미커밋 트랜잭션에 의해 수정되었다면, InnoDB는 **언두 로그(Undo Log)** 를 참조하여 현재 트랜잭션이 볼 수 있는 이전 버전의 데이터를 생성해서 반환합니다. 
        - 이 덕분에 일반적인 SELECT는 다른 트랜잭션을 블로킹하지 않습니다. (Non-locking Read)
    - 락킹(Locking): 
        - 만약 SELECT ... FOR UPDATE나 SELECT ... FOR SHARE 구문이라면, 해당 로우(Row)나 갭(Gap)에 배타 락(Exclusive Lock)이나 공유 락(Shared Lock)을 설정하여 다른 트랜잭션의 접근을 제어합니다.
6. step9. return data 



# C. how write works in b+tree based RDB?

## 핵심 
Redo Log와 Change Buffer는 Read 과정에는 직접적으로 나타나지 않고, Write 과정에서만 등장하는 핵심 컴포넌트

### 1. Redo Log (undo-log랑은 다르네?)

### 2. change buffer 개념 
쓰기 성능을 최적화하는 메커니즘이지만, 읽기에도 간접적인 영향을 줍니다 


## step by step 
`UPDATE my_table SET name = '김영희' WHERE id = 123;` 라고 가정, 

read랑 steps들이 optimizer 단계까지는 같음.\
근데 그 이후에 storage engine -> hardware disk i/o단계가 다름.\
ead의 핵심이 '캐시에서 찾기'라면, Write의 핵심은 '일단 로그에 기록하고, 실제 쓰기 작업은 나중에'이기 때문

Redo Log와 Change Buffer는 느린 디스크 쓰기 작업을 사용자(클라이언트)가 기다리지 않도록 하기 위해,\
스토리지 엔진 내부에서 "일단 로그에만 기록하고(Redo Log),\
변경점은 메모리에 적어뒀다가(Change Buffer),\
나중에 한꺼번에 처리하는" 고도로 최적화된 메커니즘입니다.


### storage engine layer 
- Step 5-1: 데이터 페이지를 버퍼 풀로 로드 및 수정 
    - Read와 마찬가지로, id=123 로우가 담긴 데이터 페이지를 버퍼 풀에서 찾거나, 없다면 디스크에서 읽어옵니다.
    - 그러니까 페이지를 버퍼풀에 올린 다음에, 값을 바꿔버리면 dirty page되니까, 원본데이터를 undo-log(rollback, mvcc 목적)에 먼저 저장한다음 값을 바꾸고, 
- Step 5-2: 리두 로그(Redo Log) 기록 (중요!)
    - 값을 바꿨을 때, 정확히 어떤 페이지에 어떤 라인에 데이터를 뭐로 바꿨다를 redo-log-buffer에 저장해서 sequentially하게 ram에 기록하고, commit 시점에 이 리두 로그 버퍼 내용을 redo-log-file disk에 "순차"저장함.  
        - 그리고 disk에 쓴 redo-log-file이 나중에 어느정도 쌓이면 한번에 배치로 disk i/o write하는 것. 이 땐 random i/o를 해서 훨씬 느림 
        - 결국 write이 빨라진건, random i/o를 나중에 하고, 지금 당장은 disk i/o를 하긴 하는데, sequential write여서 이게 훨씬 빠르니까, 이걸로 일단 떔빵해서 빠른 것. 
- Step 5-3: 체인지 버퍼(Change Buffer) 기록 (중요!)
    - change buffer는 보조 인덱스 전용
    - example 
        1. `UPDATE users SET name = '김영희' WHERE id = 105;` (name 컬럼에 보조 인덱스가 있다고 가정)
        2. id=105인 원본 데이터 페이지를 버퍼 풀에 올리고 '김영희'로 수정합니다 (Dirty Page가 됨).
        3. 이 변경 내역을 리두 로그에 기록합니다. 
        4. 이제 name 인덱스를 수정해야 합니다. 
            - CASE A: name 인덱스 페이지가 버퍼 풀에 있는 경우 
                - 그냥 버퍼 풀에서 해당 인덱스 페이지를 바로 수정합니다. (이 페이지도 Dirty Page가 됨)
            - CASE B: name 인덱스 페이지가 버퍼 풀에 없는 경우 (핵심!)
                - 디스크에서 읽어오는 Random I/O를 하지 않고, "나중에 id=105의 name 인덱스를 '김영희'로 바꿔줘" 라는 변경 내용을 **체인지 버퍼(메모리)** 에 기록하고 작업을 끝냅니다.
        - 결론: 체인지 버퍼는 보조 인덱스 업데이트 시 발생하는 Random I/O를 피하기 위해 변경 내용을 잠시 메모리에 저장해두는 '메모장' 같은 역할을 하는 것입니다.

...

- Step 8: 클라이언트에 응답
    - 리두 로그가 디스크에 기록되면, 실제 데이터 파일 쓰기와 상관없이 트랜잭션은 성공적으로 완료되고 클라이언트는 응답을 받습니다.
- Step 9: 백그라운드 플러싱 (Flushing)
    - 이후 InnoDB의 백그라운드 스레드가 여유로울 때, 버퍼 풀에 있는 '더티 페이지'들을 실제 데이터 파일에 기록하고, '체인지 버퍼'에 쌓인 변경점들을 인덱스 파일에 반영합니다.

### hardware disk i/o layer 



# D. 실제 100~800RPS 테스트의 ec2 & RDS APM 
## RDS의 APM metrics 
![](./stresstest2.png)

### PMM 중요 metrics 
- 주요지표 
    - QPS (query per second)
        - select, update, insert, delete 가 쿼리가 몇번 수행되는지 (쿼리 하나에 여러개 N개 있어도 N개로 침. 물리적인 쿼리 run한 양)
    - TPS (transaction per second)
        - 쿼리 다 실행하고 commit 한 횟수. 논리적인 작업 성공률. 1비즈니스 로직 = 1 TPS라고 봐도 무방할 듯. 
    - Buffer Pool Hit Rate
        - DB read성능의 가장 핵심적인 지표. 보통 cache hit rate 99%이상을 목표로 함. 
        - cache hit 99% 아래면, 버퍼풀 늘리거나, 인덱스 안타는 풀스캔 쿼리가 많다는 뜻이므로(풀스캔 한 잘 안읽는 page를 buffer pool에 넣는 것)
    - Buffer pool usage 
        - 버퍼 풀 사용량 
        - 사용량이 100%에 가깝다면 늘려줘야 되는 시기.
        - 보통 RDS RAM에 50% ~ 70% 까지 할당함 
    - CPU usage 
        - ec2는 cpu usage가 한 70~80%가 적절하고, db는 40%~70% 유지가 적절
- === MySQL Instance Summary (상단) ===
    - QPS (Queries Per Second): 초당 처리되는 쿼리 수. 데이터베이스가 얼마나 많은 작업을 처리하는지 보여주는 핵심 부하 지표입니다.
    - InnoDB Buffer Pool: InnoDB 엔진이 사용하는 메모리 캐시의 총 크기(2.19 G)를 보여줍니다.
    - MySQL Connections: 현재 연결된 클라이언트의 수입니다.
- === thread pool connections ===
    - MySQL Client Thread Activity:
        - Threads_running: 현재 활발하게 쿼리를 실행 중인 스레드의 수입니다.
        - Threads_connected: 현재 연결된 총 스레드 수입니다.
- === query related === 
    - Frequency Histograms & Slow Queries:
        - Com_... (select, insert, update, delete 등): 실행된 쿼리의 유형별 비율을 보여줍니다. 워크로드가 읽기 중심인지 쓰기 중심인지 파악할 수 있습니다.
        - MySQL Slow Queries: long_query_time 설정값을 초과한 느린 쿼리의 수를 보여줍니다.
    - Select Types & Sorts
        - 어떤 종류의 SELECT 쿼리가 실행되었는지, 정렬(Sort) 작업이 어떻게 수행되었는지를 보여줍니다.
        - Select_full_join: JOIN 시 양쪽 테이블 모두에서 풀 스캔이 발생했음을 의미합니다. 인덱스가 전혀 사용되지 않는 최악의 조인 방식입니다 
        - Sort_merge_passes: 정렬할 데이터가 너무 많아 메모리(sort buffer)에 담지 못하고, 임시 디스크 공간을 사용해 여러 번에 걸쳐 정렬(merge sort)했음을 의미합니다
        - Select_full_join이나 Sort_merge_passes 수치가 높게 나타난다면, 이는 인덱스 설계가 잘못되었거나 매우 비효율적인 쿼리가 실행되고 있다는 강력한 증거입니다. 특히 Sort_merge_passes는 디스크 I/O를 유발하므로 반드시 해결해야 할 문제입니다
    - Table Locks & Questions 
        - Table_locks_waited: 테이블 레벨 락(Lock)을 얻기 위해 대기한 횟수입니다.
        - Table_locks_immediate: 대기 없이 즉시 테이블 락을 얻은 횟수입니다.
        - Questions: 클라이언트가 서버로 보낸 총 요청(쿼리) 수입니다.
        - Table_locks_waited 수치가 높다면 테이블 레벨에서 심각한 경합(Contention)이 발생하고 있다는 의미입니다. 주로 MyISAM 스토리지 엔진을 사용하거나, 트랜잭션 중 명시적으로 테이블을 잠글 때 발생하며 병목 현상의 직접적인 원인이 됩니다.
- === network === 
    - mysql network traffic
        - 네트워크가 병목되는 경우는 드물긴 한데, 대용량 데이터 한번에 전송하는 배치작업할 때나 주요하게 봐야하는 메트릭
    - mysql network usage hourly 
- === mysql internals === 
    - mysql internal memory overview
        - 여기서 시스템 전체 RDS에 부여된 메모리가 4gib인데, buffer pool에 할당된 메모리가 2gib를 확인할 수 있다. 
    - top command counters 
        - set_option, select, commit, insert, update 의 비율이 어떻게 되는지 보여준다. 
        - Q. com_set_option이 제일 비율이 높은데, (그 다음은 select) 얘는 query에서 어떤 부분인거지?
        - A. Com_set_option은 실제 SELECT나 INSERT 같은 데이터 조작 쿼리가 아니라, 데이터베이스 연결(Connection)이 생성되거나 초기화될 때 호출되는 내부 설정 명령어. 부하가 늘어서 connection pool에서 새 커넥션 만들 때 호출됨 + idle 상태였던 connection 깨울 때 호출됨. 부하테스트에 RPS가 높다면 이 수치가 높은게 당연한 것.
    - mysql handlers
        - MySQL이 스토리지 엔진 레벨에서 데이터를 어떻게 읽고 쓰는지를 보여주는 저수준(low-level) 카운터입니다.
        - `Handler_read_rnd_next`: 데이터 파일의 처음부터 끝까지 순차적으로 읽은 횟수입니다. 풀 테이블 스캔의 전형적인 지표입니다.
        - `Handler_read_next`: 인덱스 순서에 따라 다음 데이터를 읽은 횟수입니다. (좋음, 인덱스 레인지 스캔)
        - `Handler_read_key`: 인덱스를 통해 정확한 위치의 데이터를 찾은 횟수입니다. (좋음)
        - Handler_read_rnd_next 수치가 Handler_read_key나 Handler_read_next에 비해 비정상적으로 높다면, 인덱스를 타지 못하는 쿼리가 많다는 확실한 증거입니다. EXPLAIN의 type: ALL과 같은 의미를 갖습니다. 
    - mysql transaction handlers
        - 스토리지 엔진 레벨에서 트랜잭션이 어떻게 처리되고 있는지를 보여주는 카운터
        - `Handler_commit`: 트랜잭션이 성공적으로 완료되어 커밋(Commit)된 횟수입니다.
            - 정상 상태 확인: 일반적으로 Handler_commit 수치가 꾸준히 증가하는 것이 정상입니다. 이는 작업들이 성공적으로 데이터베이스에 반영되고 있음을 의미합니다.
        - `Handler_rollback`: 트랜잭션이 실패하거나 명시적으로 취소되어 롤백(Rollback)된 횟수입니다.
            - Handler_rollback 수치가 예상치 못하게 높다면, 애플리케이션 로직에 문제가 있거나, 데드락(Deadlock)이 발생했거나, 제약 조건 위반 등으로 인해 트랜잭션이 계속 실패하고 있다는 강력한 신호입니다. 즉시 원인을 파악하고 해결해야 합니다. 
        - `Handler_savepoint`: 트랜잭션 내에서 SAVEPOINT가 설정된 횟수입니다.
        - `Handler_savepoint_rollback`: SAVEPOINT로 롤백된 횟수입니다
    - process state
        - SHOW FULL PROCESSLIST 명령의 결과를 시각화한 것으로, 각 스레드가 현재 어떤 상태에 있는지를 보여줍니다.
        - Threads_running이 높을 때, 스레드들이 구체적으로 어떤 상태(State 컬럼)에서 시간을 보내는지 파악할 수 있습니다. 예를 들어 Copying to tmp table, Sorting result, locked 등의 상태가 많이 보인다면, 이것이 바로 성능 저하의 원인입니다
    - query cache
        - 과거(mysql 5.8)에 사용되었던 쿼리 캐시 관련 지표입니다
        - mysql8.0 이후 부터는 항상 no data라고 나올 것. 
- === disk level === 
    - mysql file openings
    - mysql open files 
    - mysql table open cache status
        - 의미: 테이블을 열 때 테이블 캐시를 얼마나 효율적으로 사용했는지를 보여줍니다.
        - Opened_tables: 테이블 캐시에 없어 디스크에서 직접 테이블 정의 파일을 열어야 했던 횟수입니다.
        - Opened_tables 수치가 지속적으로 빠르게 증가한다면, table_open_cache와 table_definition_cache 설정값이 작다는 의미일 수 있습니다. 수천 개의 테이블을 사용하는 대규모 환경에서는 이 캐시 크기를 튜닝하는 것이 중요할 수 있습니다
    - mysql open tables
    - mysql table definition cache 
- === 추가해야할 메트릭 ===
    - InnoDB Buffer Pool Hit Rate
        - 얼마나 많은 데이터 읽기를 디스크가 아닌 메모리(버퍼 풀)에서 처리했는지 보여주는 핵심 지표입니다. 99% 이상을 목표로 합니다
        - 보통 innodb_buffer_pool_size 사이즈를 늘려서 read 성능 개선한다. 
        - 자주 사용되지는 않는데 full scan해서 용량만 많이 잡는 쿼리를 찾아 최적화 한다. 
        - 공식: hit rate = (1 - `InnoDB Data Reads` / `InnoDB Buffer Pool Requests`) * 100 
            - `InnoDB Buffer Pool Requests`: 논리적 읽기 요청
                - 쿼리 처리를 위해 InnoDB가 데이터 페이지를 요청한 총 횟수입니다. 
            - `InnoDB Data Reads`: 물리적 디스크 읽기 
                - 요청된 페이지가 버퍼 풀에 없어 디스크에서 직접 읽어온 실패(Miss) 횟수입니다

