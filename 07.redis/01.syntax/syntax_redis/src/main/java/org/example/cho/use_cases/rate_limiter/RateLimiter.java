package org.example.cho.use_cases.rate_limiter;

import java.util.List;
import java.util.concurrent.TimeUnit;
import org.springframework.dao.DataAccessException;
import org.springframework.data.redis.core.RedisOperations;
import org.springframework.data.redis.core.SessionCallback;
import org.springframework.data.redis.core.StringRedisTemplate;
import org.springframework.stereotype.Service;

@Service
public class RateLimiter {
    private final StringRedisTemplate redisTemplate;
    
    public RateLimiter(StringRedisTemplate redisTemplate) {
        this.redisTemplate = redisTemplate;
    }
    
    public boolean allowRequest(String userId, int maxRequests, int timeWindowSeconds) {
        String key = "rate_limit:" + userId;
        long currentTime = System.currentTimeMillis() / 1000;
        
        List<Object> txResults = redisTemplate.execute(new SessionCallback<List<Object>>() {
            @Override
            public List<Object> execute(RedisOperations operations) throws DataAccessException {
                operations.multi(); //step1) transaction의 시작점
                operations.opsForZSet().add(key, String.valueOf(currentTime), currentTime); //step2) 현재시간을 sortedset에 추가
                operations.opsForZSet().removeRangeByScore(key, 0, currentTime - timeWindowSeconds); //step3) 시간 윈도우 밖 오래된 요청들 제거
                operations.opsForZSet().size(key); //step4) set의 크기(요청수)를 가져옴
                return operations.exec(); //step5) 트랜젝션 실행
            }
        });
        
        // step6) 트랜잭션 결과 검증
        if (txResults == null || txResults.isEmpty()) {
            throw new RuntimeException("Redis transaction failed");
        }
        
        // step7) 마지막 결과(size 연산)가 Long 타입인지 확인
        if (!(txResults.get(txResults.size() - 1) instanceof Long)) {
            throw new RuntimeException("Unexpected result type from Redis transaction");
        }
        
        // step8) 요청 수를 가져온다.
        Long requestCount = (Long) txResults.get(txResults.size() - 1);
        
        // step9) 키의 만료 시간 설정 (트랜잭션 외부에서 수행)
        redisTemplate.expire(key, timeWindowSeconds, TimeUnit.SECONDS);
        
        // step10) 요청 수가 최대 허용 요청 수 이하인지 여부를 반환합니다.
        return requestCount <= maxRequests;
    }
}