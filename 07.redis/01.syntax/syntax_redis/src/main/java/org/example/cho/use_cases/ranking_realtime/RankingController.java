package org.example.cho.use_cases.ranking_realtime;

import java.util.List;
import org.springframework.web.bind.annotation.*;

@RestController
@RequestMapping("/api/ranking")
public class RankingController {
    
    private final RankingService rankingService;
    
    public RankingController(RankingService rankingService) {
        this.rankingService = rankingService;
    }
    
    @PostMapping("/increment")
    public void incrementScore(@RequestParam String userId, @RequestParam double score) {
        rankingService.incrementScore(userId, score);
    }
    
    @GetMapping("/top")
    public List<RankingEntry> getTopRanking(@RequestParam(defaultValue = "10") int limit) {
        return rankingService.getTopRanking(limit);
    }
    
    @GetMapping("/rank/{userId}")
    public Long getUserRank(@PathVariable String userId) {
        return rankingService.getUserRank(userId);
    }
}