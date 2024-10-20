package org.example.cho.use_cases.job_queue._01_simple;

import org.springframework.boot.autoconfigure.batch.BatchProperties.Job;
import org.springframework.data.redis.core.RedisTemplate;
import org.springframework.stereotype.Service;

@Service
public class JobQueueService {
    
    private final RedisTemplate<String, Job> redisTemplate;
    private static final String QUEUE_KEY = "job_queue";
    
    public JobQueueService(RedisTemplate<String, Job> redisTemplate) {
        this.redisTemplate = redisTemplate;
    }
    
    public void enqueueJob(Job job) {
        redisTemplate.opsForList().rightPush(QUEUE_KEY, job);
    }
    
    public Job dequeueJob() {
        return redisTemplate.opsForList().leftPop(QUEUE_KEY);
    }
}