package org.example.cho.lock.service.database;

import javax.transaction.Transactional;
import org.example.cho.lock.domain.Stock;
import org.example.cho.lock.repository.StockRepository;
import org.springframework.stereotype.Service;

@Service
public class PessimisticLockStockService {
    
    private StockRepository stockRepository;
    
    public PessimisticLockStockService(StockRepository stockRepository) {
        this.stockRepository = stockRepository;
    }
    
    /**
     * pessimistic lock
     *
     * db에 데이터에 critical section 처리하는 것.
     *
     * 장점
     *  1. 충돌이 빈번하다면, optimistic lock보다 성능이 좋을 수 있다.
     *  2. lock을 통해 확실히 critical section 정해주기 때문에, 데이터 정합성이 보장된다.
     *
     * 단점
     *  1. lock 쓰는 것 자체가 성능 하락이 심함.
     */
    
    //case3) database - pessimistic lock
    @Transactional
    public Long decrease(Long id, Long quantity) {
        Stock stock = stockRepository.findByIdWithPessimisticLock(id);
        stock.decrease(quantity);
        stockRepository.saveAndFlush(stock);
        
        return stock.getQuantity();
    }
}
