package org.example.cho.use_cases.job_queue._01_simple;

import org.springframework.boot.autoconfigure.batch.BatchProperties;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RestController;

@RestController
public class JobController {
    
    private final JobQueueService jobQueueService;
    
    public JobController(JobQueueService jobQueueService) {
        this.jobQueueService = jobQueueService;
    }
    
    @PostMapping("/job")
    public String addJob(@RequestBody BatchProperties.Job job) {
        jobQueueService.enqueueJob(job);
        return "Job added to queue";
    }
}