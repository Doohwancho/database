# what 

non clustered idx에서 pk찾고 clustered idx에서 찾는게 'bookmark lookup'이라 하는데, covering idx는 이 과정 스킵해서 빠름. 

왜냐면 non clutered idx에 leaf node에서는 pk만 있는게 아니라 index건 컬럼의 값도 있거든. 

- SELECT 절의 컬럼이 인덱스 컬럼과 같을 때
- 자주 실행되는 쿼리 성능 극대화할 때

...쓴다. 

```sql
-- 인덱스: (last_name, first_name)
CREATE INDEX idx_lastname ON employees(last_name);
CREATE INDEX idx_firstname ON employees(first_name);

-- 이거 인덱스만 보고 결과 반환 가능 (실행계획에 Extra: Using index)
EXPLAIN SELECT last_name, first_name 
FROM employees 
WHERE last_name = 'Smith';
```

example)
```sql
-- 인덱스: idx_lastname_firstname(last_name, first_name)
SELECT first_name FROM employees WHERE last_name = 'Smith';
```
- 인덱스에서 last_name = 'Smith'인 항목들을 찾음
- 인덱스에 first_name이 이미 포함되어 있으므로 그 값을 직접 가져옴
- Clustered Index 조회 단계(bookmark lookup)가 생략됨



example2)
```sql
-- hire_date에 인덱스가 걸려있는 상태에서,
-- 케이스 1: Non-Covering (룩업 필요)
mysql> EXPLAIN SELECT first_name FROM employees WHERE hire_date = '1990-01-01';
+----+-------------+-----------+------------+------+-------------------------+-------------------------+---------+-------+------+----------+-------+
| id | select_type | table     | partitions | type | possible_keys           | key                     | key_len | ref   | rows | filtered | Extra |
+----+-------------+-----------+------------+------+-------------------------+-------------------------+---------+-------+------+----------+-------+
|  1 | SIMPLE      | employees | NULL       | ref  | idx_employees_hire_date | idx_employees_hire_date | 3       | const |   65 |   100.00 | NULL  |
+----+-------------+-----------+------------+------+-------------------------+-------------------------+---------+-------+------+----------+-------+
1 row in set, 1 warning (0.01 sec)
-- Extra: 'Using index condition' 또는 없음

-- 케이스 2: Covering (룩업 불필요)
mysql> EXPLAIN SELECT hire_date FROM employees WHERE hire_date = '1990-01-01';
+----+-------------+-----------+------------+------+-------------------------+-------------------------+---------+-------+------+----------+-------------+
| id | select_type | table     | partitions | type | possible_keys           | key                     | key_len | ref   | rows | filtered | Extra       |
+----+-------------+-----------+------------+------+-------------------------+-------------------------+---------+-------+------+----------+-------------+
|  1 | SIMPLE      | employees | NULL       | ref  | idx_employees_hire_date | idx_employees_hire_date | 3       | const |   65 |   100.00 | Using index |
+----+-------------+-----------+------------+------+-------------------------+-------------------------+---------+-------+------+----------+-------------+
1 row in set, 1 warning (0.00 sec)
-- Extra: 'Using index'
```


