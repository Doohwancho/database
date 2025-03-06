---\
Objective


1. query 분석 
2. 병목제거 (write least io query)


---\
How


문제점 찾는 순서:

1. 제일 느린 쿼리 찾기
2. 어떤 테이블에서 I/O 많이 쓰는지 확인
3. 인덱스 사용 상태 파악
4. 실행 계획 분석
5. 인덱스 추가/수정 후 재측정


example)

```sql
-- 1. 실행 중인 쿼리 확인
SELECT * FROM sys.processlist
WHERE conn_id IS NOT NULL AND command != 'Sleep' AND time > 10
ORDER BY time DESC;

-- 2. 트랜잭션 확인
SELECT * FROM sys.innodb_lock_waits
ORDER BY wait_time DESC;

-- 3. 부하 큰 쿼리 확인
SELECT * FROM sys.statements_with_runtimes_in_95th_percentile
LIMIT 10;

-- 4. I/O 확인
SELECT * FROM sys.io_global_by_file_by_bytes
LIMIT 10;

-- 5. 메모리 사용량 확인
SELECT * FROM sys.memory_global_by_current_bytes
LIMIT 10;
```

이렇게 하면 DB에서 뭐가 문제인지 99% 다 알 수 있음



---\
query optimization 연습 방법론 


처음에는 기본 쿼리 실행 -> 피포먼스 스키마로 결과 분석


ex.
1. run query
2. performance_schema, sys schema로 성능 측정 
```sql
-- 최적화 전/후 비교
SELECT EVENT_ID, TRUNCATE(TIMER_WAIT/1000000000000, 6) as Duration
FROM performance_schema.events_statements_history
ORDER BY EVENT_ID DESC LIMIT 5;

-- 테이블 I/O 랭킹
SELECT * FROM sys.io_global_by_file_by_bytes;

-- 느린 쿼리 탐지
SELECT * FROM sys.statements_with_runtimes_in_95th_percentile;

-- 사용되지 않는 인덱스 찾기
SELECT * FROM sys.schema_unused_indexes;
```
3. 의심가는 부분에 인덱스 추가하거나 join 튜닝하거나 함 
```sql
CREATE INDEX idx_emp_gender ON employees(gender);
CREATE INDEX idx_emp_hire_date ON employees(hire_date);
CREATE INDEX idx_salaries_to_date ON salaries(to_date);
```
4. 다시 performance_schema, sys schema로 성능 측정





---\
concepts


a. setup sample database\
b. 실행계획\
c. index\
d. join\
e. sort\
f. dml



---\
Todos


a-1. [sample database - northwind 예제 for mysql](https://www.geeksengine.com/database/sample/what-is-northwind-database.php)\
a-2. [sample database - sakila 예제 for mysql](https://dev.mysql.com/doc/sakila/en/sakila-structure.html)

b-1. [실행계획 맛보기](https://www.youtube.com/watch?v=_oRsPUqyr10&list=PL9hiYwOHVUQduJN7Pf_kOR8htpJU7K1H8&index=32&ab_channel=Jacob%27sLecture)\
b-2. how to read mysql explain :white_check_mark:\
b-4. sqlchecker :white_check_mark:

c-1. view :white_check_mark:

d-1. subquery vs join (1:M:N:1) 성능 비교 :white_check_mark:

e-1. groupby & having :white_check_mark:
