package org.example.cho.monitoring;

import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;
import org.springframework.data.redis.core.RedisTemplate;
import org.springframework.stereotype.Service;

@Service
public class UserService {
    private final UserRedisRepository userRedisRepository;
    private final RedisTemplate<String, User> redisTemplate;
    
    public UserService(UserRedisRepository userRedisRepository,
        RedisTemplate<String, User> redisTemplate) {
        this.userRedisRepository = userRedisRepository;
        this.redisTemplate = redisTemplate;
    }
    
    public User getUserById(Long id) {
        // Try to get from Redis first
        User user = userRedisRepository.findById(id);
        if (user != null) {
            return user;
        }
        return user;
    }
    
    public void createUser(User user) {
        userRedisRepository.save(user);
    }
    
    public void bulkCreateUsers(List<User> users) {
        Map<String, User> userMap = users.stream()
            .collect(Collectors.toMap(user -> "user:" + user.getId(), user -> user));
        redisTemplate.opsForValue().multiSet(userMap);
    }
    
//    public void deleteUser(Long id) {
//        userRedisRepository.delete(id);
//    }
}