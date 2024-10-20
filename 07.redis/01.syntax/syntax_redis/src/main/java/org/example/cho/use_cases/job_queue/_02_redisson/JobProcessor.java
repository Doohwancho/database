package org.example.cho.use_cases.job_queue._02_redisson;

import org.springframework.scheduling.annotation.Async;
import org.springframework.stereotype.Service;

@Service
public class JobProcessor {
    
    private final JobQueueService jobQueueService;
    
    public JobProcessor(JobQueueService jobQueueService) {
        this.jobQueueService = jobQueueService;
    }
    
    @Async
    public void processJobs() {
        while (true) {
            try {
                Job job = jobQueueService.dequeueJob();
                if (job != null) {
                    // Process the job
                    System.out.println("Processing job: " + job);
                    // Simulate job processing
                    Thread.sleep(1000);
                    jobQueueService.completeJob(job.getId());
                }
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                break;
            }
        }
    }
}
