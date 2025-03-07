```sql
-- 두 컬럼 모두 자주 사용하는데 단일 인덱스 여러 개 = 비효율
CREATE INDEX idx_lastname ON employees(last_name);
CREATE INDEX idx_firstname ON employees(first_name);

-- 복합 인덱스 하나 = 더 효율적
CREATE INDEX idx_lastname_firstname ON employees(last_name, first_name);
```

## 순서 중요!!!!

```sql
-- 인덱스: (last_name, first_name)

-- 이건 인덱스 완전 사용 가능 (개꿀)
EXPLAIN SELECT * FROM employees WHERE last_name = 'Smith' AND first_name = 'John';

-- 이것도 인덱스 사용 가능 (첫번째 컬럼만)
EXPLAIN SELECT * FROM employees WHERE last_name = 'Smith';

-- 이건 인덱스 사용 불가능 (첫번째 컬럼이 없음 ㅉㅉ)
EXPLAIN SELECT * FROM employees WHERE first_name = 'John';
```
