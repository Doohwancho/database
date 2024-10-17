package org.example.cho.lock.repository;

import java.time.Duration;
import org.springframework.data.redis.core.RedisTemplate;
import org.springframework.stereotype.Component;

//case6) redis - Lettuce
@Component
public class RedisLockRepository {
    
    private RedisTemplate<String, String> redisTemplate;
    
    public RedisLockRepository(RedisTemplate<String, String> redisTemplate) {
        this.redisTemplate = redisTemplate;
    }
    
    public Boolean lock(Long key) {
        return redisTemplate
            .opsForValue()
            .setIfAbsent(generateKey(key), "lock", Duration.ofMillis(3_000)); //named lock처럼 key:value에서, value에  "lock"을 걸고, 반환하는 방식.
    }
    
    public Boolean unlock(Long key) {
        return redisTemplate.delete(generateKey(key));
    }
    
    private String generateKey(Long key) {
        return key.toString();
    }
}
