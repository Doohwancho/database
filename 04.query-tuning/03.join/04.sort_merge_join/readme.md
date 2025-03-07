# what 

table a join table b 시에,

1. 먼저 join column 기준으로 두 테이블 모두 merge sort 한다 
2. 그 이후에 join 한다 


Q. 언제 씀?

- 일반적인 상황에서는 잘 안쓰임. 대부분 다른 join들이 쓰임. 상황에 맞아야 쓰임
- 보통 대용량 데이터 join인데, equal join이 아니라 <, >같은 범위조인할 떄 쓰인다. (대용량 테이블 join인데 equal join은 hash join 씀)
- 왜냐면 범위 조인은 정렬된 상태에서 찾는게 효율적인데 sort merge join은 이미 정렬해논 상태라 더 효율적임 
- 아니면 보통 nested loop join 쓰는데, driven table에 적절한 index가 없어서 너무 비효율적일 떄 쓰기도 한다. 


Q. 왜 좋음?

미리 정렬하는 과정이 있기 때문에, 정렬된 애들을 순서대로 가져오는 범위조인하는 상황에서 유리함 


Q. 단점은?

정렬한 다음에 PGA영역에 올려놓는데, 테이블 너무 커서 PGA가 꽉찬다? -> swap memory 씀 -> 느려짐 

그러면 이 조인 쓸 때 db 시스템에 pga 영역 메모리가 얼마나 찼는지 모니터링 해야겠네?
