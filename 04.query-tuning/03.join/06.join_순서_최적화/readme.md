# what 

여러 테이블 조인할 때 어떤 순서로 조인할지가 성능 크게 좌우함 ㅇㅇ 옵티마이저가 결정하지만 가끔 삽질함.

최적 조인 순서 

1. 필터링으로 작은 결과셋 나오는 테이블 먼저
2. 조인했을 때 rows수가 최소한 되는 순서대로 해야 NL join시 유리한 듯 


# case 
## 1) 작은 결과셋 먼저 필터링 
```sql
-- 이렇게 하면 employees 테이블 필터링부터 해서 빠름
SELECT e.first_name, d.dept_name
FROM employees e
JOIN dept_emp de ON e.emp_no = de.emp_no
JOIN departments d ON de.dept_no = d.dept_no
WHERE e.birth_date > '1965-01-01';
```

e가 driving table인데, 먼저 where절로 필터링 해서 작은 데이터 만든 다음에 NL join 하는게 더 빠르다.

## 2) join 전에 subquery 로 작은 데이터로 필터링 먼저 한 후에, 조인하면 좋음 
```sql
-- 대용량 조인할 때 필터링 먼저
SELECT e.first_name, s.salary
FROM (SELECT * FROM employees WHERE birth_date > '1965-01-01') e
JOIN salaries s ON e.emp_no = s.emp_no
WHERE s.to_date = '9999-01-01';
```



# example 
```sql
select count(*) from employees; -- 300024
select count(*) from dept_emp; -- 331603
select count(*) from departments; -- 9
select count(*) from titles; -- 443308
```

![](../../00.sample-database/test_db/erd/employees-schema.png)



```sql
-- case1) 기본 조인 순서 (옵티마이저 결정)
-- cost: 149182.95
EXPLAIN 
SELECT e.first_name, d.dept_name, t.title
FROM employees e
JOIN dept_emp de ON e.emp_no = de.emp_no
JOIN departments d ON de.dept_no = d.dept_no
JOIN titles t ON e.emp_no = t.emp_no
WHERE e.birth_date > '1965-01-01';


-- case2) cost: 167569.51
EXPLAIN 
SELECT /*+ JOIN_ORDER(e, t, de, d) */ e.first_name, d.dept_name, t.title
FROM employees e
JOIN dept_emp de ON e.emp_no = de.emp_no
JOIN departments d ON de.dept_no = d.dept_no
JOIN titles t ON e.emp_no = t.emp_no
WHERE e.birth_date > '1965-01-01';
```

Q. 왜 titles를 먼저 조인하는게 성능이 더 구리지?

employees에서 e.birth_date로 먼저 33%정도 필터링 한 후에, titles를 join해도 어짜피 같은 rows 숫자 아닌가?

titles를 먼저 join하나, dept_emp, departments를 먼저 join하나 똑같은거 아닐까?


Case 1 (기본 순서: e → de → d → t)

1. e(필터 후): 99,679행
2. e + de: 110,174행
3. e + de + d: 110,174행 (변화 없음)
4. e + de + d + t: 163,255행

Case 2 (힌트 순서: e → t → de → d)

1. e(필터 후): 99,679행
2. e + t: 147,704행
3. e + t + de: 163,255행
4. e + t + de + d: 163,255행


아~ titles에 중복되는 employee들이 많아서 rows수가 늘어나니까, 

join 중간에 NL조인할 떄 driving table의 사이즈가 커져서 

효율이 더 안좋아지는구나!
