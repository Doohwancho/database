package org.example.cho.look_aside_write_through_strategy;

import java.util.concurrent.TimeUnit;
import javax.transaction.Transactional;
import lombok.RequiredArgsConstructor;
import org.springframework.data.redis.core.RedisTemplate;
import org.springframework.stereotype.Service;

/*
    Q. what is this code?
    A. 젤 많이 쓰이는 cache 전략 조합(look aside + write around)
    
    Q. pros
    A. 1. 자주 읽히는 애들 위주로 캐싱되서 효율적이다
       2. 캐시 서버가 다운되어도 db에서 가져올 수 있다.
       
    Q. cons
    A. 1. 데이터 불일치? -> update 부분에서 cache도 같이 update하는 코드가 있으니 괜찮다.
       2. 초기 성능 저하 -> 캐시가 비어있는 초기에 많은 캐시 미스가 일어날 수 있다.
    
    Q. 주로 언제 쓰는게 유리?
    A. 1. read:write 비율에서 read가 훨씬 높은 경우
       2. update가 자주 업데이트 되지 않는 경우
       
    Q. 고려할 수 있는 potential 문제점?
    A. 분산 시스템 환경에서 cache stampede problem 일어날 수 있다.
 */

@Service
@RequiredArgsConstructor
public class ProductService {
    private final ProductRepository productRepository;
    private final RedisTemplate<String, Product> redisTemplate;
    
    private static final String CACHE_KEY_PREFIX = "product:";
    private static final long CACHE_TTL_SECONDS = 3600; // 1시간
    
    public Product getProduct(Long id) {
        String cacheKey = CACHE_KEY_PREFIX + id;
        
        // Look Aside: 캐시 확인
        Product cachedProduct = redisTemplate.opsForValue().get(cacheKey);
        if (cachedProduct != null) {
            return cachedProduct;
        }
        
        // Cache miss: DB에서 조회
        Product product = productRepository.findById(id)
            .orElseThrow(() -> new RuntimeException("Product not found"));
        
        // 캐시에 저장
        redisTemplate.opsForValue().set(cacheKey, product, CACHE_TTL_SECONDS, TimeUnit.SECONDS);
        
        return product;
    }
    
    @Transactional
    public Product createProduct(Product product) {
        // Write Around: DB에만 쓰기
        Product savedProduct = productRepository.save(product);
        
        // 캐시는 업데이트하지 않음
        return savedProduct;
    }
    
    @Transactional
    public Product updateProduct(Long id, Product updatedProduct) {
        Product product = productRepository.findById(id)
            .orElseThrow(() -> new RuntimeException("Product not found"));
        
        product.setName(updatedProduct.getName());
        product.setPrice(updatedProduct.getPrice());
        
        // Write Around: DB에만 쓰기
        Product savedProduct = productRepository.save(product);
        
        // 캐시 삭제 (선택적)
        String cacheKey = CACHE_KEY_PREFIX + id;
        redisTemplate.delete(cacheKey);
        
        return savedProduct;
    }
}