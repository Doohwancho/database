---\
Goal


familiarize with redis



---\
Concepts


a. syntax\
b. lock - 재고시스템으로 알아보는 동시성이슈 해결방법(along with redis's lettuce lock, redisson's lock) :white_check_mark:\
c. usecase: rate-limiter :white_check_mark:\
d. usecase: visitor_count :white_check_mark:\
e. usecase: 선착순 100명(FCFS) :white_check_mark:\
f. cache strategy(look aside + write through) :white_check_mark:\
g. usecase: realtime ranking :white_check_mark:\
h. usecase: like :white_check_mark:


---\
Todos


b-0. init: 100개의 쓰레드가 동시에 공유자원을 update 할 때 race condition 발생! :white_check_mark:\
b-1. java - synchronized :white_check_mark:\
b-2. database - pessimistic lock :white_check_mark:\
b-3. database - optimistic lock :white_check_mark:\
b-4. database - named lock :white_check_mark:\
b-5. redis - Lettuce :white_check_mark:\
b-6. redis - Redisson :white_check_mark:\
b-7. database - update query :white_check_mark:\
b-8. performance test of all lock methods :white_check_mark:

c-1. redis로 rate-limiter 구현 :white_check_mark:\
c-2. redis로 인증 재시도시 30초 후 재시도 rate-limit 코드 구현 :white_check_mark:

d-1. redis로 visitor_count 구현 :white_check_mark:

e-1. redis로 선착순 100명 시스템 구현 :white_check_mark:\
e-2. 선착순 100명에 대기큐 기능까지 붙이기 :white_check_mark:

f-1. cache strategy(look aside + write through) :white_check_mark:

---\
Reference


c-1. [rate-limiter in redis](https://github.com/villainscode/Spring-Redis)
