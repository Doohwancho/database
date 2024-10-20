package org.example.cho.use_cases.job_queue._02_redisson;

import java.util.UUID;
import javax.transaction.Transactional;
import org.redisson.api.RBlockingQueue;
import org.redisson.api.RedissonClient;
import org.springframework.stereotype.Service;

@Service
public class JobQueueService {
    
    private final RedissonClient redissonClient;
    private final JobRepository jobRepository;
    private static final String QUEUE_NAME = "jobQueue";
    
    public JobQueueService(RedissonClient redissonClient, JobRepository jobRepository) {
        this.redissonClient = redissonClient;
        this.jobRepository = jobRepository;
    }
    
    @Transactional
    public void enqueueJob(String type, String data) {
        Job job = new Job();
        job.setId(UUID.randomUUID().toString());
        job.setType(type);
        job.setData(data);
        job.setStatus("QUEUED");
        
        jobRepository.save(job);
        
        RBlockingQueue<String> queue = redissonClient.getBlockingQueue(QUEUE_NAME);
        queue.add(job.getId());
    }
    
    @Transactional
    public Job dequeueJob() throws InterruptedException {
        RBlockingQueue<String> queue = redissonClient.getBlockingQueue(QUEUE_NAME);
        String jobId = queue.take(); // This will block until a job is available
        
        Job job = jobRepository.findById(jobId).orElse(null);
        if (job != null) {
            job.setStatus("PROCESSING");
            jobRepository.save(job);
        }
        
        return job;
    }
    
    @Transactional
    public void completeJob(String jobId) {
        Job job = jobRepository.findById(jobId).orElse(null);
        if (job != null) {
            job.setStatus("COMPLETED");
            jobRepository.save(job);
        }
    }
}
