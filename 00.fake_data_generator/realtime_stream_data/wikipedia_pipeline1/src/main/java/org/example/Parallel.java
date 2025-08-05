package org.example;

import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.DeserializationFeature;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import org.example.dto.*;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.boot.CommandLineRunner;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.stereotype.Component;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicLong;

@SpringBootApplication
public class Parallel{

    public static void main(String[] args) {
        SpringApplication.run(Parallel.class, args);
    }
}

@Component
class StreamConsumer implements CommandLineRunner {

    private static final Logger logger = LoggerFactory.getLogger(StreamConsumer.class);
    private final ObjectMapper objectMapper;

    // 구독할 스트림 목록
    private static final String BASE_STREAM_URL = "https://stream.wikimedia.org/v2/stream/recentchange,page-create,page-delete,page-move";

    // 병렬 연결 수 (최대 10개)
    private static final int PARALLEL_CONNECTIONS = 10;

    // 초당 요청 수를 세기 위한 스레드 안전한 카운터
    private final AtomicLong requestCounter = new AtomicLong(0);

    public StreamConsumer() {
        this.objectMapper = new ObjectMapper();
        this.objectMapper.configure(DeserializationFeature.FAIL_ON_UNKNOWN_PROPERTIES, false);
    }

    @Override
    public void run(String... args) throws Exception {
        logger.info("애플리케이션 시작. {}개의 병렬 연결을 생성합니다...", PARALLEL_CONNECTIONS);

        // 1초마다 전체 TPS를 출력하는 스케줄러 시작
        startTpsCounter();

        // 병렬 연결 수만큼 스레드를 가진 스레드 풀 생성
        ExecutorService executorService = Executors.newFixedThreadPool(PARALLEL_CONNECTIONS);

        // 각 스레드가 다른 'part'를 담당하도록 작업을 제출
        for (int i = 0; i < PARALLEL_CONNECTIONS; i++) {
            final int partNumber = i;
            executorService.submit(() -> connectAndProcessStream(partNumber));
        }

        // 애플리케이션이 바로 종료되지 않도록 대기
        Thread.currentThread().join();
    }

    private void startTpsCounter() {
        ScheduledExecutorService scheduler = Executors.newSingleThreadScheduledExecutor();
        scheduler.scheduleAtFixedRate(() -> {
            long count = requestCounter.getAndSet(0);
            logger.info("--- Total TPS: {} req/s ---", count);
        }, 1, 1, TimeUnit.SECONDS);
    }

    /**
     * 지정된 part에 연결하여 스트림을 처리합니다.
     * @param partNumber 연결할 스트림의 part (0-9)
     */
    private void connectAndProcessStream(int partNumber) {
        Thread.currentThread().setName("stream-part-" + partNumber);

        while (true) {
            HttpURLConnection connection = null;
            BufferedReader reader = null;
            try {
                // 각 연결에 맞는 part 파라미터를 추가하여 URL 생성
                String urlWithParams = BASE_STREAM_URL + "?parts=[" + partNumber + "]";
                URL url = new URL(urlWithParams);

                connection = (HttpURLConnection) url.openConnection();
                connection.setRequestMethod("GET");
                connection.setConnectTimeout(10000);
                connection.setReadTimeout(0);
                connection.connect();

                int responseCode = connection.getResponseCode();
                if (responseCode == HttpURLConnection.HTTP_OK) {
                    logger.info("[Part {}] 스트림에 성공적으로 연결되었습니다.", partNumber);
                    reader = new BufferedReader(new InputStreamReader(connection.getInputStream(), StandardCharsets.UTF_8));
                    String line;
                    while ((line = reader.readLine()) != null) {
                        if (line.startsWith("data: ")) {
                            String jsonData = line.substring(6);
                            processJsonData(jsonData);
                        }
                    }
                } else {
                    logger.error("[Part {}] 스트림 연결 실패. 응답 코드: {}", partNumber, responseCode);
                }

            } catch (Exception e) {
                logger.error("[Part {}] 스트림 처리 중 오류 발생: {}", partNumber, e.getMessage());
            } finally {
                if (reader != null) try { reader.close(); } catch (Exception e) { /* 무시 */ }
                if (connection != null) connection.disconnect();
                logger.info("[Part {}] 스트림 연결이 끊겼습니다. 5초 후 재연결을 시도합니다.", partNumber);
                try {
                    Thread.sleep(5000);
                } catch (InterruptedException ie) {
                    Thread.currentThread().interrupt();
                    logger.warn("[Part {}] 재연결 대기 중 인터럽트 발생. 스트림 처리를 종료합니다.", partNumber);
                    break;
                }
            }
        }
    }

    private void processJsonData(String jsonData) {
        try {
            JsonNode rootNode = objectMapper.readTree(jsonData);
            String topic = rootNode.path("meta").path("topic").asText();

            if (topic.endsWith("mediawiki.recentchange")) {
                objectMapper.treeToValue(rootNode, RecentChangeEvent.class);
            } else if (topic.endsWith("mediawiki.page-create")) {
                objectMapper.treeToValue(rootNode, PageCreateEvent.class);
            } else if (topic.endsWith("mediawiki.page-delete")) {
                objectMapper.treeToValue(rootNode, PageDeleteEvent.class);
            } else if (topic.endsWith("mediawiki.page-move")) {
                objectMapper.treeToValue(rootNode, PageMoveEvent.class);
            } else if (topic.endsWith("mediawiki.page-properties-change")) {
                objectMapper.treeToValue(rootNode, PagePropertiesChangeEvent.class);
            } else {
                logger.warn("알 수 없는 스트림 타입: {}", topic);
                return;
            }

            requestCounter.incrementAndGet();

        } catch (JsonProcessingException e) {
            logger.error("JSON 파싱 실패: {}", jsonData, e);
        }
    }
}
