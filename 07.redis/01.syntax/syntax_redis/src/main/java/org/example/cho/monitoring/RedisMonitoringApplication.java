package org.example.cho.monitoring;

import java.util.ArrayList;
import java.util.List;
import org.springframework.boot.CommandLineRunner;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.context.annotation.Bean;

@SpringBootApplication
public class RedisMonitoringApplication {
    public static void main(String[] args) {
        SpringApplication.run(RedisMonitoringApplication.class, args);
    }
    
    @Bean
    public CommandLineRunner initializeData(UserService userService) {
        return args -> {
            System.out.println("Initializing data...");
            List<User> users = new ArrayList<>();
            for (int i = 0; i <= 10000; i++) {
                User user = new User(Long.valueOf(i), "User " + i);
                users.add(user);
            }
            userService.bulkCreateUsers(users);
            System.out.println("Data initialization complete.");
        };
    }
}
