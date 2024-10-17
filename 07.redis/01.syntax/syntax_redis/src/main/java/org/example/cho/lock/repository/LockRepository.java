package org.example.cho.lock.repository;

import org.example.cho.lock.domain.Stock;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;

public interface LockRepository extends JpaRepository<Stock, Long> {
    //case5) database - named lock
    //mysql 기준, get_lock과 release_lock을 통해, 락을 획득/반환 할 수 있다.
    @Query(value = "select get_lock(:key, 3000)", nativeQuery = true)
    void getLock(String key); //lock 잠구고
    
    @Query(value = "select release_lock(:key)", nativeQuery = true)
    void releaseLock(String key); //lock 잠군걸 해지
}
