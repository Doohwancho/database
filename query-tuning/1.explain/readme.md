---\
Goal


practice sql on parts I'm not too familiar with

---\
Config


database: sakila





# A. explain 결과 읽는 법
![](images/2023-04-29-22-14-23.png)



핵심:
1. select_type을 보고 join 이 효율적으로 되는지 확인
2. key를 보고 의도한 인덱스 잘 타는지 확인
3. 같은 목적, 다르게 구현된 query의 rows 보고 비교
4. extra에서 의도하지 않은 테이블 만드는지 확인


## A.1. id
주의 해야 할 것은 id가 테이블의 실행순서를 의미하지는 않는다는 점입니다.

## A.2. select_type
SELECT 문의 유형을 의미합니다. 다음과 같은 종류가 있습니다.

---
performance in order
1. SIMPLE
	- 단순한 SELECT 문
	- 제일 빠르다
2. PRIMARY
	- 서브쿼리를 감싸는 외부 쿼리 or UNION이 포함될 경우 첫번째 SELECT 문
	- The performance depends on the complexity of the inner queries.
3. DERIVED
	- FROM 절에 작성된 서브쿼리
	- it can impact performance as they may require creating temporary tables
4. UNION
	1. UNION, UNION ALL로 합쳐진 SELECT 문
	2. UNION and UNION ALL can affect performance, as they involve combining the results of multiple SELECT statements
5. SUBQUERY
	- 독립적으로 수행되는 서브쿼리(SELECT, WHERE 절에 추가된 서브쿼리)
6. DEPENDENT SUBQUERY : 서브쿼리가 바깥쪽 SELECT 쿼리에 정의된 칼럼을 사용 하는 경우
7. DEPENDENT UNION
	- 외부에 정의된 칼럼을 UNION으로 결합된 쿼리에서 사용하는 경우
8. MATERIALZED
	- These queries create temporary tables for IN clause subqueries and join them, which can impact performance
9. UNCACHEABLE SUBQUERY
	- RAND(), UUID() 등 조회마다 결과가 달라지는 경우
	- caching 못하기 때문에 조회 성능이 나쁘다.



## A.3. type
인덱스 레인지 풀 스캔, 혹은 테이블 풀 스캔을 줄일 수 있는 방향으로 개선해야 합니다.

---
performance in order
1. system
	1. 테이블에 데이터가 없거나 한 개만 있는 경우
2. const
	1. pk나 unique key 를 where절에 넣어서 1개만 가져오는 쿼리
	2. This is very efficient since only one row is accessed.
3. eq_ref
	1. 조인이 수행될 때, join에서 가져오는 테이블의 pk로 검색하는 쿼리
	2. This is efficient since only one row is accessed from the joined table.
4. ref
	1. 일반 인덱스, key(firstname) 같은 애들 where절에 있을 때 발동되는 실행계획
	2. Similar to eq_ref, but the index used is not unique, and more than one row may match the condition. It is still efficient but may need to access multiple rows.
5. range
	1. 인덱스를 범위 스캔
	2. ex. first_name이 인덱스일 때, select * from employees where first_name >= 'A' and first_name <= 'B';
6. index
	1. 인덱스 풀 스캔
7. all
	1. 테이블 풀 스캔. 앵간하면 피해.
	2. 근데 인덱스 잘못 설정하거나, 검색 범위가 전체 데이터의 양의 대략 20%인가? 넘어가면 full scan이 더 빠름.
	3. 근데 얘는 인덱스 커버리지가 50%나 되니까 사용 안함.
	4. 예를들어, gender이 enum('m','f')인 경우, 인덱스가 어짜피 원본 데이터 1/2이고 그거 다 보고 기존 데이터 반틈 참조할 바에야, 원본데이터 full-scan이 더 빠름.
	5. gender에 인덱스 잡아봤자, 내부 엔진이 인덱스 쓰는게 구리다라는걸 판단하고 fullscan 쓴다.

## A.4. key
옵티마이저가 실제로 선택한 인덱스
possible_keys 중에서 하나를 선택한다.


## A.5. rows
SQL문을 수행하기 위해 접근하는 데이터의 모든 행 수


## A.6. filtered
검색한 rows 대비 실제로 우리가 찾던 row가 몇%인지 말해줌.

filtered를 보고, type에 인덱스 타거나 full scan 등 중 어느게 더 효과적인지 알 수 있다.


## A.6. extra

filesort나 group by를 위한 temp 테이블 생성보다 인덱스를 활용하여 sorting/group by를 수행할 수 있다면 성능을 개선할 수 있습니다.

---
performance in order
1.  Using index (Covering Index)
	- This is generally the most efficient because it allows the query to be processed using only the index, without needing to access the underlying data.
2.  Using where
	- This indicates that the query is using a WHERE clause to filter results.
	- The performance depends on the complexity of the filtering condition and the presence of appropriate indexes.
3.  Distinct
	- This is used to remove duplicates from the result set.
	- The performance impact depends on the size of the result set and the presence of appropriate indexes.
4.  Using Filesort
	- This indicates that the query requires sorting, which can be resource-intensive, especially for large result sets.
	- The presence of appropriate indexes can help improve the performance of sorting.
5.  Using temporary
	- This indicates that MySQL needs to create a temporary table to process the query, usually when it involves DISTINCT, GROUP BY, or ORDER BY clauses.
	- Creating a temporary table can be resource-intensive and can lead to performance issues, especially for large data sets.




# Q1. 1:M:N:1 검색


problem: 특정 카테고리 이름(ex. 'Action') 에 속하는 film 검색

## case1) using subquery twice

```sql
EXPLAIN
SELECT SQL_NO_CACHE *
FROM film
WHERE film.film_id IN (
	SELECT film_category.film_id
	FROM film_category
	WHERE film_category.category_id = (
		SELECT category_id
		FROM category
		WHERE category.name = 'Action'
	)
)
```


+-------------------------------------------------+
|                   SQLCHECK                      |
+-------------------------------------------------+
> RISK LEVEL    :: ALL ANTI-PATTERNS
> SQL FILE NAME :: test.sql
> COLOR MODE    :: ENABLED
> VERBOSE MODE  :: ENABLED
> DELIMITER     :: ;
-------------------------------------------------
==================== Results ===================
No issues found.


---
![](images/2023-04-29-20-56-28.png)

1. category 테이블을 subquery로 full scan을 한 듯? 왜 full scan 했는가? category.name을 인덱스 안걸어줬거든.
2. 아무리 full scan이어도 16개 rows밖에 안했는데, 성능이 10배가 차이나는게 놀날 노짜네. extra보면 using where 썼다고 보임.
3. film_category.id를 step1과 비교해서 얻은 row수가 총 64개라 저리 되있는거고. filtered는 여기서 쿼리한거 100%다 가져다 썼다는 거니까, 야물딱지게 검색했다는 뜻이군. (index/full scan 따지는건 다른 얘기겠지만..)
4. film_category의 type이 ref로 되있는데, ref는 where절에 인덱스 있으면 타는 애. extra 컬럼 보면, using where, using index라고 나온다. film_category 테이블의 category_id를 인덱스 걸어줬기 때문.
5. WHERE절에 IN을 쓴건, 내부적으로 INNER JOIN으로 처리된 듯 하다. 아래와 같은 sql을 써도 실행계획 보면 결과값이 똑같다.

```
SELECT *
FROM film AS f
INNER JOIN (
	SELECT film_category.film_id
	FROM film_category
	WHERE film_category.category_id = (
		SELECT category_id
		FROM category
		WHERE category.name = 'Action')
	) AS fc
ON f.film_id = fc.film_id;
```

![](images/2023-04-29-21-11-30.png)

읽는법
1. 왼쪽 아래서 오른쪽 위 순서대로 읽는다.
2. 각 박스 왼쪽위에 적힌 숫자는 cost이다.
3. 각 박스 하단에 bold체로 적혀있는건, SELECT시 사용한 key다.
4. nested loop은 join을 나타낸다.
5. 빨간색일 수록 성능이 안좋고, 초록색일 수록 성능이 좋다.

생각
1. 역시 full scan에 빨간불이 들어왔다. 근데 cost는 1.85밖에 안된다. cost는 io쪽 보다는 cpu쪽 비용인 듯 하다.
2. film_category와 category 테이블 full scan한걸 where절로 필터링 하는데, 이 때, category_id가 인덱스되있어서 얘를 기준으로 찾고, film_category 테이블의 category_id(FK)는 pk가 아니라 'non-unique key' lookup 이라고 나와있다.
3. film 테이블과 film_category 테이블을 INNER JOIN하는데, 인덱스 걸린 행을 연결짓기 때문에(film_id) 인덱스를 타고 조인함 -> hash match에 최적화 되어있어서 빠름. join시 N개의 행을 잇는데, hash match가 O(1)이니까, 결국 O(N)인 것.
4. film table에서 primary key인 (film_id)를 써서 lookup 으로 1 row를 봐서 잘 이어졌나 validation check함.

![](images/2023-05-01-01-01-55.png)

- 오 lock걸린 시간도 나오네?
- rows examined: 16은 db io가 아니라, join후에 이게 정말 맞게 join됬는지 확인하기 위해 16개의 rows를 봤다는거 아닐까?
- film_category 테이블과 film테이블 조인할 때, 한쪽은 pk이고, 다른쪽이 pk가 아닌 인덱스 조인하는데, 후자쪽 테이블이 맞는지 검증하는데 16개를 쓴게 아닐까?
- timing: 972um
	- nested subquery가 join 2번하는 것 보다 느린 이유 case2에서 후술


### feedback

sqlcheck 돌려본 결과, no issue found.


```sql
SELECT *
FROM film AS f
INNER JOIN (
	SELECT film_category.film_id
	FROM film_category
	WHERE film_category.category_id = (
		SELECT category_id
		FROM category
		WHERE category.name = 'Action')
	) AS fc
ON f.film_id = fc.film_id;
```

근데 이 코드도 case1코드랑 결과/성능은 똑같이 뽑히는데, sqlcheck 돌려보면 anti-pattern이라고 이슈 엄청뜬다.

왜지?

다른점은 where in -> join 으로 바꾼건데,
select * + join 안에 nested subquery 넣는게 anti-pattern인가 보다.


+-------------------------------------------------+
|                   SQLCHECK                      |
+-------------------------------------------------+
> RISK LEVEL    :: ALL ANTI-PATTERNS
> SQL FILE NAME :: test.sql
> COLOR MODE    :: ENABLED
> VERBOSE MODE  :: ENABLED
> DELIMITER     :: ;
-------------------------------------------------
==================== Results ===================

-------------------------------------------------
SQL Statement at line 1: [1m[31mselect * from film as f inner join ( select film_category.film_id from
film_category where film_category.category_id = ( select category_id from
category where category.name = 'action') ) as fc on f.film_id = fc.film_id;[0m[39m
[test.sql]: ([1m[32mHIGH RISK[0m[39m) [1m[34mSELECT *[0m[39m

---
● Inefficiency in moving data to the consumer:
When you SELECT *, you're often retrieving more columns from the database than
your application really needs to function. This causes more data to move from
the database server to the client, slowing access and increasing load on your
machines, as well as taking more time to travel across the network. This is
especially true when someone adds new columns to underlying tables that didn't
exist and weren't needed when the original consumers coded their data access.

1. io를 좀 더 줄이고 싶다면, 필요한 column만 가져오라는데..
2. 근데 row를 io한 시점에서 몇개 column 더 가져오는게 속도상 큰 의미가 있으려나?
3. 여하튼 이 피드백은 select *을 임의로 한거니까 넘어간다.

---
● Indexing issues:
Consider a scenario where you want to tune a query to a high level of
performance. If you were to use *, and it returned more columns than you
actually needed, the server would often have to perform more expensive methods
to retrieve your data than it otherwise might. For example, you wouldn't be able
to create an index which simply covered the columns in your SELECT list, and
even if you did (including all columns [shudder]), the next guy who came around
and added a column to the underlying table would cause the optimizer to ignore
your optimized covering index, and you'd likely find that the performance of
your query would drop substantially for no readily apparent reason.

1. select * 한게 인덱스까지 영향을 주나? 아닌거 아냐?


---
● Binding
Problems:
When you SELECT *, it's possible to retrieve two columns of the same name from
two different tables. This can often crash your data consumer. Imagine a query
that joins two tables, both of which contain a column called "ID". How would a
consumer know which was which? SELECT * can also confuse views (at least in some
versions SQL Server) when underlying table structures change -- the view is not
rebuilt, and the data which comes back can be nonsense. And the worst part of it
is that you can take care to name your columns whatever you want, but the next
guy who comes along might have no way of knowing that he has to worry about
adding a column which will collide with your already-developed names.
[Matching Expression: [1m[34mselect *[0m[39m at line 1]

1. select * 했을 때, from에서 다른 테이블도 명시한걸 의도치 않게 가져올 수 있겠구나. 안티패턴 맞네.
2. 유지보수할 때에도 select * 하면 어떤 의도를 가지고 이 query를 썼는지 파악하기 힘들 수 있겠네.

[test.sql]: ([1m[32mHIGH RISK[0m[39m) [1m[34mJOIN Without Equality Check[0m[39m
● Use = with JOIN:
JOIN should always have an equality check to ensure proper scope of records.
[Matching Expression: [1m[34mjoin ( select film_category.film_id from film_category where[0m[39m at line 3]


==================== Summary ===================
All Anti-Patterns and Hints  :: 2
>  High Risk   :: 2
>  Medium Risk :: 0
>  Low Risk    :: 0
>  Hints       :: 0



## case2) using join

```sql
EXPLAIN
SELECT SQL_NO_CACHE f.*
FROM film f
	JOIN film_category fc ON f.film_id = fc.film_id
	JOIN category c ON fc.category_id = c.category_id
WHERE c.name = 'Action';
```


+-------------------------------------------------+
|                   SQLCHECK                      |
+-------------------------------------------------+
> RISK LEVEL    :: ALL ANTI-PATTERNS
> SQL FILE NAME :: test.sql
> COLOR MODE    :: ENABLED
> VERBOSE MODE  :: ENABLED
> DELIMITER     :: ;
-------------------------------------------------
==================== Results ===================
No issues found.


![](images/2023-04-30-03-20-12.png)

1. index사용하는 primary, subquery에서 다 simple로 바뀌었다 -> 성능향상
2. 뭐야 category table에서 fullscan하는건 똑같네?

![](images/2023-04-29-21-10-58.png)

- 100 rows?
	- 16+62 = 78 rows여야 하는데, +22는 어디서 나왔지?
	- 최종 rows수가 64 인데 왜 100이지?
	- case1에서 64처음 나온건 subquery를 where절로 받아 뽑아낸게 64io라는거고, join때 64 나온건, pk로 조인하는건 pk가 유니크하니까, O(N * 1) 해서 이전 64개가 그대로 나온게 아닐까?
	- 그리고 case2에서도 c join fc 해서 100 io 나온 다음, 이걸 f 테이블과 조인시, pk로 조인하니까 똑같이 O(N * 1) 해서 100이 나온거고?
	- 그렇다면, 저 nested loop join이 다 같은 조인이 아니라, pk랑 조인하는건 빨리 뽑히는데, full table scan과 non-unique key로 조인하는건 기존 테이블 rows 수 대비(16+62 = 78) merge할 때 드는 io cost까지 더해진게 100이라는거네?


- cost
	- case2는 join을 2번해서 그런가 cost가 47로, case1대비 18더 높다.
	- 왜? case1는 merge를 1번밖에 안하는데 case2는 merge를 2번하니까. merge cpu 연산 비용이 높나 보다.

![](images/2023-05-01-01-03-57.png)

679um (case1보다 1.5배정도 빠르다. 물론 분산에 단일값 양극단 비교하면 다를 수도 있지만 평균적으로 본 경우)


---
Q. 왜 join 2번한게 nested subquery보다 빠른가?


case1) nested subquery 방식
1. category table을 full scan
2. category_film 테이블에 where로 이어줌
3. film join category_film

case2) join * 2 방식
1. category table을 full scan
2. category_film join category
3. film join category_film


explain visualizer을 보면, case2의 io가 100이고, case1의 io가 64인데도 불구하고, case2가 빠르다.

왜?

join은 optimizer가 잘 쓸 수 있도록 최적화 되어있음(index를 이용한다던가..)\
subquery는 아님.

심지어 nested subquery 쓰면, optimizer가 인덱스 안타고 inner subquery당 each row of outer subquery 를 연결하는 O(N^2) 뜰 수도 있음 -> 성능하락 심각

