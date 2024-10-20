package org.example.cho.use_cases.like;

import java.util.Arrays;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.data.redis.core.RedisCallback;
import org.springframework.data.redis.core.RedisTemplate;
import org.springframework.stereotype.Service;

import java.util.Set;
import java.util.stream.Collectors;

/*
    Q. why add like-feature using redis not mysql?
    A. 좋아요, 조회수같이 read, update가 빠른 기능들은 db에서 io하는 것 보다, RAM에서 처리하는게 더 빠르고 db 자원을 더 아낄 수 있다.
    
    Q. 구현 기능 리스트?
    A.
    1. likePost: Adds a like to a post and records it for the user.
    2. unlikePost: Removes a like from a post and the user's liked posts.
    getPostLikes: Retrieves the number of likes and the list of users who liked a post.
    getUserLikes: Retrieves the posts liked by a user, sorted by time.
    isPostLikedByUser: Checks if a user has liked a specific post.
    getCommonLikers: Finds users who have liked multiple specified posts.
    getMostLikedPosts: Retrieves the most liked posts.
 */

@Service
public class LikeService {
    
    private final RedisTemplate<String, String> redisTemplate;
    
    @Autowired
    public LikeService(RedisTemplate<String, String> redisTemplate) {
        this.redisTemplate = redisTemplate;
    }
    
    //좋아요 버튼 누를 때, 레디스에 좋아요+1 하고 어떤 유저가 어떤 post에 언제 좋아요 눌렀는지 write
    public boolean likePost(String userId, String postId) {
        String postLikesKey = "like:post:" + postId;
        String userLikesKey = "user:likes:" + userId;
        
        //Q. why RedisCallback over SessionCallback?
        //A. you use RedisCallback for fine-grained multiple redis operations
        //   you use SessionCallback works same except it has transaction.
        //   you don't need transaction for like-feature, therefore you use RedisCallback over SessionCallback
        
        return redisTemplate.execute((RedisCallback<Boolean>) connection -> {
            connection.openPipeline();
            connection.sAdd(postLikesKey.getBytes(), userId.getBytes());
            connection.zAdd(userLikesKey.getBytes(), System.currentTimeMillis(), postId.getBytes());
            var results = connection.closePipeline();
            return (Long) results.get(0) == 1;
        });
    }
    
    //좋아요 버틀 취소할 때, undo
    public boolean unlikePost(String userId, String postId) {
        String postLikesKey = "like:post:" + postId;
        String userLikesKey = "user:likes:" + userId;
    
        return redisTemplate.execute((RedisCallback<Boolean>) connection -> {
            connection.openPipeline();
            connection.sRem(postLikesKey.getBytes(), userId.getBytes());
            connection.zRem(userLikesKey.getBytes(), postId.getBytes());
            var results = connection.closePipeline();
            return (Long) results.get(0) == 1;
        });
    }
    
    // 좋아요 총 몇개이고 누가 눌렀는지 query
    public PostLikes getPostLikes(String postId) {
        String postLikesKey = "like:post:" + postId;
        Long likeCount = redisTemplate.opsForSet().size(postLikesKey);
        Set<String> likers = redisTemplate.opsForSet().members(postLikesKey);
        return new PostLikes(likeCount, likers);
    }
    
    // 특정 유저가 좋아요 누른 리스트 찾기
    public Set<UserLike> getUserLikes(String userId) {
        String userLikesKey = "user:likes:" + userId;
        Set<String> likedPosts = redisTemplate.opsForZSet().reverseRange(userLikesKey, 0, -1);
        return likedPosts.stream()
            .map(postId -> new UserLike(postId, redisTemplate.opsForZSet().score(userLikesKey, postId).longValue()))
            .collect(Collectors.toSet());
    }
    
    // post가 특정 유저에게 좋아요 눌렸는지 찾기
    public boolean isPostLikedByUser(String userId, String postId) {
        String postLikesKey = "like:post:" + postId;
        return Boolean.TRUE.equals(redisTemplate.opsForSet().isMember(postLikesKey, userId));
    }
    
    // 특정 게시물 여러개를 동시에 like 누른 유저 찾기
    public Set<String> getCommonLikers(Set<String> postIds) {
        String[] postLikesKeys = postIds.stream()
            .map(postId -> "like:post:" + postId)
            .toArray(String[]::new);
        return redisTemplate.opsForSet().intersect(Arrays.asList(postLikesKeys));
    }
    
    //가장 많이 좋아요 눌린 post 찾기
    public Set<String> getMostLikedPosts(long limit) {
        return redisTemplate.opsForZSet().reverseRange("posts:by_likes", 0, limit - 1);
    }
}