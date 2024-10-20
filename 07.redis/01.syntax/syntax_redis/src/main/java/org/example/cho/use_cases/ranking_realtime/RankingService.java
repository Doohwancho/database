package org.example.cho.use_cases.ranking_realtime;

import org.springframework.data.redis.core.RedisTemplate;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;

@Service
public class RankingService {
    
    private final RedisTemplate<String, String> redisTemplate;
    private final String RANKING_KEY = "ranking";
    
    public RankingService(RedisTemplate<String, String> redisTemplate) {
        this.redisTemplate = redisTemplate;
    }
    
    @Transactional
    public void incrementScore(String userId, double score) {
        redisTemplate.opsForZSet().incrementScore(RANKING_KEY, userId, score);
    }
    
    public List<RankingEntry> getTopRanking(int limit) {
        Set<String> range = redisTemplate.opsForZSet().reverseRange(RANKING_KEY, 0, limit - 1);
        List<RankingEntry> ranking = new ArrayList<>();
        
        if (range != null) {
            int rank = 1;
            for (String userId : range) {
                Double score = redisTemplate.opsForZSet().score(RANKING_KEY, userId);
                ranking.add(new RankingEntry(rank++, userId, score));
            }
        }
        
        return ranking;
    }
    
    public Long getUserRank(String userId) {
        Long rank = redisTemplate.opsForZSet().reverseRank(RANKING_KEY, userId);
        return rank != null ? rank + 1 : null;
    }
}