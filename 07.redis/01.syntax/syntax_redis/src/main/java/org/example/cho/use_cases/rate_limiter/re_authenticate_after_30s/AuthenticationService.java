package org.example.cho.use_cases.rate_limiter.re_authenticate_after_30s;

import org.springframework.data.redis.core.RedisTemplate;
import org.springframework.stereotype.Service;
import java.util.concurrent.TimeUnit;

@Service
public class AuthenticationService {
    
    private final RedisTemplate<String, String> redisTemplate;
    
    public AuthenticationService(RedisTemplate<String, String> redisTemplate) {
        this.redisTemplate = redisTemplate;
    }
    
    public boolean canSendAuthCode(String userId) {
        String key = "auth-text:" + userId;
        Boolean isKeySet = redisTemplate.opsForValue().setIfAbsent(key, "1", 30, TimeUnit.SECONDS);
        //1. Redis에 키가 없으면 키를 생성하고 값을 설정합니다 (인증 시도를 기록).
        //2. 키가 이미 존재하면 아무 작업도 수행하지 않습니다 (이미 인증 시도가 있었음을 의미).
        //3. 키가 생성되면 30초의 만료 시간을 설정합니다.
        
        return Boolean.TRUE.equals(isKeySet);
    }
    
    public String sendAuthCode(String userId) {
        if (canSendAuthCode(userId)) {
            String authCode = generateAuthCode();
            // 실제 인증 코드 전송 로직 구현
            return authCode;
        } else {
            throw new RuntimeException("인증 문자를 보내기 위해서는 1분 후에 다시 시도해주세요.");
        }
    }
    
    private String generateAuthCode() {
        // 인증 코드 생성 로직 구현
        return "123456"; // 예시 코드
    }
}