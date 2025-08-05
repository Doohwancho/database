# what 
위키미디어에 stream 트래픽을 10개 쓰레드가 병렬로 받는 코드.\
[병렬요청은 최대 10개까지](https://enterprise.wikimedia.com/blog/realtime-api-parallel-connections-restart-support/)

트래픽이 많을 줄 알았는데 20~700 RPS으로 요동치고, 보통 한 200RPS정도 된다. 

이런식이면 부하거는 서버 따로 띄워서 하는게 좋을 지 싶다. 

## 실행 로그 
```
.   ____          _            __ _ _
/\\ / ___'_ __ _ _(_)_ __  __ _ \ \ \ \
( ( )\___ | '_ | '_| | '_ \/ _` | \ \ \ \
\\/  ___)| |_)| | | | | || (_| |  ) ) ) )
'  |____| .__|_| |_|_| |_\__, | / / / /
=========|_|==============|___/=/_/_/_/
:: Spring Boot ::               (v2.7.18)

2025-08-05 12:27:59.472  INFO 9396 --- [           main] org.example.Parallel                     : Starting Parallel using Java 1.8.0_181 on DESKTOP-BOVSO20 with PID 9396 (C:\Users\happy\Desktop\data_pipeline\wikipedia_pipeline1\build\classes\java\main started by happy in C:\Users\happy\Desktop\data_pipeline)
2025-08-05 12:27:59.476  INFO 9396 --- [           main] org.example.Parallel                     : No active profile set, falling back to 1 default profile: "default"
2025-08-05 12:28:00.288  INFO 9396 --- [           main] o.s.b.w.embedded.tomcat.TomcatWebServer  : Tomcat initialized with port(s): 8080 (http)
2025-08-05 12:28:00.294  INFO 9396 --- [           main] o.apache.catalina.core.StandardService   : Starting service [Tomcat]
2025-08-05 12:28:00.295  INFO 9396 --- [           main] org.apache.catalina.core.StandardEngine  : Starting Servlet engine: [Apache Tomcat/9.0.83]
2025-08-05 12:28:00.401  INFO 9396 --- [           main] o.a.c.c.C.[Tomcat].[localhost].[/]       : Initializing Spring embedded WebApplicationContext
2025-08-05 12:28:00.402  INFO 9396 --- [           main] w.s.c.ServletWebServerApplicationContext : Root WebApplicationContext: initialization completed in 877 ms
2025-08-05 12:28:00.693  INFO 9396 --- [           main] o.s.b.w.embedded.tomcat.TomcatWebServer  : Tomcat started on port(s): 8080 (http) with context path ''
2025-08-05 12:28:00.701  INFO 9396 --- [           main] org.example.Parallel                     : Started Parallel in 1.712 seconds (JVM running for 3.127)
2025-08-05 12:28:00.704  INFO 9396 --- [           main] org.example.StreamConsumer               : 애플리케이션 시작. 10개의 병렬 연결을 생성합니다...
2025-08-05 12:28:01.717  INFO 9396 --- [pool-1-thread-1] org.example.StreamConsumer               : --- Total TPS: 0 req/s ---
2025-08-05 12:28:02.121  INFO 9396 --- [  stream-part-8] org.example.StreamConsumer               : [Part 8] 스트림에 성공적으로 연결되었습니다.
2025-08-05 12:28:02.152  INFO 9396 --- [  stream-part-2] org.example.StreamConsumer               : [Part 2] 스트림에 성공적으로 연결되었습니다.
2025-08-05 12:28:02.152  INFO 9396 --- [  stream-part-3] org.example.StreamConsumer               : [Part 3] 스트림에 성공적으로 연결되었습니다.
2025-08-05 12:28:02.152  INFO 9396 --- [  stream-part-1] org.example.StreamConsumer               : [Part 1] 스트림에 성공적으로 연결되었습니다.
2025-08-05 12:28:02.152  INFO 9396 --- [  stream-part-7] org.example.StreamConsumer               : [Part 7] 스트림에 성공적으로 연결되었습니다.
2025-08-05 12:28:02.156  INFO 9396 --- [  stream-part-5] org.example.StreamConsumer               : [Part 5] 스트림에 성공적으로 연결되었습니다.
2025-08-05 12:28:02.156  INFO 9396 --- [  stream-part-0] org.example.StreamConsumer               : [Part 0] 스트림에 성공적으로 연결되었습니다.
2025-08-05 12:28:02.157  INFO 9396 --- [  stream-part-4] org.example.StreamConsumer               : [Part 4] 스트림에 성공적으로 연결되었습니다.
2025-08-05 12:28:02.157  INFO 9396 --- [  stream-part-6] org.example.StreamConsumer               : [Part 6] 스트림에 성공적으로 연결되었습니다.
2025-08-05 12:28:02.157  INFO 9396 --- [  stream-part-9] org.example.StreamConsumer               : [Part 9] 스트림에 성공적으로 연결되었습니다.
2025-08-05 12:28:02.712  INFO 9396 --- [pool-1-thread-1] org.example.StreamConsumer               : --- Total TPS: 42 req/s ---
2025-08-05 12:28:03.716  INFO 9396 --- [pool-1-thread-1] org.example.StreamConsumer               : --- Total TPS: 24 req/s ---
2025-08-05 12:28:04.716  INFO 9396 --- [pool-1-thread-1] org.example.StreamConsumer               : --- Total TPS: 995 req/s ---
2025-08-05 12:28:05.708  INFO 9396 --- [pool-1-thread-1] org.example.StreamConsumer               : --- Total TPS: 141 req/s ---
2025-08-05 12:28:06.717  INFO 9396 --- [pool-1-thread-1] org.example.StreamConsumer               : --- Total TPS: 197 req/s ---
2025-08-05 12:28:07.706  INFO 9396 --- [pool-1-thread-1] org.example.StreamConsumer               : --- Total TPS: 133 req/s ---
2025-08-05 12:28:08.707  INFO 9396 --- [pool-1-thread-1] org.example.StreamConsumer               : --- Total TPS: 20 req/s ---
2025-08-05 12:28:09.713  INFO 9396 --- [pool-1-thread-1] org.example.StreamConsumer               : --- Total TPS: 390 req/s ---
2025-08-05 12:28:10.714  INFO 9396 --- [pool-1-thread-1] org.example.StreamConsumer               : --- Total TPS: 588 req/s ---
2025-08-05 12:28:11.719  INFO 9396 --- [pool-1-thread-1] org.example.StreamConsumer               : --- Total TPS: 223 req/s ---
2025-08-05 12:28:12.718  INFO 9396 --- [pool-1-thread-1] org.example.StreamConsumer               : --- Total TPS: 279 req/s ---
2025-08-05 12:28:13.719  INFO 9396 --- [pool-1-thread-1] org.example.StreamConsumer               : --- Total TPS: 60 req/s ---
2025-08-05 12:28:14.716  INFO 9396 --- [pool-1-thread-1] org.example.StreamConsumer               : --- Total TPS: 200 req/s ---
2025-08-05 12:28:15.720  INFO 9396 --- [pool-1-thread-1] org.example.StreamConsumer               : --- Total TPS: 449 req/s ---
2025-08-05 12:28:16.706  INFO 9396 --- [pool-1-thread-1] org.example.StreamConsumer               : --- Total TPS: 321 req/s ---
2025-08-05 12:28:17.718  INFO 9396 --- [pool-1-thread-1] org.example.StreamConsumer               : --- Total TPS: 319 req/s ---
```