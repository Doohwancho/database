package org.example.cho.use_cases.job_queue._01_simple;
import org.springframework.boot.autoconfigure.batch.BatchProperties.Job;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Component;

@Component
public class JobQueueListener {
    
    private final JobQueueService jobQueueService;
    private final JobProcessor jobProcessor;
    
    public JobQueueListener(JobQueueService jobQueueService, JobProcessor jobProcessor) {
        this.jobQueueService = jobQueueService;
        this.jobProcessor = jobProcessor;
    }
    
    @Scheduled(fixedDelay = 1000) // 1초마다 실행
    public void pollAndProcessJobs() {
        Job job = jobQueueService.dequeueJob();
        if (job != null) {
            jobProcessor.processJob(job);
        }
    }
}
