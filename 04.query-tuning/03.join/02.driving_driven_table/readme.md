# what 
- A, B 테이블 있을 때
- A가 드라이빙이면: A 테이블 각 레코드마다 B 테이블 찾음
- B가 드라이빙이면: B 테이블 각 레코드마다 A 테이블 찾음


## Q. 어느 테이블이 driving table이면 좋고 어느게 driven table이면 좋음?
1. 작은 데이터셋을 드라이빙으로 선택해야 성능 굿
    - 왜냐면 nested loop join은 2중 for loop인데, table-a: 100 rows, table-b: 100,000 rows 라고 치면,
    - a가 driving table이면 100번만 인덱스 타서 값을 찾으면 되는데, 
    - b가 driving table이면 100,000번 인덱스 타서 값을 찾아야 하니까...
    - 그래서 **driving table은 사이즈가 작아야 하고, driven table은 찾는 컬럼에 인덱스가 걸려있어야 함.**
2. 조인 조건에 인덱스 있는 테이블을 드리븐으로 쓰면 더 굿
    - 그래야 찾을 때 인덱스 타고 찾으니까 효율적이지 



# 1. 단 쳐봐 
```sql
-- 먼저 테이블 사이즈부터 쟤볼까?
select count(*) from employees;   -- 300024
select count(*) from dept_emp;    -- 331603
select count(*) from departments; -- 9


-- employees DB 예제
EXPLAIN FORMAT=JSON 
SELECT e.first_name, d.dept_name
FROM employees e
JOIN dept_emp de ON e.emp_no = de.emp_no
JOIN departments d ON de.dept_no = d.dept_no
WHERE e.emp_no < 10050;
```

```
{
    "query_block": {
    "select_id": 1,
    "cost_info": {
      "query_cost": "46.70"
    },
    "nested_loop": [
      {
        "table": {
          "table_name": "e",
          "access_type": "range",
          "possible_keys": [
            "PRIMARY"
          ],
          "key": "PRIMARY",
          "used_key_parts": [
            "emp_no"
          ],
          "key_length": "4",
          "rows_examined_per_scan": 49,
          "rows_produced_per_join": 49,
          "filtered": "100.00",
          "cost_info": {
            "read_cost": "5.17",
            "eval_cost": "4.90",
            "prefix_cost": "10.07",
            "data_read_per_join": "6K"
          },
          "used_columns": [
            "emp_no",
            "first_name"
          ],
          "attached_condition": "(`employees`.`e`.`emp_no` < 10050)"
        }
      },
      {
        "table": {
          "table_name": "de",
          "access_type": "ref",
          "possible_keys": [
            "PRIMARY",
            "dept_no"
          ],
          "key": "PRIMARY",
          "used_key_parts": [
            "emp_no"
          ],
          "key_length": "4",
          "ref": [
            "employees.e.emp_no"
          ],
          "rows_examined_per_scan": 1,
          "rows_produced_per_join": 54,
          "filtered": "100.00",
          "using_index": true,
          "cost_info": {
            "read_cost": "12.26",
            "eval_cost": "5.42",
            "prefix_cost": "27.74",
            "data_read_per_join": "1K"
          },
          "used_columns": [
            "emp_no",
            "dept_no"
          ]
        }
      },
      {
        "table": {
          "table_name": "d",
          "access_type": "eq_ref",
          "possible_keys": [
            "PRIMARY"
          ],
          "key": "PRIMARY",
          "used_key_parts": [
            "dept_no"
          ],
          "key_length": "16",
          "ref": [
            "employees.de.dept_no"
          ],
          "rows_examined_per_scan": 1,
          "rows_produced_per_join": 54,
          "filtered": "100.00",
          "cost_info": {
            "read_cost": "13.54",
            "eval_cost": "5.42",
            "prefix_cost": "46.70",
            "data_read_per_join": "9K"
          },
          "used_columns": [
            "dept_no",
            "dept_name"
          ]
        }
      }
    ]
  }
} 
```

Q. 어떻게 이 로그에서 driving table과 driven table log 식별함?
A. 맨 처음 나오는게 driving table이고, 순서대로 driven table임.

1. 첫 번째 요소: "table_name": "e" (employees 테이블) - 드라이빙 테이블
2. 두 번째 요소: "table_name": "de" (dept_emp 테이블) - 드리븐 테이블 1
3. 세 번째 요소: "table_name": "d" (departments 테이블) - 드리븐 테이블 2

실행순서 
1. employees 테이블에서 emp_no < 10050 조건에 맞는 행들을 찾음
2. 각 행마다 dept_emp 테이블에서 매칭되는 행을 조인
3. 각 매칭에 대해 departments 테이블에서 최종 매칭을 찾음

"access_type"을 보면 조인 유형이 보인다.

1. employees(e): "access_type": "range" - emp_no < 10050 조건으로 인덱스 범위 스캔
2. dept_emp(de): "access_type": "ref" - employees 테이블의 emp_no를 참조
3. departments(d): "access_type": "eq_ref" - dept_emp의 dept_no를 참조하는 가장 효율적인 조인

