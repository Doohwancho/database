# 0. what 

Information Schema는 메타데이터 보는 거임. DB 자체 정보, 테이블 정보, 실행 중인 쿼리, 락 상태, 다 볼 수 있음


# 1. 단 쳐봐 

## 1-1. 실행 중인 쿼리 모니터링 
```sql 
-- 현재 실행 중인 쿼리 확인
SELECT * FROM information_schema.PROCESSLIST
WHERE COMMAND != 'Sleep'
ORDER BY TIME DESC;
```

```sql
-- 지금 돌고 있는 쿼리 확인
SELECT id, user, host, db, time, state, info 
FROM information_schema.PROCESSLIST 
WHERE command != 'Sleep' 
AND time > 5  -- 5초 이상 실행 중인 쿼리만
ORDER BY time DESC;
```

- 현업에서는 이거 매일 봐야 함. 누가 DB에서 오래 걸리는 쿼리 날리고 있는지 바로 알 수 있음. 
- 특히 time 컬럼 높은 애들 찾아서 죽여야 할 때도 있음 ㅋㅋㅋ

```sql
-- 쿼리 죽이기 (아무나 하면 안됨 ㅋㅋ)
KILL 123;  -- 123은 process_id
```


## 1-2. 트랜젝션 상태 분석(락 문제 해결)

```sql
-- 5초 이상 실행 중인 트랜잭션
SELECT trx_id, trx_started, trx_mysql_thread_id, trx_query, trx_operation_state, 
       trx_tables_in_use, trx_tables_locked, trx_rows_locked, trx_rows_modified
FROM information_schema.INNODB_TRX
WHERE trx_state = 'RUNNING'
AND TIMESTAMPDIFF(SECOND, trx_started, NOW()) > 5
ORDER BY trx_started;
```

- 실무에서 DB 느려지면 이거 먼저 봄. 
- 락 걸린 트랜잭션 있으면 다른 쿼리 다 대기하게 되니까 빨리 찾아서 해결해야 함.

## 1-3. 저장공간 관리

```sql
-- employees DB 테이블 크기 확인
SELECT table_schema, table_name, 
       ROUND(data_length/1024/1024,2) AS data_size_mb,
       ROUND(index_length/1024/1024,2) AS index_size_mb,
       ROUND((data_length+index_length)/1024/1024,2) AS total_size_mb,
       table_rows
FROM information_schema.TABLES
WHERE table_schema = 'employees'
ORDER BY total_size_mb DESC;
```

- 서버 용량 부족할 때 이거 먼저 확인함. 
- 어떤 테이블이 공간 많이 차지하는지 알 수 있음. 
- 실제 현업에서는 TB 단위로 커지는 테이블도 많아서 모니터링 필수임.


## 1-4. 인덱스 정보 확인(튜닝 목적)
```sql
-- 중복 인덱스 찾기 (인덱스 앞부분이 같으면 중복임)
SELECT t.table_schema, t.table_name, 
       s.index_name, s.column_name, s.seq_in_index
FROM information_schema.statistics s
JOIN information_schema.tables t 
  ON s.table_schema = t.table_schema AND s.table_name = t.table_name
WHERE t.table_schema = 'employees'
ORDER BY t.table_schema, t.table_name, s.index_name, s.seq_in_index;
```

- 인덱스 정리할 때 필수임. 
- 중복 인덱스 많으면 쓸데없이 공간 낭비되고 INSERT/UPDATE 느려짐.


## 1-5. 외래키 관계 확인 (스키마 분석) 

```sql
-- 외래키 관계 확인
SELECT table_name, column_name, constraint_name, 
       referenced_table_name, referenced_column_name
FROM information_schema.key_column_usage
WHERE referenced_table_name IS NOT NULL
AND table_schema = 'employees'
ORDER BY table_name;
```

- 처음 들어간 회사 DB 분석할 때 이거 꼭 돌려봄. 
- ERD 다이어그램 없을 때 이걸로 테이블 관계 파악 가능.



