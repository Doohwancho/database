---\
practice


subquery 최적화 연습 예제 (test_db, employees)


```sql
-- 부서별 최고 연봉자 
SELECT d.dept_name, e.first_name, e.last_name, s.salary
FROM employees e
JOIN dept_emp de ON e.emp_no = de.emp_no
JOIN departments d ON de.dept_no = d.dept_no
JOIN salaries s ON e.emp_no = s.emp_no
WHERE de.to_date = '9999-01-01'
AND s.to_date = '9999-01-01'
AND (de.dept_no, s.salary) IN (
    SELECT de2.dept_no, MAX(s2.salary)
    FROM dept_emp de2
    JOIN salaries s2 ON de2.emp_no = s2.emp_no
    WHERE de2.to_date = '9999-01-01'
    AND s2.to_date = '9999-01-01'
    GROUP BY de2.dept_no
)
ORDER BY d.dept_name;
```
