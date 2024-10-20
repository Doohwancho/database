package org.example.cho.visitor_count;

import org.springframework.data.redis.core.StringRedisTemplate;
import org.springframework.stereotype.Service;

@Service
public class VisitorTrackingService {
    
    private final StringRedisTemplate redisTemplate;
    
    public VisitorTrackingService(StringRedisTemplate redisTemplate) {
        this.redisTemplate = redisTemplate;
    }
    
    public void trackVisitor(String visitorId) {
        String key = "daily_visitors:" + getCurrentDate();
        redisTemplate.opsForHyperLogLog().add(key, visitorId);
    }
    
    public long getUniqueVisitorCount() {
        String key = "daily_visitors:" + getCurrentDate();
        return redisTemplate.opsForHyperLogLog().size(key);
    }
    
    private String getCurrentDate() {
        // 현재 날짜를 "YYYY-MM-DD" 형식으로 반환하는 메서드
        return java.time.LocalDate.now().toString();
    }
}