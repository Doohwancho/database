# 0. what 

Performance Schema랑 Information Schema가 정보는 많은데 개 복잡하고 생 데이터라서 해석하기 어려움 ㅇㅇ 

그래서 sys 스키마 만듬. 쟤네들 더 보기쉽게 가공해서 제공함.

mysql 5.7부터 도입된 기능

# 1. 단 쳐봐 
```sql
-- 호스트별 연결 수 및 평균 latency
SELECT * FROM sys.host_summary;

-- 느린 쿼리 확인
SELECT * FROM sys.statements_with_runtimes_in_95th_percentile;

-- 테이블별 전체 I/O 사용량
SELECT * FROM sys.io_global_by_file_by_bytes;

-- 미사용 인덱스 확인
SELECT * FROM sys.schema_unused_indexes;

-- 중복 인덱스 확인
SELECT * FROM sys.schema_redundant_indexes;
```


# 2. 각 항목 분석
## 2-1. 어느 request 서버가 db 부하를 유독 많이 줌?
```sql
-- 이거 돌려보면 어떤 클라이언트가 DB 터트리고 있는지 바로 알 수 있음
SELECT * FROM sys.host_summary
ORDER BY statement_latency DESC;

-- 더 자세한 정보를 보려면 이거 써라
SELECT * FROM sys.host_summary_by_statement_type
WHERE host != 'background'
ORDER BY total_latency DESC;
```

1. 실무에서는 어떤 서버가 DB 부하 유발하는지 찾을 때 씀. 
2. 특정 API 서버 하나가 이상한 성능 구린 쿼리 날리면 이거로 바로 찾아냄


## 2-2. slow query 찾기 

```sql
-- 95퍼센타일 느린 쿼리 찾기 (employees DB에서 테스트)
SELECT db, exec_count, query,
       format_time(avg_latency) as avg_latency,
       format_time(max_latency) as max_latency
FROM sys.statements_with_runtimes_in_95th_percentile
WHERE db = 'employees'
ORDER BY avg_latency DESC
LIMIT 5;

-- 아니면 풀 스캔 쿼리만 찾기
SELECT * FROM sys.statements_with_full_table_scans
WHERE db = 'employees';
```

실무에서는 매일 이거 확인함.

누가 짠 느린 쿼리 때문에 DB 터질 때 범인 찾는 용도로 씀


## 2-3. disk i/o 병목 찾기 (i/o 사용량 추적)

```sql 
-- 테이블별 I/O 사용량
SELECT file_name, format_bytes(total_read) as total_read,
       format_bytes(total_written) as total_written,
       format_bytes(total) as total
FROM sys.io_global_by_file_by_bytes
WHERE file_name LIKE '%employees%'
ORDER BY total DESC;
```

어떤 테이블이 disk i/o 많이하는지 찾는 용도로 씀.

왜냐면 자주 읽히는 테이블은 캐싱 해둔다음에 disk i/o가 최소화 되야 정상이거든.


## 2-4. 인덱스 관리 
```sql
-- 안 쓰는 인덱스 찾기 (employees DB에서)
SELECT * FROM sys.schema_unused_indexes
WHERE object_schema = 'employees';

-- 중복 인덱스 찾기
SELECT * FROM sys.schema_redundant_indexes
WHERE table_schema = 'employees';

-- 테이블별 인덱스 통계
SELECT * FROM sys.schema_index_statistics
WHERE table_schema = 'employees'
ORDER BY rows_selected DESC
LIMIT 10;
```

- 회사 가면 아무도 인덱스 관리 안 해서 쓸데없는 인덱스 엄청 많음.
- 이거로 "야 이거 왜 만들었냐" 싸움 자주 붙음 


## 2-5. 메모리 사용량 분석

```sql
-- 테이블별 메모리 사용량
SELECT * FROM sys.memory_by_host_by_current_bytes;

-- 테이블별 버퍼 사용량
SELECT object_schema, object_name, allocated, data
FROM sys.innodb_buffer_stats_by_table
WHERE object_schema = 'employees'
ORDER BY allocated DESC;
```

실무에서 db서버에 RAM이 부족하다? 

이걸 돌리면 어느 테이블이 메모리 제일 많이 잡아먹는지 찾을 수 있음 


## 2-6. 테이블 분석 (이슈 테이블 찾기) 

```sql
-- 테이블 전체 통계
SELECT table_schema, table_name, rows_fetched, rows_inserted, 
       rows_updated, rows_deleted, io_read_requests, io_write_requests
FROM sys.schema_table_statistics
WHERE table_schema = 'employees'
ORDER BY (io_read_requests + io_write_requests) DESC;

-- 자주 변경되는 테이블 (INSERT/UPDATE/DELETE 많은 테이블)
SELECT table_schema, table_name, 
       rows_inserted + rows_updated + rows_deleted AS total_changes
FROM sys.schema_table_statistics
WHERE table_schema = 'employees'
ORDER BY total_changes DESC;
```

- 실전에서는 어떤 테이블이 트래픽 많은지 찾는데 씀. 
- 자주 변경되는 테이블은 파티셔닝이나 최적화가 더 필요한 경우가 많음.

