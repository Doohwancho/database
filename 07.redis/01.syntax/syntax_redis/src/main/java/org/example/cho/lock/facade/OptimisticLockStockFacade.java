package org.example.cho.lock.facade;


import org.example.cho.lock.service.database.OptimisticLockStockService;
import org.springframework.stereotype.Service;

/**
 * case4) database - Optimistic Lock
 *
 * Stock.java에 @Version을 주고,
 * update Stock where id = 1, version = 1을 두 쓰레드가 동시에 시도했을 때,
 * thread A가 이미 update 먼저해서, 버전이 다르면,
 * thread B는 Exception 뜨면서, 50ms 후에 재시도 하게 함.
 *
 * 장점
 *  1. lock 안잡기 때문에, pessimistic lock보다 성능이 대체로 더 좋다. (충돌이 많이 안일어나는 경우)
 * 단점
 *  1. update 실패시 재 시도 로직을 개발자가 수동으로 이렇게 작성해줘야 함.
 *  2. 충돌이 많으면 Pessimistic Lock이 성능상 더 좋다.
 */

@Service
public class OptimisticLockStockFacade {
    
    private OptimisticLockStockService optimisticLockStockService;
    
    public OptimisticLockStockFacade(OptimisticLockStockService optimisticLockStockService) {
        this.optimisticLockStockService = optimisticLockStockService;
    }
    
    //case4) database - optimistic lock
    public void decrease(Long id, Long quantity) throws InterruptedException {
        while (true) {
            try {
                optimisticLockStockService.decrease(id, quantity);
                
                break;
            } catch (Exception e) {
                Thread.sleep(50); //where id = 1, version = 1 이 틀렸을 때, 재시도 할 수 있게, .sleep(50ms)을 걸어줌.
            }
        }
    }
}

