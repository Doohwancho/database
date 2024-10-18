package org.example.cho.선착순;


/*
    Q. 왜 java lock(e.g synchronized)나 mysql/redis lock으로 구현 안함?
    A. 1. java lock은 scale out 환경에서는 못쓴다.
       2. 수만명이 한 곳에 접근해서 lock걸고 read~write 하는 구조는 lock contention 때문에 자원소모도 크고 latency도 길어진다.
       3. queue에 넣고 순서대로 빼는 것, 그리고 일정 수준 이상 이면 안넣는게 lock contention을 피하면서 선착순 시스템 구현하기 적합한 구조이다.
    
    Q. 어떻게 이 코드를 활용?
    A. step1) coupon repository extend하던가 redis만을 위한 repository 만들어서,
       step2) queue에 선착순 100명 유저 저장한 이후,
       step3) 본 작업(e.g 선착순 100명에게 쿠폰 발급한다던가..)을 @Transaction 내에 여러 타 작업과 같이 하면 됨
    
    
    Q. 관련 redis command?
    A. 1. 사용자 등록: LPUSH event:queue user_id
       2. 큐 길이 제한(100명): LTRIM event:queue 0 99
       3. 등록 확인: LLEN event:queue
       
    Q. 특징?
    A. Redis의 단일 스레드 특성을 활용하여 자연스럽게 race condition 방지
    
    Q. 추가 기능?
    A. 1. 대기 목록 관리: 100명 이후의 사용자를 별도의 대기 리스트에 저장
       2. 실시간 순위 표시: Redis Sorted Set을 사용하여 등록 순서와 시간 기록
 */

import org.springframework.data.redis.core.StringRedisTemplate;
import org.springframework.stereotype.Service;

@Service
public class FCFS {
    private final StringRedisTemplate redisTemplate;
    
    public FCFS(StringRedisTemplate redisTemplate) {
        this.redisTemplate = redisTemplate;
    }
    
    //선착순 100명만 통과시키고, 그 이후에 온 유저id는 return Exception
    public boolean registerUser(String userId) {
        Long position = redisTemplate.opsForList().leftPush("event:queue", userId);
        redisTemplate.opsForList().trim("event:queue", 0, 99);
        
        if (position != null && position <= 100) {
            return true; // 등록 성공
        } else {
            redisTemplate.opsForList().remove("event:queue", 0, userId);
            return false; // 등록 실패 (101번째 이후)
        }
    }
}
