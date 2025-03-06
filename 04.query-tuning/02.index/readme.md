---\
practice


index 최적화 연습 예제 (test_db, employees)

```sql
-- 성별로 급여 차이 분석 (인덱스 최적화 연습용)
SELECT e.gender, AVG(s.salary) as avg_salary
FROM employees e
JOIN salaries s ON e.emp_no = s.emp_no
WHERE s.to_date = '9999-01-01'
GROUP BY e.gender;
```
