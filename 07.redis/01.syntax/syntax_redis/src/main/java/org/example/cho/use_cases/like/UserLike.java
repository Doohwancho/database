package org.example.cho.use_cases.like;
public class UserLike {
    public final String postId;
    public final long timestamp;
    
    public UserLike(String postId, long timestamp) {
        this.postId = postId;
        this.timestamp = timestamp;
    }
}