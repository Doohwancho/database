
```
-- 이런 쿼리는 개빠름
SELECT * FROM employees WHERE emp_no BETWEEN 10001 AND 10010;
```

1. 왜냐면 pk기준으로 clustered(정렬) 됬거든 
2. 순차 i/o라 빠름 


- pk기준으로 정렬된게 clustered index, b+tree leaf node에 실제 데이터가 담겨있어서 바로 return leaf_node.data 하면 되서 빠름.
- pk말고 특정 컬럼으로 인덱스 만든게 non clustered index인데, 얜 leaf node에 데이터 안담겨있고, 데이터가 담겨있는 row의 pk가 담겨있어서, clustered index 한번 더 찾아야 함. 그래서 좀 더 느림. 
- non clustered idx에서 pk찾고, 그 pk기반으로 clustered idx에 b+Tree에서 찾는 과정을 'bookmark lookup'이라고 한다.

