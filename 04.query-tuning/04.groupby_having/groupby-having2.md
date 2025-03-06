---\
What 


test_db(employees) 버전 

group by 최적화 연습 

```sql
-- 입사 연도별 급여 증가율 분석 
SELECT YEAR(e.hire_date) as hire_year,
       AVG(s.salary) as avg_salary
FROM employees e
JOIN salaries s ON e.emp_no = s.emp_no
WHERE s.to_date = '9999-01-01'
GROUP BY hire_year
ORDER BY hire_year;
```
