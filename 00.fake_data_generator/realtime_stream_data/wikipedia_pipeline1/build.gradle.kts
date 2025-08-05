plugins {
    java
    id("org.springframework.boot") version "2.7.18" // 스프링 부트 2.x 버전으로 변경
    id("io.spring.dependency-management") version "1.0.15.RELEASE" // Spring Boot 2.7.x와 호환되는 버전
}

group = "com.example"
version = "0.0.1-SNAPSHOT"

java {
    sourceCompatibility = JavaVersion.VERSION_1_8 // JDK 1.8 (Java 8)로 변경
}

repositories {
    mavenCentral()
}

dependencies {
    // 기본적인 Spring MVC 웹 애플리케이션을 위한 의존성
    implementation("org.springframework.boot:spring-boot-starter-web")
    implementation("com.fasterxml.jackson.core:jackson-databind:2.13.3")
}

tasks.withType<Test> {
    useJUnitPlatform()
}
