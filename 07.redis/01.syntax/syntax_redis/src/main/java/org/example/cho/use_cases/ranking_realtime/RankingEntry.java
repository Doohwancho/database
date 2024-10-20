package org.example.cho.use_cases.ranking_realtime;


import lombok.Value;

@Value
public class RankingEntry {
    private int rank;
    private String userId;
    private Double score;
    
    // Constructor, getters, and setters
}
