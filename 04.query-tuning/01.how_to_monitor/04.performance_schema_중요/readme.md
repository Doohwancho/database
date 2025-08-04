
# 1. 단 쳐봐 

## step1) on!
```sql
-- 활성화 확인
SHOW VARIABLES LIKE 'performance_schema';
```

## step2) slow query 찾기 

```sql
-- 실행 시간 긴 쿼리들 찾기
SELECT DIGEST_TEXT, COUNT_STAR, AVG_TIMER_WAIT/1000000000 as avg_time_ms
FROM performance_schema.events_statements_summary_by_digest
ORDER BY avg_time_ms DESC
LIMIT 10;

-- employees DB 관련 쿼리만 보고 싶으면
SELECT DIGEST_TEXT, COUNT_STAR, AVG_TIMER_WAIT/1000000000 as avg_time_ms
FROM performance_schema.events_statements_summary_by_digest
WHERE SCHEMA_NAME = 'employees'
ORDER BY avg_time_ms DESC
LIMIT 10;
```

## step3) 테이블 별 I/O 사용량 체크
```sql
-- employees DB 테이블 별 I/O 분석
SELECT OBJECT_SCHEMA, OBJECT_NAME, COUNT_READ, COUNT_WRITE, 
       SUM_TIMER_READ/1000000000 as read_time_ms, 
       SUM_TIMER_WRITE/1000000000 as write_time_ms
FROM performance_schema.table_io_waits_summary_by_table
WHERE OBJECT_SCHEMA = 'employees'
ORDER BY read_time_ms + write_time_ms DESC;
```

결과 
```
+---------------+--------------+------------+-------------+--------------+---------------+
| OBJECT_SCHEMA | OBJECT_NAME  | COUNT_READ | COUNT_WRITE | read_time_ms | write_time_ms |
+---------------+--------------+------------+-------------+--------------+---------------+
| employees     | salaries     |         10 |     2844047 |       4.8454 |     5118.8010 |
| employees     | dept_emp     |    2394967 |      331603 |    1270.8019 |     1331.7610 |
| employees     | titles       |         10 |      443308 |       0.9569 |      917.2769 |
| employees     | departments  |     476756 |           9 |     293.7260 |        0.0391 |
| employees     | employees    |     401961 |           0 |     175.6638 |        0.0000 |
| employees     | dept_manager |         24 |          24 |       1.3586 |        0.3298 |
+---------------+--------------+------------+-------------+--------------+---------------+
6 rows in set (0.01 sec)
```

- OBJECT_SCHEMA: 데이터베이스 스키마 이름 (여기서는 모두 'employees')
- OBJECT_NAME: 테이블 이름
- COUNT_READ: 테이블에서 읽기 연산이 발생한 횟수
- COUNT_WRITE: 테이블에 쓰기 연산이 발생한 횟수
- read_time_ms: 읽기 연산에 소요된 총 시간 (밀리초)
- write_time_ms: 쓰기 연산에 소요된 총 시간 (밀리초)


Q. 어케 해석함?
A. 테이블별 몇번 read/write 됬는지 비율 딱 보면, 어느 테이블에 부하가 많이 걸리는지 파악가능 -> 해당 테이블 관련 sql 뜯어보면서 index/join 튜닝 등 하자.


## step4) index 사용량 분석 

```sql
-- 인덱스 사용량 분석
SELECT OBJECT_SCHEMA, OBJECT_NAME, INDEX_NAME, COUNT_FETCH, 
       COUNT_INSERT, COUNT_UPDATE, COUNT_DELETE
FROM performance_schema.table_io_waits_summary_by_index_usage
WHERE OBJECT_SCHEMA = 'employees'
ORDER BY COUNT_FETCH DESC;
```

```
+---------------+--------------+-------------------------+-------------+--------------+--------------+--------------+
| OBJECT_SCHEMA | OBJECT_NAME  | INDEX_NAME              | COUNT_FETCH | COUNT_INSERT | COUNT_UPDATE | COUNT_DELETE |
+---------------+--------------+-------------------------+-------------+--------------+--------------+--------------+
| employees     | dept_emp     | PRIMARY                 |     2063364 |            0 |            0 |            0 |
| employees     | departments  | PRIMARY                 |      476747 |            0 |            0 |            0 |
| employees     | dept_emp     | dept_no                 |      331603 |            0 |            0 |            0 |
| employees     | employees    | NULL                    |      300025 |            0 |            0 |            0 |
| employees     | employees    | idx_employees_hire_date |      101936 |            0 |            0 |            0 |
| employees     | dept_manager | dept_no                 |          24 |            0 |            0 |            0 |
| employees     | titles       | NULL                    |          10 |       443308 |            0 |            0 |
| employees     | salaries     | NULL                    |          10 |      2844047 |            0 |            0 |
| employees     | departments  | dept_name               |           9 |            0 |            0 |            0 |
| employees     | employees    | PRIMARY                 |           0 |            0 |            0 |            0 |
| employees     | departments  | NULL                    |           0 |            9 |            0 |            0 |
| employees     | dept_manager | PRIMARY                 |           0 |            0 |            0 |            0 |
| employees     | dept_manager | NULL                    |           0 |           24 |            0 |            0 |
| employees     | dept_emp     | NULL                    |           0 |       331603 |            0 |            0 |
| employees     | titles       | PRIMARY                 |           0 |            0 |            0 |            0 |
| employees     | salaries     | PRIMARY                 |           0 |            0 |            0 |            0 |
+---------------+--------------+-------------------------+-------------+--------------+--------------+--------------+
16 rows in set (0.01 sec)
```

- OBJECT_SCHEMA: 데이터베이스 스키마 이름 (모두 'employees')
- OBJECT_NAME: 테이블 이름
- INDEX_NAME: 인덱스 이름 (NULL은 테이블 풀 스캔을 의미)
- COUNT_FETCH: 인덱스를 통한 데이터 읽기 횟수
- COUNT_INSERT: 인덱스에 레코드 삽입 횟수
- COUNT_UPDATE: 인덱스 업데이트 횟수
- COUNT_DELETE: 인덱스에서 레코드 삭제 횟수


Q. 어떻게 해석?

- PK말고 직접 만든 index(ex. idx_employees_hire_date)가 랭킹 상단에 있다? -> 자주 쓰이고 있다 == 유용하다. 안쓰이는 인덱스? == 지워야 한다. 메모리만 차지하고있다.
- index name이 null인데 count_fetch가 30만회라는건, 인덱스 안탄 full scan 했다는 뜻 -> 인덱스 추가 고려해볼까?



```sql
-- 안 쓰이는 인덱스 찾기 (이거 개꿀임)
SELECT OBJECT_SCHEMA, OBJECT_NAME, INDEX_NAME
FROM performance_schema.table_io_waits_summary_by_index_usage
WHERE INDEX_NAME IS NOT NULL
AND COUNT_STAR = 0
AND OBJECT_SCHEMA = 'employees';
```

```
+---------------+--------------+------------+
| OBJECT_SCHEMA | OBJECT_NAME  | INDEX_NAME |
+---------------+--------------+------------+
| employees     | employees    | PRIMARY    |
| employees     | dept_manager | PRIMARY    |
| employees     | titles       | PRIMARY    |
| employees     | salaries     | PRIMARY    |
+---------------+--------------+------------+
4 rows in set (0.00 sec)
```

얘네들 index 삭제하기. 메모리만 처먹음

## step5) 쿼리 실행 계획 문제 파악

```sql
--- 샘플 쿼리. 부서별 평균 급여 
SELECT d.dept_name, AVG(s.salary) as avg_salary
FROM employees e
JOIN dept_emp de ON e.emp_no = de.emp_no
JOIN departments d ON de.dept_no = d.dept_no
JOIN salaries s ON e.emp_no = s.emp_no
WHERE de.to_date = '9999-01-01'  -- 현재 부서만
AND s.to_date = '9999-01-01'     -- 현재 급여만
GROUP BY d.dept_name
ORDER BY avg_salary DESC;
```

```
+--------------------+------------+
| dept_name          | avg_salary |
+--------------------+------------+
| Sales              | 88852.9695 |
| Marketing          | 80058.8488 |
| Finance            | 78559.9370 |
| Research           | 67913.3750 |
| Production         | 67843.3020 |
| Development        | 67657.9196 |
| Customer Service   | 67285.2302 |
| Quality Management | 65441.9934 |
| Human Resources    | 63921.8998 |
+--------------------+------------+
9 rows in set (1.02 sec)
```

```sql
-- 마지막으로 실행한 쿼리 정보 가져오기
SELECT * FROM performance_schema.events_statements_current;
```

하면... 컬럼 겁나 많이 나오는데, 핵심 컬럼만 뽑자. 

```
| SQL_TEXT                            | TIMER_WAIT | LOCK_TIME | ROWS_AFFECTED | ROWS_SENT | ROWS_EXAMINED | NO_INDEX_USED |
|-------------------------------------|------------|-----------|---------------|-----------|---------------|---------------|
| SELECT * FROM performance_schema... | 257000000  | 1000000   | 0             | 0         | 0             | 1             |
```

```sql
SELECT SQL_TEXT, TIMER_WAIT/1000000000 as exec_time_ms, ROWS_EXAMINED, ROWS_SENT,
       CREATED_TMP_TABLES, NO_INDEX_USED, SELECT_SCAN
FROM performance_schema.events_statements_history
ORDER BY EVENT_ID DESC LIMIT 5;
```

```sql
SELECT DIGEST_TEXT, COUNT_STAR, AVG_TIMER_WAIT/1000000000 as avg_time_ms, 
       SUM_ROWS_EXAMINED, SUM_ROWS_SENT, SUM_NO_INDEX_USED
FROM performance_schema.events_statements_summary_by_digest
WHERE SCHEMA_NAME = 'employees'
ORDER BY avg_time_ms DESC LIMIT 5;
```

- 중요 컬럼 
    1. TIMER_WAIT: 쿼리 실행 시간 (단위: 피코초) 
        - 1,000,000,000으로 나누면 밀리초(ms)
    2. LOCK_TIME: 락 획득 기다린 시간
        - 이게 높으면 락 경합 있는거임 ㅇㅇ
    3. ROWS_EXAMINED: 쿼리가 조사한 row 수
        - 이게 ROWS_SENT보다 훨씬 높으면 인덱스 최적화 필요함
        - ex. 100만 줄 조사해서 10줄 리턴하면 망한거임
    4. ROWS_SENT: 실제로 클라이언트에 리턴한 row 수
        - 이거 적은데 ROWS_EXAMINED 높으면 인덱스 추가하셈
    5. NO_INDEX_USED: 인덱스 안 썼으면 1
        - 이거 1이면 풀 테이블 스캔 한거라 거의 대부분 문제임
    6. CREATED_TMP_TABLES, CREATED_TMP_DISK_TABLES: 임시 테이블 생성 횟수
        - 디스크에 생성된거면 메모리 부족해서 개 느린거임 
    7. SELECT_SCAN: 테이블 풀 스캔 횟수
        - 이거 높으면 인덱스 추가하셈
    8. SORT_ROWS: 정렬한 행 수
        9. 이거 높으면 ORDER BY 최적화 필요

---\
실전 해석법 (현업에서 ㄹㅇ 쓰는거)

1. 인덱스 사용 안 하는지 체크
    - NO_INDEX_USED=1 && ROWS_EXAMINED 많으면 인덱스 추가해야함
2. 검사 vs 결과 비율
    - ROWS_EXAMINED/ROWS_SENT 비율이 100:1 넘어가면 심각한 비효율
    - 여기 avg_salary 쿼리는 부서가 9개인데 수백만건 검사했을 가능성 
3. 임시 테이블 사용
    - CREATED_TMP_TABLES > 0이고 CREATED_TMP_DISK_TABLES > 0이면 메모리 부족
    - GROUP BY, ORDER BY에 적절한 인덱스 필요
4. 쿼리 패턴 찾기
    - events_statements_summary_by_digest로 비슷한 쿼리 그룹화해서 문제 패턴 찾기


---\
쿼리를 고쳐보자 


Q. 이 쿼리 어떻게 고침?
```sql
SELECT d.dept_name, AVG(s.salary) as avg_salary
FROM employees e
JOIN dept_emp de ON e.emp_no = de.emp_no
JOIN departments d ON de.dept_no = d.dept_no
JOIN salaries s ON e.emp_no = s.emp_no
WHERE de.to_date = '9999-01-01'  -- 현재 부서만
AND s.to_date = '9999-01-01'     -- 현재 급여만
GROUP BY d.dept_name
ORDER BY avg_salary DESC;
```

1. to_date 컬럼에 인덱스 추가 
```sql
CREATE INDEX idx_dept_emp_to_date ON dept_emp(to_date);
CREATE INDEX idx_salaries_to_date ON salaries(to_date);
```

2. 복합 인덱스로 조인 최적화
```sql
CREATE INDEX idx_salaries_emp_date ON salaries(emp_no, to_date);
CREATE INDEX idx_dept_emp_emp_date ON dept_emp(emp_no, to_date);
```

3. 실행 후 다시 확인
```sql
SELECT SQL_TEXT, TIMER_WAIT/1000000000 as exec_time_ms, ROWS_EXAMINED, ROWS_SENT
FROM performance_schema.events_statements_history
WHERE SQL_TEXT LIKE '%AVG(s.salary)%'
ORDER BY EVENT_ID DESC LIMIT 1;
```

