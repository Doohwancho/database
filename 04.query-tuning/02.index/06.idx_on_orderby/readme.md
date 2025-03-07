# what 

인덱스는 이미 정렬된 상태로 데이터를 유지하므로, ORDER BY 절과 일치하는 인덱스가 있으면 추가 정렬 작업 없이 이미 정렬된 결과를 바로 가져올 수 있다.

```sql
-- 인덱스 없이 정렬 (Using filesort)
EXPLAIN SELECT emp_no, hire_date FROM employees ORDER BY hire_date;

-- 인덱스 생성
CREATE INDEX idx_hire_date ON employees(hire_date);

-- 인덱스를 활용한 정렬 (Using index)
EXPLAIN SELECT emp_no, hire_date FROM employees ORDER BY hire_date;
```


# cases 

## 다중 컬럼 order by 
```sql
-- 복합 인덱스 생성
CREATE INDEX idx_dept_salary ON salaries(emp_no, salary);

-- 인덱스 순서와 일치하는 정렬 (효율적)
EXPLAIN SELECT emp_no, salary FROM salaries 
WHERE emp_no < 10100
ORDER BY emp_no, salary;

-- 인덱스 순서와 불일치하는 정렬 (비효율적)
EXPLAIN SELECT emp_no, salary FROM salaries 
WHERE emp_no < 10100
ORDER BY salary, emp_no;
```

## 실폐사례 

```sql
-- 인덱스: idx_lastname_firstname(last_name, first_name)

-- 최적화 실패 케이스들:
EXPLAIN SELECT * FROM employees 
WHERE last_name = 'Smith' 
ORDER BY birth_date;  -- 인덱스에 없는 컬럼으로 정렬

EXPLAIN SELECT * FROM employees 
ORDER BY first_name, last_name;  -- 인덱스 컬럼 순서와 불일치
```


