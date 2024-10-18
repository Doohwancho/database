
---\
Objective


ERD 설계시...

1. 확장성(정규화, scale out: sharding & partitioning & replication(master & read replicas))
2. 정합성(ACID, concurrency control(like lock, transaction, MVCC), FK, constraints, Trigger for 정합성 검사)
3. 성능을 위한 반정규화


---\
Concepts


a. erd\
b. rdb modeling tips\
c. vocabs & terms\
d. entity && attributes\
e. keys\
f. modules\
g. relations a.k.a 오리발, 까치발\
h. normalization\
i. de-normalization\
j. data type\
x. constraints\
z. erd-modeling-tips







---\
Todos


a-1. VSC ERD Editor trial :white_check_mark:

b-1. [rdb modeling tip by 이상한나라의 신기한 개발자](https://www.youtube.com/watch?v=T_0TaT0bFHQ) :white_check_mark:

c-1. 용어집 from book - real mysql :white_check_mark:

d-1. entity && attributes from book - real mysql :white_check_mark:

e-1. primary key :white_check_mark:

f-1. 고객-구매-상품 :white_check_mark:\
f-2. 고객-주소 :white_check_mark:\
f-3. 회원-친구 :white_check_mark:\
f-4. [사원-자격증](https://www.youtube.com/watch?v=ga69WBO9Gy0&list=PL9hiYwOHVUQduJN7Pf_kOR8htpJU7K1H8&index=11&ab_channel=Jacob%27sLecture) :white_check_mark:\
f-5. [환자-질병](https://www.youtube.com/watch?v=ga69WBO9Gy0&list=PL9hiYwOHVUQduJN7Pf_kOR8htpJU7K1H8&index=11&ab_channel=Jacob%27sLecture) :white_check_mark:\
f-6. [업체-상품](https://www.youtube.com/watch?v=ga69WBO9Gy0&list=PL9hiYwOHVUQduJN7Pf_kOR8htpJU7K1H8&index=11&ab_channel=Jacob%27sLecture) :white_check_mark:\
f-7. [회원-주문-주문내역-상품](https://www.youtube.com/watch?v=ga69WBO9Gy0&list=PL9hiYwOHVUQduJN7Pf_kOR8htpJU7K1H8&index=11&ab_channel=Jacob%27sLecture) :white_check_mark:\
f-8. [전공-학생-학생의수강과목-과목](https://www.youtube.com/watch?v=ga69WBO9Gy0&list=PL9hiYwOHVUQduJN7Pf_kOR8htpJU7K1H8&index=11&ab_channel=Jacob%27sLecture) :white_check_mark:\
f-9. [이력서-가족사항-학력](https://www.youtube.com/watch?v=jxEYZwhS1gU&list=PL9hiYwOHVUQduJN7Pf_kOR8htpJU7K1H8&index=14&ab_channel=Jacob%27sLecture) :white_check_mark:\
f-10. [공급자-견적서-견적서품목-품목](https://www.youtube.com/watch?v=jxEYZwhS1gU&list=PL9hiYwOHVUQduJN7Pf_kOR8htpJU7K1H8&index=14&ab_channel=Jacob%27sLecture) :white_check_mark:\
f-11. [employee](https://www.youtube.com/watch?v=HXV3zeQKqGY) :white_check_mark:\
f-12. [대화방](https://www.youtube.com/watch?v=42LMmwSFGIE) :white_check_mark:\
f-13. [영화](https://www.youtube.com/watch?v=bBLD07ayLqA&list=PL9hiYwOHVUQduJN7Pf_kOR8htpJU7K1H8&index=18&ab_channel=Jacob%27sLecture) :white_check_mark:\
f-14. [강의개설과 수강신청](https://www.youtube.com/watch?v=Qs_cC6lvx2s&list=PL9hiYwOHVUQduJN7Pf_kOR8htpJU7K1H8&index=20&ab_channel=Jacob%27sLecture) :white_check_mark:\
f-15. [학습과 출석](https://www.youtube.com/watch?v=Qs_cC6lvx2s&list=PL9hiYwOHVUQduJN7Pf_kOR8htpJU7K1H8&index=20&ab_channel=Jacob%27sLecture) :white_check_mark:\
f-16. [교보문고](https://www.youtube.com/watch?v=_TaygcntsJQ&list=PL9hiYwOHVUQduJN7Pf_kOR8htpJU7K1H8&index=21&ab_channel=Jacob%27sLecture) :white_check_mark:\
f-17. [ecommerce by database star](https://www.youtube.com/watch?v=1HamqOuv2Cw&ab_channel=DatabaseStar) :white_check_mark:\
f-18. [ecommerce of gmarket](https://www.slideshare.net/HanSungKim4/db-project-gmarket) :white_check_mark:\
f-19. [sample ecommerce erd from erdcloud](https://www.erdcloud.com/d/hz4NGJK8vb2H9G3jB) :white_check_mark:\
f-20. [Building a Scalable E-Commerce Data Model](https://fabric.inc/blog/ecommerce-data-model/) :white_check_mark:\
f-21. [f-lab, online marketplace](https://github.com/f-lab-edu/online-marketplace) :white_check_mark:\
f-22. 상품주문 구현방법론 비교 :white_check_mark:


g-1. relation, 오리발 까치발 from book - real mysql :white_check_mark:\
g-2. 식별관계(실선) vs 비식별관계(점선) - real mysql :white_check_mark:\
g-2. 상속이 여러번일 땐 자식 pk는 uuid로 해라 - real mysql :white_check_mark:\
g-3. m:m 관계 해소 - real mysql :white_check_mark:\
g-4. 관계 통합 - real mysql :white_check_mark:\
g-5. [cardinality: 1:M 관계, M:N 관계 구분](https://www.youtube.com/watch?v=Yquru9V8HJ8&list=PL9hiYwOHVUQduJN7Pf_kOR8htpJU7K1H8&index=9&ab_channel=Jacob%27sLecture) :white_check_mark:

h-1. 제1 정규화 :white_check_mark:\
h-2. 제2 정규화 :white_check_mark:\
h-3. 제3 정규화 :white_check_mark:

i-1. 반정규화 why and use-case :white_check_mark:

j-1. pk 선택 :white_check_mark:\
j-2. data type 선택 :white_check_mark:\
j-3. column length 선택 :white_check_mark:\
j-4. character-set like utf-8 :white_check_mark:\
j-5. null 보단 not null :white_check_mark:


z-1. erd 그리기 전, 기존 존재하던 앱/서비스의 erd 선행분석하기 :white_check_mark:
z-2. 해당 도메인에 관련 법 확인하기 :white_check_mark:
z-3. ERD 작성 전에는 시스템에서 제공할 기능이나 시나리오를 먼저 작성하기 :white_check_mark:
z-4. 정규화 & 반정규화 해서 테이블 수 많아지는거 문제 안됨
z-etc
z-1. erd를 보고 전체 로직이 보여야 한다. :white_check_mark:
z-2. 마음의 relation :white_check_mark:
z-3. 엔코아 사장, 부사장은 디비의 신이다. :white_check_mark:



---\
More


x-x. [analyze okky modeling, demo](https://vuerd.github.io/)

