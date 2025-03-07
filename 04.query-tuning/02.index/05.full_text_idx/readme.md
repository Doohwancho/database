# what 

텍스트 검색 전용 인덱스. LIKE '%키워드%' 같은 와일드카드 검색보다 훨씬 빠름.

- 블로그 포스트, 제품 설명 등 텍스트 검색할 때
- 단어 검색, 자연어 검색 필요할 때

example)

```sql
-- 이건 개느림 (인덱스 못 씀)
SELECT * FROM articles WHERE content LIKE '%mysql%';

-- 전체 텍스트 인덱스 쓰면 개빠름
CREATE FULLTEXT INDEX idx_ft_content ON articles(content);
SELECT * FROM articles WHERE MATCH(content) AGAINST('mysql');
```

