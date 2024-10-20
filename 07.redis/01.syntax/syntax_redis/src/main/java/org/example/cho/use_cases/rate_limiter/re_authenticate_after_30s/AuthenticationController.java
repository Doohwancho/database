package org.example.cho.use_cases.rate_limiter.re_authenticate_after_30s;

import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

@RestController
@RequestMapping("/api/auth")
public class AuthenticationController {
    
    private final AuthenticationService authenticationService;
    
    public AuthenticationController(AuthenticationService authenticationService) {
        this.authenticationService = authenticationService;
    }
    
    @PostMapping("/send-code")
    public ResponseEntity<String> sendAuthCode(@RequestParam String userId) {
        try {
            String authCode = authenticationService.sendAuthCode(userId);
            return ResponseEntity.ok("인증 코드가 발송되었습니다: " + authCode);
        } catch (RuntimeException e) {
            return ResponseEntity.badRequest().body(e.getMessage());
        }
    }
}
