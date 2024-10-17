package org.example.cho.lock.service.java;

import javax.transaction.Transactional;
import org.example.cho.lock.domain.Stock;
import org.example.cho.lock.repository.StockRepository;
import org.springframework.stereotype.Service;

@Service
public class StockService {
    
    private final StockRepository stockRepository;
    
    public StockService(StockRepository stockRepository) {
        this.stockRepository = stockRepository;
    }
    
    
    //case1) default -> race condition에 취약하다..
    public void decrease(Long id, Long quantity) {
        Stock stock = stockRepository.findById(id).orElseThrow();
        
        stock.decrease(quantity);
        
        stockRepository.saveAndFlush(stock);
    }
    
    /**
     * solution 01) synchronized
     *
     * Q. StockService.decrease()에 synchronized 붙이면 해결되지 않을까?
     *
     *     @Transactional
     * ex. public synchronized void decrease(Long id, Long quantity) { ... }
     *
     * A. 그래도 실패함.
     * Expected :0
     * Actual   :21
     *
     * Q. 왜지? critical section 적용했잖아?
     *
     * A. @Transactional 어노테이션 때문.
     *
     * @Transactional 붙이면, 이 메서드 안 코드 그대로 복사해서,
     *
     * copied_transactional_method(){
     *      Transaction.start(); //12:00
     *
     *      stock.decrease(); //12:01
     *
     *      //문제 발생지점! 12:02~12:04 (db가 업데이트 아직 안됬는데, 다른 쓰레드가 update되기 전 값을 가져가서, race conditon 발생!)
     *
     *      Transaction.finish(); //12:05. 트랜젝션 종료 시점에 database를 업데이트 한다.
     * }
     *
     * 이런 시간 순으로, transaction 열고, decrease() 쿼리 날리고, transaction 닫는 식으로 진행됨.
     *
     * 이 떄, decrease() 수행 후, transaction을 닫는 고 시간 사이동안, 아직 sql에서 stock값이 update가 안되었음.
     * 이 떄, 다른 쓰레드가 stock.decrease(); 날리면, race condition이 발생하는 것.
     *
     *
     * ...따라서 @Transactional 빼고, synchronized만 붙이면, critical section 보장해 주기 때문에, race condition 해결 가능.
     */
    //case2) synchronized로 동시성 문제 해결!
    public synchronized void decreaseSynchronized(Long id, Long quantity) {
        Stock stock = stockRepository.findById(id).orElseThrow();
        
        stock.decrease(quantity);
        
        stockRepository.saveAndFlush(stock);
    }
    
    
    /**
     * synchronized의 문제점
     *
     * 하나의 process 안에서만 race condition을 보장함.
     *
     * 그런데 서버가 2대 이상에서 같은 db의 Stock을 조회하려고 하면,
     * 즉, 다른 process 사이에서 같은 메모리를 참조하려고 하면,
     * race condition이 발생한다.
     *
     */
    
    //case8) database: UPDATE query
    @Transactional
    public void decreaseUpdateQuery(Long id, Long quantity) {
        Stock stock = stockRepository.findById(id).orElseThrow();
        
        stockRepository.decreaseQuantity(id, quantity);
        
        stockRepository.saveAndFlush(stock);
    }
}
