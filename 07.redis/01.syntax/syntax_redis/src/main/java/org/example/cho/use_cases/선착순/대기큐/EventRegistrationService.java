package org.example.cho.use_cases.선착순.대기큐;

import org.springframework.data.redis.core.RedisTemplate;
import org.springframework.stereotype.Service;

/*
    Q. what is this code?
    A. queue에 선착순 100명 담은 후, 나머지 인원들은 waiting_list에 넣는 것.
 */

@Service
public class EventRegistrationService {
    
    private final RedisTemplate<String, String> redisTemplate;
    private static final String EVENT_QUEUE = "event:queue";
    private static final String WAITING_LIST = "event:waiting_list";
    private static final long MAX_PARTICIPANTS = 100;
    
    public EventRegistrationService(RedisTemplate<String, String> redisTemplate) {
        this.redisTemplate = redisTemplate;
    }
    
    public String registerUser(String userId) {
        Long position = redisTemplate.opsForList().leftPush(EVENT_QUEUE, userId);
        redisTemplate.opsForList().trim(EVENT_QUEUE, 0, MAX_PARTICIPANTS - 1);
        
        if (position != null && position <= MAX_PARTICIPANTS) {
            return "Registered successfully. Your position: " + position;
        } else {
            // 대기 목록에 추가
            Long waitingPosition = redisTemplate.opsForList().rightPush(WAITING_LIST, userId);
            return "Added to waiting list. Your waiting position: " + waitingPosition;
        }
    }
    
    public Long getRegisteredCount() {
        return redisTemplate.opsForList().size(EVENT_QUEUE);
    }
    
    public Long getWaitingCount() {
        return redisTemplate.opsForList().size(WAITING_LIST);
    }
    
    public String checkStatus(String userId) {
        Long position = redisTemplate.opsForList().indexOf(EVENT_QUEUE, userId);
        if (position != null && position >= 0) {
            return "Registered. Position: " + (position + 1);
        }
        
        Long waitingPosition = redisTemplate.opsForList().indexOf(WAITING_LIST, userId);
        if (waitingPosition != null && waitingPosition >= 0) {
            return "On waiting list. Waiting position: " + (waitingPosition + 1);
        }
        
        return "Not registered";
    }
}