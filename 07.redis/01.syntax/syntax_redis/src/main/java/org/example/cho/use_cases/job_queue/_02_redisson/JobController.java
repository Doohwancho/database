package org.example.cho.use_cases.job_queue._02_redisson;

import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

@RestController
public class JobController {
    
    private final JobQueueService jobQueueService;
    
    public JobController(JobQueueService jobQueueService) {
        this.jobQueueService = jobQueueService;
    }
    
    @PostMapping("/job")
    public String enqueueJob(@RequestParam String type, @RequestParam String data) {
        jobQueueService.enqueueJob(type, data);
        return "Job enqueued successfully";
    }
}