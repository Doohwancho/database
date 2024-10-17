package org.example.cho.lock.facade;


import java.util.concurrent.TimeUnit;
import org.example.cho.lock.service.java.StockService;
import org.redisson.api.RLock;
import org.redisson.api.RedissonClient;
import org.springframework.stereotype.Component;

@Component
public class RedissonLockStockFacade {
    
    private RedissonClient redissonClient;
    
    private StockService stockService;
    
    public RedissonLockStockFacade(RedissonClient redissonClient, StockService stockService) {
        this.redissonClient = redissonClient;
        this.stockService = stockService;
    }
    
    public void decrease(Long key, Long quantity) {
        RLock lock = redissonClient.getLock(key.toString());
        
        try {
            boolean available = lock.tryLock(10, 1, TimeUnit.SECONDS); //lock 획득 최대 대기시간 10초, 안에 들어갔다가 timeout되서 나오는 시간 1초
            
            if (!available) {
                System.out.println("lock 획득 실패");
                return;
            }
            
            stockService.decrease(key, quantity);
        } catch (InterruptedException e) {
            throw new RuntimeException(e);
        } finally {
            lock.unlock();
        }
    }
}
