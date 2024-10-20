package org.example.cho.monitoring;

import java.util.concurrent.TimeUnit;
import org.springframework.data.redis.core.RedisTemplate;
import org.springframework.stereotype.Repository;

@Repository
public class UserRedisRepository {
    
    private final RedisTemplate<String, User> redisTemplate;
    private static final String KEY_PREFIX = "user:";
    
    public UserRedisRepository(RedisTemplate<String, User> redisTemplate) {
        this.redisTemplate = redisTemplate;
    }
    
    public void save(User user) {
        String key = KEY_PREFIX + user.getId();
        redisTemplate.opsForValue().set(key, user, 1, TimeUnit.HOURS);
    }
    
    public User findById(Long id) {
        String key = KEY_PREFIX + id;
        return (User) redisTemplate.opsForValue().get(key);
    }
    
    public void delete(Long id) {
        String key = KEY_PREFIX + id;
        redisTemplate.delete(key);
    }
}
