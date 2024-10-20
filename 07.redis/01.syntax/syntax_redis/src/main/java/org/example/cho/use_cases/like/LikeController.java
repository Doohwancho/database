package org.example.cho.use_cases.like;

import java.util.Set;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/api/likes")
public class LikeController {
    
    private final LikeService likeService;
    
    @Autowired
    public LikeController(LikeService likeService) {
        this.likeService = likeService;
    }
    
    @PostMapping("/{userId}/like/{postId}")
    public boolean likePost(@PathVariable String userId, @PathVariable String postId) {
        return likeService.likePost(userId, postId);
    }
    
    @DeleteMapping("/{userId}/unlike/{postId}")
    public boolean unlikePost(@PathVariable String userId, @PathVariable String postId) {
        return likeService.unlikePost(userId, postId);
    }
    
    @GetMapping("/post/{postId}")
    public PostLikes getPostLikes(@PathVariable String postId) {
        return likeService.getPostLikes(postId);
    }
    
    @GetMapping("/user/{userId}")
    public Set<UserLike> getUserLikes(@PathVariable String userId) {
        return likeService.getUserLikes(userId);
    }
    
    @GetMapping("/{userId}/liked/{postId}")
    public boolean isPostLikedByUser(@PathVariable String userId, @PathVariable String postId) {
        return likeService.isPostLikedByUser(userId, postId);
    }
    
    @PostMapping("/common-likers")
    public Set<String> getCommonLikers(@RequestBody Set<String> postIds) {
        return likeService.getCommonLikers(postIds);
    }
    
    @GetMapping("/most-liked")
    public Set<String> getMostLikedPosts(@RequestParam(defaultValue = "10") long limit) {
        return likeService.getMostLikedPosts(limit);
    }
}