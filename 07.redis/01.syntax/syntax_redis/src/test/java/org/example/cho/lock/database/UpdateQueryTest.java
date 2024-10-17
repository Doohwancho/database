package org.example.cho.lock.database;


import static org.junit.jupiter.api.Assertions.assertEquals;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import org.example.cho.lock.domain.Stock;
import org.example.cho.lock.repository.StockRepository;
import org.example.cho.lock.service.java.StockService;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;

@SpringBootTest
public class UpdateQueryTest {
    //case8) database: UPDATE query
    @Autowired
    private StockService stockService;
    
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
    
    @Test
    @DisplayName("1. 요청이 1개씩 들어오는 상황")
    public void decrease_test() {
        stockService.decrease(1L, 1L);
        
        Stock stock = stockRepository.findById(1L).orElseThrow();
        // 100 - 1 = 99
        
        assertEquals(99, stock.getQuantity());
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
                    stockService.decreaseUpdateQuery(1L, 1L);
                } finally {
                    latch.countDown();
                }
            });
        }
        
        latch.await();
        
        Stock stock = stockRepository.findById(1L).orElseThrow();
        
        // 100 - (100 * 1) = 0
        assertEquals(0, stock.getQuantity()); //expected 0, actual: 88
        /**
         * Q. 왜 0이 아니지? 88개 왜 씹힘?
         *
         * A. race condition이 일어났기 때문.
         *    race condition이란, 둘 이상의 쓰레드가 공유 데이터를 동시에 참조하고 변경하려 할 떄, 나타나는 문제.
         *
         *    1. thread A가 Stock에 접근 후,
         *    2. thread A가 값을 5->4로 변경 후, 일 끝난 다음,
         *    3. thread B가 Stock에 접근 후,
         *    4. thread B가 값을 4->3로 변경 후, 일 끝낼 것이라 예상했지만,
         *
         *    실상은,
         *    1. threadA가 Stock에 접근 후,
         *    2. threadB가 Stock에 접근 후,
         *    3. thread A가 update Stock to 4로 하고 마침
         *    4. thread B가 update Stock to 4로 하고 마침
         *
         *
         *  Q. 이 문제 어떻게 해결?
         *  A. critical section
         *
         *  java의 경우, synchronized를 서비스에 쿼리날리는 메서드에 씌워주면 됨. StockService.java 참조.
         */
    }
}

