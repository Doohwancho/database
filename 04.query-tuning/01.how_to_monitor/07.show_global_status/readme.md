# 0. what 

서버 전체 상태 확인용도 

# 1. 단 쳐봐

```sql
-- 캐시 히트율 계산
SHOW GLOBAL STATUS LIKE 'Innodb_buffer_pool%';

-- 전체 쿼리 통계
SHOW GLOBAL STATUS LIKE 'Com_%';

-- 쿼리 캐시 상태
SHOW GLOBAL STATUS LIKE 'Qcache%';
```

# 2. 어케씀?

## 2-1. 버퍼 풀 히트율 분석 (캐시 효율성) 

```sql
-- 버퍼 풀 히트율 계산
SHOW GLOBAL STATUS LIKE 'Innodb_buffer_pool%';
```

현업에서는 이 데이터로 버퍼 풀 히트율을 계산함 

```
히트율 = (Innodb_buffer_pool_read_requests - Innodb_buffer_pool_reads) / Innodb_buffer_pool_read_requests * 100
```

1. **히트율이 95% 이하면 innodb_buffer_pool_size를 늘려야 할지 검토한다.**

2. 높은 Innodb_buffer_pool_reads의 문제: 디스크에서 직접 읽기가 많음, 메모리 캐시 부족
    - 해결: innodb_buffer_pool_size 증가 (일반적으로 서버 RAM의 70-80%)

실무에서는 모니터링 대시보드에 이 수치를 항상 표시해두고 추이를 관찰한다.

## 2-2. 쿼리 타입별 실행 빈도 분석 

현업에서는 이 데이터로:

1. SELECT vs UPDATE/INSERT/DELETE 비율을 확인하여 읽기/쓰기 워크로드 분석
2. SELECT 비율이 높으면 읽기 복제본 추가 검토
3. UPDATE/DELETE가 많으면 인덱스 최적화 및 일괄 처리 검토
4. Com_select, Com_update 등의 추이를 시계열로 그래프화하여 비정상 패턴 감지


## 2-3. 쿼리 캐시 효율성 분석
```sql
-- 쿼리 캐시 상태
SHOW GLOBAL STATUS LIKE 'Qcache%';
```

MySQL 8.0에서는 쿼리 캐시가 제거되었지만, 이전 버전에서는:

1. Qcache_hits/(Qcache_hits + Com_select) 비율로 캐시 효율성 측정
2. Qcache_lowmem_prunes가 높으면 쿼리 캐시 크기 증가 고려
3. Qcache_free_blocks와 Qcache_total_blocks 비율로 단편화 확인


## 2-4. 연결 관리 분석
```sql
-- 연결 상태 확인
SHOW GLOBAL STATUS LIKE '%connection%';
SHOW GLOBAL STATUS LIKE 'Threads_%';
```
현업에서는:

1. Max_used_connections/max_connections(설정값) 비율이 85% 이상이면 경고 발생
    - 문제: 연결 풀 소진 위험
    - 해결: 커넥션 풀 최적화, 불필요한 연결 정리, max_connections 증가
2. Aborted_connects가 증가하면 연결 문제 추적
3. Threads_connected와 Threads_running 비율로 활성 쿼리 부하 확인




## 2-5. 테이블 락 및 트랜잭션 분석 
```sql
-- 락 상태 확인
SHOW GLOBAL STATUS LIKE '%lock%';
-- 트랜잭션 상태
SHOW GLOBAL STATUS LIKE 'Innodb_trx%';
```

현업에서는:

1. Innodb_row_lock_waits와 Innodb_row_lock_time_avg로 락 경합 모니터링
2. 값이 급증하면 트랜잭션 패턴 재검토 (긴 트랜잭션, 부적절한 인덱스 등)


## 2-6. 임시 테이블 사용 분석 
```sql
-- 임시 테이블 사용량
SHOW GLOBAL STATUS LIKE 'Created_tmp%';
```

현업에서는:

1. Created_tmp_disk_tables/Created_tmp_tables 비율이 높으면 경고 발생
    - 높은 Created_tmp_disk_tables의 문제: 메모리 내 임시 테이블 공간 부족
    - 해결: tmp_table_size, max_heap_table_size 증가, 복잡한 쿼리 최적화
2. 디스크 기반 임시 테이블이 많으면 tmp_table_size와 max_heap_table_size 조정 검토
3. GROUP BY, ORDER BY 쿼리 최적화 검토
