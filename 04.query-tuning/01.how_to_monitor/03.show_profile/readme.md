# 0. what 

프로파일링은, 

1. sql query에 각 단계별로 쪼갠 후, 
2. 각 단계별로 몇초 소요되는지 보여줌.

그래서 특정 단계에 소요시간이 예를들어 **0.1초(100ms)** 이상 걸린다? 

그게 **병목구간**이라는 거임.

그게 인덱스를 안걸어서든, join이 비효율적으로 이루어져이든, sort가 비효율적으로 이루어져서든...



example)

1. "sending data" - 개 존나 시간 오래 걸리면 쿼리가 데이터 처리하는데 병목이라는 뜻임. 인덱스 없거나 풀스캔 하고 있을 확률 높음 ㅇㅇ
    - 이걸 돌려보고, "sending data" 부분이 오래걸린다? -> index 추가하셈 
2. "creating tmp table" - 임시 테이블 만드는데 시간 오래 걸리면 GROUP BY, ORDER BY 최적화 해야함 ㅋㅋ 메모리에서 디스크로 넘어가는지 체크해봐라
    - 쿼리에 임시 테이블 만드는구간이 많이 나온다? -> 쿼리 잘못짰을 확률 높음 
3. "copying to tmp table" - 디스크 복사 중인데 이것도 오래 걸리면 sort_buffer_size 늘려봐라 ㅇㅇ
4. "filesort" - ORDER BY가 인덱스 안 타고 메모리에서 정렬 중임. 오래 걸리면 인덱스 추가해봐라 ㅋㅋ
5. global variable 조절하고 적용한거 확인할 때 프로파일링 떠봄 
    - ex. sort_buffer_size, join_buffer_size 같은 설정 튜닝할 때 효과 바로 확인 가능


근데 요새는 다들 EXPLAIN ANALYZE 많이 씀. 더 직관적이라서.

show profile은 deprecated될 예정이라, Performance Schema 써라. 



# 1. 단 쳐봐 
```sql
-- 프로파일링 활성화
SET profiling = 1;

-- 쿼리 실행
SELECT e.emp_no, e.first_name, d.dept_name
FROM employees e
JOIN dept_emp de ON e.emp_no = de.emp_no
JOIN departments d ON de.dept_no = d.dept_no
WHERE e.hire_date > '1990-01-01' and e.hire_date < '1990-12-31';


-- 프로파일 결과 확인
SHOW PROFILES;

-- 특정 쿼리의 상세 프로파일
SHOW PROFILE FOR QUERY 1;

-- CPU와 메모리 사용량까지 확인
SHOW PROFILE CPU, MEMORY FOR QUERY 1;

```


# 2. step by step walk through 

## 2-1. 돌렸던 sql 리스트 떠보자 

```
SHOW PROFILES
```
...하면 실행한 쿼리 리스트 나오고, 몇초 걸렸는지 나옴. 

```sql
--------------------------------------------------------+
| Query_ID | Duration   | Query                                                                                                                                                                                                              |
+----------+------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
|        1 | 0.00030700 | -- 쿼리 실행                                                                                                                                                                                                       |
|        2 | 0.00023300 | SELECT e.emp_no, e.first_name, d.dept_name
FROM employees e
JOIN dept_emp de ON e.emp_no = de.emp_no
JOIN departments d ON de.dept_no = d.dept_no
WHERE e.hire_date > '1990-01-01' and e.hire_date < '1990-12-31'> |
|        3 | 0.00005700 | -- 프로파일 결과 확인                                                                                                                                                                                              |
|        4 | 0.00005300 | -- 특정 쿼리의 상세 프로파일                                                                                                                                                                                       |
|        5 | 0.00006000 | -- CPU와 메모리 사용량까지 확인                                                                                                                                                                                    |
|        6 | 0.00021400 | -- 프로파일링 활성화                                                                                                                                                                                               |
|        7 | 0.00016300 | SET profiling = 1                                                                                                                                                                                                  |
|        8 | 0.00004700 | -- 쿼리 실행                                                                                                                                                                                                       |
|        9 | 0.09541900 | SELECT e.emp_no, e.first_name, d.dept_name
FROM employees e
JOIN dept_emp de ON e.emp_no = de.emp_no
JOIN departments d ON de.dept_no = d.dept_no
WHERE e.hire_date > '1990-01-01' and e.hire_date < '1990-12-31'  |
|       10 | 0.00007900 | -- 프로파일 결과 확인                                                                                                                                                                                              |
```
9번 쿼리가 실제 실행한 쿼리

0.095초 걸렸다는 말. 95ms 

그럼 9번 쿼리를 프로파일링 해보자 

```
SHOW PROFILE FOR QUERY 9
```

## 2-2. 실행한 쿼리를 프로파일링 해보자 

```sql 
mysql> SHOW PROFILE FOR QUERY 1;
+--------------------------------+----------+
| Status                         | Duration |
+--------------------------------+----------+
| starting                       | 0.001281 |
| Executing hook on transaction  | 0.000236 |
| starting                       | 0.000014 |
| checking permissions           | 0.000005 |
| checking permissions           | 0.000003 |
| checking permissions           | 0.000005 |
| Opening tables                 | 0.000593 |
| init                           | 0.000010 |
| System lock                    | 0.000221 |
| optimizing                     | 0.000489 |
| statistics                     | 0.000985 |
| preparing                      | 0.000040 |
| executing                      | 0.101111 | <---- 시간 많이 잡아먹음
| end                            | 0.000010 |
| query end                      | 0.000004 |
| waiting for handler commit     | 0.000007 |
| closing tables                 | 0.000006 |
| freeing items                  | 0.000010 |
| cleaning up                    | 0.000011 |
+--------------------------------+----------+
19 rows in set, 1 warning (0.00 sec)
```

### 2-2-1. Q. 여기서 뭐가 중요함?

0.1s(100ms)이상 걸리는 구간 = 병목구간.


example)

1. "sending data" - 개 존나 시간 오래 걸리면 쿼리가 데이터 처리하는데 병목이라는 뜻임. 인덱스 없거나 풀스캔 하고 있을 확률 높음 ㅇㅇ
    - 이걸 돌려보고, "sending data" 부분이 오래걸린다? -> index 추가하셈 
2. "creating tmp table" - 임시 테이블 만드는데 시간 오래 걸리면 GROUP BY, ORDER BY 최적화 해야함 ㅋㅋ 메모리에서 디스크로 넘어가는지 체크해봐라
    - 쿼리에 임시 테이블 만드는구간이 많이 나온다? -> 쿼리 잘못짰을 확률 높음 
3. "copying to tmp table" - 디스크 복사 중인데 이것도 오래 걸리면 sort_buffer_size 늘려봐라 ㅇㅇ
4. "filesort" - ORDER BY가 인덱스 안 타고 메모리에서 정렬 중임. 오래 걸리면 인덱스 추가해봐라 ㅋㅋ
5. global variable 조절하고 적용한거 확인할 때 프로파일링 떠봄 
    - ex. sort_buffer_size, join_buffer_size 같은 설정 튜닝할 때 효과 바로 확인 가능



## 2-3. CPU, memory 사용량까지 확인해보자 
```sql
mysql> SHOW PROFILE CPU, MEMORY FOR QUERY 1;
+--------------------------------+----------+----------+------------+
| Status                         | Duration | CPU_user | CPU_system |
+--------------------------------+----------+----------+------------+
| starting                       | 0.001281 | 0.000186 |   0.000230 |
| Executing hook on transaction  | 0.000236 | 0.000006 |   0.000101 |
| starting                       | 0.000014 | 0.000011 |   0.000003 |
| checking permissions           | 0.000005 | 0.000004 |   0.000001 |
| checking permissions           | 0.000003 | 0.000001 |   0.000002 |
| checking permissions           | 0.000005 | 0.000004 |   0.000001 |
| Opening tables                 | 0.000593 | 0.000089 |   0.000096 |
| init                           | 0.000010 | 0.000006 |   0.000004 |
| System lock                    | 0.000221 | 0.000012 |   0.000057 |
| optimizing                     | 0.000489 | 0.000027 |   0.000134 |
| statistics                     | 0.000985 | 0.000144 |   0.000406 |
| preparing                      | 0.000040 | 0.000037 |   0.000002 |
| executing                      | 0.101111 | 0.080132 |   0.010604 |
| end                            | 0.000010 | 0.000003 |   0.000007 |
| query end                      | 0.000004 | 0.000002 |   0.000002 |
| waiting for handler commit     | 0.000007 | 0.000006 |   0.000000 |
| closing tables                 | 0.000006 | 0.000006 |   0.000001 |
| freeing items                  | 0.000010 | 0.000007 |   0.000003 |
| cleaning up                    | 0.000011 | 0.000010 |   0.000001 |
+--------------------------------+----------+----------+------------+
19 rows in set, 1 warning (0.00 sec)
```

1. CPU_user: 사용자 공간에서 실행되는 CPU 시간을 의미합니다. 
    - 데이터 처리, 정렬, 조인, 연산 등 데이터베이스 엔진이 직접 수행하는 작업에 사용됩니다.
2. CPU_system: 시스템 콜을 처리하는 커널 공간에서 실행되는 CPU 시간을 의미합니다. 
    - 주로 I/O 작업(디스크 읽기/쓰기), 메모리 할당, 네트워크 작업 등 운영체제 수준의 작업에 사용됩니다.

---\
Q. 어떻게 해석함?


1. CPU 연산 병목: CPU_user 값이 높고 CPU_system 값이 상대적으로 낮으면, 데이터베이스 엔진이 수행하는 계산 작업(**정렬, 조인, 필터링** 등)이 병목임을 의미합니다.
    - ex. "sending data" 단계가 있었다면 그 부분에서 높은 CPU_system 값을 보였을 것입니다.
2. I/O 병목: CPU_system 값이 높거나 CPU_user에 비해 상대적으로 비율이 높으면, 디스크 I/O나 시스템 리소스 접근이 병목임을 의미합니다.
    - 만약 I/O 병목이었다면, CPU_system 값이 더 높거나 CPU_user와 비슷한 수준이었을 것

ex)

```
executing | 0.101111 | 0.080132 | 0.010604
```

- CPU_user(0.080132)가 CPU_system(0.010604)보다 약 7.6배 높습니다.
- 이는 전체 CPU 시간의 약 88%가 사용자 공간에서 소비되었음을 의미합니다.
- 따라서 병목이 주로 데이터 처리 연산(조인, 필터링 등)에 있고, 디스크 I/O는 상대적으로 적은 영향을 미쳤다고 볼 수 있습니다.

