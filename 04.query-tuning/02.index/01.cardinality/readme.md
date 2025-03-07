# what is cardinality?

uniqueness가 높을수록 cardinality가 높다.

예를들어 male/female은 100만 테이블 절반이 남자, 절반이 여자니까, unique하게 구분 못함 == cardinality 낮음 

cardinality가 높을 수록 index 컬럼 삼으면 좋다.

pk는 clutered라 b+tree leaf node에 바로 데이터 access 되는데,

내가 만든 인덱스는 b+tree leaf node 도달하면 그제서야 id 얻어서 다른 b+tree에 가는데 

바로 가서 한번에 찾아야지 비슷한애들 뭉쳐있으면,

그 뭉쳐있는 애들 또 full scan 때려야 하잖아 

ex) 100만 rows에 M/F가 인덱스면, 인덱스 타도 여전히 50만 rows를 읽어야 하잖아... 비효율적이지 

