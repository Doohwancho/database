package org.example.cho.visitor_count;

import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

@RestController
public class VisitorController {
    
    private final VisitorTrackingService visitorTrackingService;
    
    public VisitorController(VisitorTrackingService visitorTrackingService) {
        this.visitorTrackingService = visitorTrackingService;
    }
    
    @PostMapping("/track")
    public void trackVisitor(@RequestParam String visitorId) {
        visitorTrackingService.trackVisitor(visitorId);
    }
    
    @GetMapping("/count")
    public long getUniqueVisitorCount() {
        return visitorTrackingService.getUniqueVisitorCount();
    }
}