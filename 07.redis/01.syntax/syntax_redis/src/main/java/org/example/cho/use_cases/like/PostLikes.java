package org.example.cho.use_cases.like;

import java.util.Set;

public class PostLikes {
    public final long likeCount;
    public final Set<String> likers;
    
    public PostLikes(long likeCount, Set<String> likers) {
        this.likeCount = likeCount;
        this.likers = likers;
    }
}