package org.example.cho.use_cases.job_queue._02_redisson;

import java.io.Serializable;
import javax.persistence.Entity;
import javax.persistence.Id;
import lombok.Data;

@Data
@Entity
public class Job implements Serializable {
    @Id
    private String id;
    private String type;
    private String data;
    private String status;
}