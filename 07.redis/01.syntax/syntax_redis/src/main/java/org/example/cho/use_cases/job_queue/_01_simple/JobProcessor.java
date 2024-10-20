package org.example.cho.use_cases.job_queue._01_simple;

import org.springframework.boot.autoconfigure.batch.BatchProperties.Job;
import org.springframework.scheduling.annotation.Async;
import org.springframework.stereotype.Component;

@Component
public class JobProcessor {
    
    @Async
    public void processJob(Job job) {
        // 실제 작업 처리 로직
        System.out.println("Processing job: " + job);
        // 예: 이메일 보내기, 알림톡 보내기 등
    }
}