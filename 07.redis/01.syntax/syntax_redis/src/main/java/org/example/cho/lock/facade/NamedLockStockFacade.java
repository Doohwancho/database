package org.example.cho.lock.facade;

import javax.transaction.Transactional;
import org.example.cho.lock.repository.LockRepository;
import org.example.cho.lock.service.database.NamedLockStockService;
import org.springframework.stereotype.Component;

@Component
public class NamedLockStockFacade {
    
    private LockRepository lockRepository;
    
    private NamedLockStockService stockService;
    
    public NamedLockStockFacade(LockRepository lockRepository, NamedLockStockService stockService) {
        this.lockRepository = lockRepository;
        this.stockService = stockService;
    }
    
    //case5) named lock
    @Transactional
    public void decrease(Long id, Long quantity) {
        try {
            lockRepository.getLock(id.toString()); //get lock후,
            stockService.decrease(id, quantity); //update가 끝나면,
        } finally {
            lockRepository.releaseLock(id.toString()); //lock 반환
        }
    }
}
