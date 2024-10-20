package org.example.cho.use_cases.job_queue._02_redisson;
import org.springframework.data.jpa.repository.JpaRepository;

public interface JobRepository extends JpaRepository<Job, String> {
}