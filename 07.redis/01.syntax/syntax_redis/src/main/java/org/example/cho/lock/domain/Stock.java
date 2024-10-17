package org.example.cho.lock.domain;

import javax.persistence.*;

@Entity
public class Stock {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;
    
    private Long productId;
    
    private Long quantity;
    
    //for case4) optimistic lock
    @Version
    private Long version;
    
    public Stock() {
    
    }
    
    public Stock(Long productId, Long quantity) {
        this.productId = productId;
        this.quantity = quantity;
    }
    
    public Long getQuantity() {
        return quantity;
    }
    
    public void decrease(Long quantity) {
        if (this.quantity - quantity < 0) {
            throw new RuntimeException("foo");
        }
        
        this.quantity -= quantity;
    }
}
