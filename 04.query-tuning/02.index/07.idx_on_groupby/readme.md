# what 

GROUP BY도 ORDER BY와 유사하게 데이터를 특정 컬럼 기준으로 그룹화해야 하므로, 인덱스가 정렬 순서를 제공하여 작업 효율성을 높일 수 있다.

# cases

## 단일 컬럼 groupby 
```sql
-- 인덱스 없이 그룹화 (Using temporary; Using filesort)
EXPLAIN SELECT gender, COUNT(*) FROM employees GROUP BY gender;

-- 인덱스 생성
CREATE INDEX idx_gender ON employees(gender);

-- 인덱스를 활용한 그룹화 (Using index)
EXPLAIN SELECT gender, COUNT(*) FROM employees GROUP BY gender;
```

## 다중 컬럼 groupby 
```sql
-- 복합 인덱스 생성
CREATE INDEX idx_dept_emp_dept_date ON dept_emp(dept_no, to_date);

-- 인덱스 활용 그룹화 쿼리
EXPLAIN SELECT dept_no, to_date, COUNT(*) 
FROM dept_emp 
GROUP BY dept_no, to_date;
```

