package org.example.cho.lock.facade;

import org.example.cho.lock.repository.RedisLockRepository;
import org.example.cho.lock.service.java.StockService;
import org.springframework.stereotype.Component;

//case6) redis - Lettuce
@Component
public class LettuceLockStockFacade {
    
    private RedisLockRepository redisLockRepository;
    
    private StockService stockService;
    
    public LettuceLockStockFacade(RedisLockRepository redisLockRepository, StockService stockService) {
        this.redisLockRepository = redisLockRepository;
        this.stockService = stockService;
    }
    
    public void decrease(Long key, Long quantity) throws InterruptedException {
        while (!redisLockRepository.lock(key)) { //spin lock이니, 100ms 간격으로 계속 lock을 얻으려고 시도
            Thread.sleep(100);
        }
        
        try {
            stockService.decrease(key, quantity); //lock을 얻었으면, 메서드 수행 후,
        } finally {
            redisLockRepository.unlock(key); //lock을 반환
        }
    }
}
