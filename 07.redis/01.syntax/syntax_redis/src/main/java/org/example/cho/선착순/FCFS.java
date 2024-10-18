package org.example.cho.선착순;


/*
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
