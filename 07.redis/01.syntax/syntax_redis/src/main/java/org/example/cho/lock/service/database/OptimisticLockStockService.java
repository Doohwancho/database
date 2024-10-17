package org.example.cho.lock.service.database;

import javax.transaction.Transactional;
import org.example.cho.lock.domain.Stock;
import org.example.cho.lock.repository.StockRepository;
import org.springframework.stereotype.Service;

@Service
public class OptimisticLockStockService {
    
    private StockRepository stockRepository;
    
    public OptimisticLockStockService(StockRepository stockRepository) {
        this.stockRepository = stockRepository;
    }
    
    //case4) database - optimistic lock
    @Transactional
    public void decrease(Long id, Long quantity) {
        Stock stock = stockRepository.findByIdWithOptimisticLock(id);
        stock.decrease(quantity);
        
        stockRepository.saveAndFlush(stock);
    }
}
