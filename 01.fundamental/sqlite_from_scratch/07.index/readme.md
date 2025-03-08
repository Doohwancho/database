---\
Q. index 구현하려면 어케 해야할까?


1. 일단 인덱스 저장할 별도의 b-tree 만듬(각 인덱스마다 만들어야 함. email_idx, username_idx 인덱스 2개 만들었으면 b-tree 2개 필요)
2. 각 인덱스의 b-tree의 leaf 값은 원본 데이터의 pk(rowid)값이 담겨있음. 
3. insert 1 hello hello@google.com 로 저장할 때, 원본 b-tree에만 저장하는게 아니라, 그 후에 그 테이블에 걸려있는 인덱스 b-tree에도 저장해야 함. 
    - 만약 한 테이블에 인덱스가 10개 걸려있으면 write을 원본데이터 저장 + 10번 해야하니까 그만큼 느려지겠지?
    - 그래서 인덱스 막 만들지 말라는거임 write 성능 안좋아지니까. RAM 잡아먹는건 둘째치고 

