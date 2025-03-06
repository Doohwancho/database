# 0. what 

테이블 정보는 결국 'INFORMATION_SCHEMA' 조회하는거임.

1. 테이블 & 컬럼	INFORMATION_SCHEMA.COLUMNS
2. 외래키 관계	INFORMATION_SCHEMA.KEY_COLUMN_USAGE
3. 인덱스 정보	INFORMATION_SCHEMA.STATISTICS
4. 뷰 정보	INFORMATION_SCHEMA.VIEWS
5. 트리거 정보	INFORMATION_SCHEMA.TRIGGERS
6. 프로시저 & 함수	INFORMATION_SCHEMA.ROUTINES



# 1. 테이블 정보 

```sql
SELECT 
    TABLE_SCHEMA AS db_name,
    TABLE_NAME AS table_name,
    COLUMN_NAME AS column_name,
    DATA_TYPE AS data_type,
    COLUMN_TYPE AS column_type,
    IS_NULLABLE AS is_nullable,
    COLUMN_KEY AS column_key,
    EXTRA AS extra
FROM INFORMATION_SCHEMA.COLUMNS
WHERE TABLE_SCHEMA NOT IN ('mysql', 'performance_schema', 'information_schema', 'sys')
ORDER BY TABLE_SCHEMA, TABLE_NAME, ORDINAL_POSITION;
```


```
+-----------+----------------------+-------------+-----------+---------------+-------------+------------+-------+
| db_name   | table_name           | column_name | data_type | column_type   | is_nullable | column_key | extra |
+-----------+----------------------+-------------+-----------+---------------+-------------+------------+-------+
| employees | current_dept_emp     | emp_no      | int       | int           | NO          |            |       |
| employees | current_dept_emp     | dept_no     | char      | char(4)       | NO          |            |       |
| employees | current_dept_emp     | from_date   | date      | date          | YES         |            |       |
| employees | current_dept_emp     | to_date     | date      | date          | YES         |            |       |
| employees | departments          | dept_no     | char      | char(4)       | NO          | PRI        |       |
| employees | departments          | dept_name   | varchar   | varchar(40)   | NO          | UNI        |       |
| employees | dept_emp             | emp_no      | int       | int           | NO          | PRI        |       |
| employees | dept_emp             | dept_no     | char      | char(4)       | NO          | PRI        |       |
| employees | dept_emp             | from_date   | date      | date          | NO          |            |       |
| employees | dept_emp             | to_date     | date      | date          | NO          |            |       |
| employees | dept_emp_latest_date | emp_no      | int       | int           | NO          |            |       |
| employees | dept_emp_latest_date | from_date   | date      | date          | YES         |            |       |
| employees | dept_emp_latest_date | to_date     | date      | date          | YES         |            |       |
| employees | dept_manager         | emp_no      | int       | int           | NO          | PRI        |       |
| employees | dept_manager         | dept_no     | char      | char(4)       | NO          | PRI        |       |
| employees | dept_manager         | from_date   | date      | date          | NO          |            |       |
| employees | dept_manager         | to_date     | date      | date          | NO          |            |       |
| employees | employees            | emp_no      | int       | int           | NO          | PRI        |       |
| employees | employees            | birth_date  | date      | date          | NO          |            |       |
| employees | employees            | first_name  | varchar   | varchar(14)   | NO          |            |       |
| employees | employees            | last_name   | varchar   | varchar(16)   | NO          |            |       |
| employees | employees            | gender      | enum      | enum('M','F') | NO          |            |       |
| employees | employees            | hire_date   | date      | date          | NO          |            |       |
| employees | salaries             | emp_no      | int       | int           | NO          | PRI        |       |
| employees | salaries             | salary      | int       | int           | NO          |            |       |
| employees | salaries             | from_date   | date      | date          | NO          | PRI        |       |
| employees | salaries             | to_date     | date      | date          | NO          |            |       |
| employees | titles               | emp_no      | int       | int           | NO          | PRI        |       |
| employees | titles               | title       | varchar   | varchar(50)   | NO          | PRI        |       |
| employees | titles               | from_date   | date      | date          | NO          | PRI        |       |
| employees | titles               | to_date     | date      | date          | YES         |            |       |
+-----------+----------------------+-------------+-----------+---------------+-------------+------------+-------+
```

- 참고 
    - column_key: PRI (Primary Key), MUL (Foreign Key 가능), UNI (Unique Key)
    - extra: auto_increment 여부


# 2. table간 FK 관계 조회 

```sql
SELECT 
    TABLE_SCHEMA AS db_name,
    TABLE_NAME AS table_name,
    COLUMN_NAME AS column_name,
    CONSTRAINT_NAME AS constraint_name,
    REFERENCED_TABLE_NAME AS referenced_table,
    REFERENCED_COLUMN_NAME AS referenced_column
FROM INFORMATION_SCHEMA.KEY_COLUMN_USAGE
WHERE REFERENCED_TABLE_NAME IS NOT NULL
ORDER BY TABLE_SCHEMA, TABLE_NAME;
```

```
+-----------+--------------+-------------+---------------------+------------------+-------------------+
| db_name   | table_name   | column_name | constraint_name     | referenced_table | referenced_column |
+-----------+--------------+-------------+---------------------+------------------+-------------------+
| employees | dept_emp     | emp_no      | dept_emp_ibfk_1     | employees        | emp_no            |
| employees | dept_emp     | dept_no     | dept_emp_ibfk_2     | departments      | dept_no           |
| employees | dept_manager | emp_no      | dept_manager_ibfk_1 | employees        | emp_no            |
| employees | dept_manager | dept_no     | dept_manager_ibfk_2 | departments      | dept_no           |
| employees | salaries     | emp_no      | salaries_ibfk_1     | employees        | emp_no            |
| employees | titles       | emp_no      | titles_ibfk_1       | employees        | emp_no            |
+-----------+--------------+-------------+---------------------+------------------+-------------------+
```

# 3. 모든 테이블의 pk 정보 
```sql
SELECT 
    TABLE_SCHEMA AS db_name,
    TABLE_NAME AS table_name,
    COLUMN_NAME AS primary_key_column
FROM INFORMATION_SCHEMA.KEY_COLUMN_USAGE
WHERE CONSTRAINT_NAME = 'PRIMARY'
ORDER BY TABLE_SCHEMA, TABLE_NAME;
```


# 4. index 정보 
```sql
SELECT 
    TABLE_SCHEMA AS db_name,
    TABLE_NAME AS table_name,
    INDEX_NAME AS index_name,
    COLUMN_NAME AS column_name,
    SEQ_IN_INDEX AS seq_in_index,
    NON_UNIQUE AS non_unique,
    INDEX_TYPE AS index_type,
    COMMENT
FROM INFORMATION_SCHEMA.STATISTICS
WHERE TABLE_SCHEMA NOT IN ('mysql', 'performance_schema', 'information_schema', 'sys')
ORDER BY TABLE_SCHEMA, TABLE_NAME, INDEX_NAME, SEQ_IN_INDEX;
```

```
+-----------+--------------+------------+-------------+--------------+------------+------------+---------+
| db_name   | table_name   | index_name | column_name | seq_in_index | non_unique | index_type | COMMENT |
+-----------+--------------+------------+-------------+--------------+------------+------------+---------+
| employees | departments  | dept_name  | dept_name   |            1 |          0 | BTREE      |         |
| employees | departments  | PRIMARY    | dept_no     |            1 |          0 | BTREE      |         |
| employees | dept_emp     | dept_no    | dept_no     |            1 |          1 | BTREE      |         |
| employees | dept_emp     | PRIMARY    | emp_no      |            1 |          0 | BTREE      |         |
| employees | dept_emp     | PRIMARY    | dept_no     |            2 |          0 | BTREE      |         |
| employees | dept_manager | dept_no    | dept_no     |            1 |          1 | BTREE      |         |
| employees | dept_manager | PRIMARY    | emp_no      |            1 |          0 | BTREE      |         |
| employees | dept_manager | PRIMARY    | dept_no     |            2 |          0 | BTREE      |         |
| employees | employees    | PRIMARY    | emp_no      |            1 |          0 | BTREE      |         |
| employees | salaries     | PRIMARY    | emp_no      |            1 |          0 | BTREE      |         |
| employees | salaries     | PRIMARY    | from_date   |            2 |          0 | BTREE      |         |
| employees | titles       | PRIMARY    | emp_no      |            1 |          0 | BTREE      |         |
| employees | titles       | PRIMARY    | title       |            2 |          0 | BTREE      |         |
| employees | titles       | PRIMARY    | from_date   |            3 |          0 | BTREE      |         |
+-----------+--------------+------------+-------------+--------------+------------+------------+---------+
```

- seq_in_index: 인덱스 내 컬럼 순서
- non_unique: 0이면 Unique, 1이면 중복 가능
- index_type: 인덱스 유형 (BTREE, FULLTEXT, HASH 등)

# 5. view 조회 
```
SELECT 
    TABLE_SCHEMA AS db_name,
    TABLE_NAME AS view_name,
    VIEW_DEFINITION AS definition
FROM INFORMATION_SCHEMA.VIEWS
WHERE TABLE_SCHEMA NOT IN ('mysql', 'performance_schema', 'information_schema', 'sys')
ORDER BY TABLE_SCHEMA, TABLE_NAME;
```


# 6. trigger 조회 

```sql
SELECT 
    TRIGGER_SCHEMA AS db_name,
    TRIGGER_NAME AS trigger_name,
    EVENT_MANIPULATION AS event,
    EVENT_OBJECT_TABLE AS table_name,
    ACTION_TIMING AS timing,
    ACTION_STATEMENT AS definition
FROM INFORMATION_SCHEMA.TRIGGERS
WHERE TRIGGER_SCHEMA NOT IN ('mysql', 'performance_schema', 'information_schema', 'sys')
ORDER BY TRIGGER_SCHEMA, TRIGGER_NAME;

```

# 7. procedure 조회 
```sql
SELECT 
    ROUTINE_SCHEMA AS db_name,
    ROUTINE_NAME AS procedure_name,
    ROUTINE_TYPE AS routine_type,
    DTD_IDENTIFIER AS return_type,
    ROUTINE_DEFINITION AS definition
FROM INFORMATION_SCHEMA.ROUTINES
WHERE ROUTINE_SCHEMA NOT IN ('mysql', 'performance_schema', 'information_schema', 'sys')
ORDER BY ROUTINE_SCHEMA, ROUTINE_NAME;
```


