package org.example.cho.lock.repository;

import javax.persistence.LockModeType;
import org.example.cho.lock.domain.Stock;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Lock;
import org.springframework.data.jpa.repository.Modifying;
import org.springframework.data.jpa.repository.Query;

public interface StockRepository extends JpaRepository<Stock, Long> {
    
    //case3) database - pessimistic lock
    @Lock(value = LockModeType.PESSIMISTIC_WRITE)
    @Query("select s from Stock s where s.id=:id")
    Stock findByIdWithPessimisticLock(Long id);
    
    //case4) database - optimistic lock
    @Lock(value = LockModeType.OPTIMISTIC)
    @Query("select s from Stock s where s.id = :id")
    Stock findByIdWithOptimisticLock(Long id);
    
    //case8) database - Update query
    @Modifying
    @Query("UPDATE Stock s SET s.quantity = s.quantity - :quantity WHERE s.id = :id")
    int decreaseQuantity(Long id, Long quantity);
}
