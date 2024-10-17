package org.example.cho.lock.database;

import static org.junit.jupiter.api.Assertions.assertEquals;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import org.example.cho.lock.domain.Stock;
import org.example.cho.lock.facade.NamedLockStockFacade;
import org.example.cho.lock.repository.StockRepository;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;

@SpringBootTest
public class NamedLockTest {
    //case5) database: named lock
    @Autowired
    private NamedLockStockFacade stockService;
    
    @Autowired
    private StockRepository stockRepository;
    
    
    @BeforeEach
    public void insert() {
        Stock stock = new Stock(1L, 100L);
        
        stockRepository.saveAndFlush(stock);
    }
    
    @AfterEach
    public void delete() {
        stockRepository.deleteAll();
    }
    /**
     * Q. what is CountDownLatch?
     *
     * A. A synchronization aid that allows one or more threads to wait until a set of operations being performed in other threads completes.
     */
    
    @Test
    @DisplayName("2. 요청이 동시에 100개씩 들어오는 상황")
    public void 동시에_100명이_주문() throws InterruptedException {
        int threadCount = 100;
        ExecutorService executorService = Executors.newFixedThreadPool(32); //ExecuteService는 비동기 요청을 단순화 하는 자바 API
        CountDownLatch latch = new CountDownLatch(threadCount); //CountDownLatch는 다른 쓰레드의 작업이 완료될 때 까지 기다리게 만드는 클래스
        
        for (int i = 0; i < threadCount; i++) {
            executorService.submit(() -> {
                try {
                    stockService.decrease(1L, 1L);
                } finally {
                    latch.countDown();
                }
            });
        }
        
        latch.await();
        
        Stock stock = stockRepository.findById(1L).orElseThrow();
        
        // 100 - (100 * 1) = 0
        assertEquals(0, stock.getQuantity()); //expected 0, actual: 88
    }
}
