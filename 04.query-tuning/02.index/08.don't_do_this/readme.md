# case1) 인덱스에 함수 끼우지마 
```sql
-- 이런거 인덱스 못 씀
EXPLAIN SELECT * FROM employees WHERE YEAR(birth_date) = 1960;
```

# case2) not equal 쓰지마 
```sql
-- 이것도 대부분 풀 테이블 스캔함
EXPLAIN SELECT * FROM employees WHERE last_name != 'Smith';
```

# case3) like 앞에 % 쓰지마 
```sql
-- 이것도 인덱스 못 씀 ㅋㅋㅋ
EXPLAIN SELECT * FROM employees WHERE last_name LIKE '%mith';
```

# case4) OR 조건 (인덱스 각각 다를 때):
```sql
-- 두 컬럼 각각 인덱스 있어도 OR 쓰면 인덱스 못 쓸수도 있음
EXPLAIN SELECT * FROM employees 
WHERE first_name = 'Georgi' OR last_name = 'Smith';
```
