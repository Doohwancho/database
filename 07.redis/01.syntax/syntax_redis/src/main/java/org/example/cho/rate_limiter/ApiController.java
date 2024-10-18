package org.example.cho.rate_limiter;

import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestHeader;
import org.springframework.web.bind.annotation.RestController;

@RestController
public class ApiController {
    private final RateLimiter rateLimiter;
    
    public ApiController(RateLimiter rateLimiter) {
        this.rateLimiter = rateLimiter;
    }
    
    @GetMapping("/api/resource")
    public ResponseEntity<String> getResource(@RequestHeader("User-Id") String userId) {
        try {
            if (!rateLimiter.allowRequest(userId, 10, 60)) {
                return ResponseEntity.status(HttpStatus.TOO_MANY_REQUESTS).body("Rate limit exceeded");
            }
            // 실제 리소스 처리 로직
            return ResponseEntity.ok("Resource data");
        } catch (Exception e) {
            // 로깅 추가
            return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).body("An error occurred");
        }
    }
}