# Q. slow query 어케 찾음?


```sql 
-- 1. 느린 쿼리 찾기
SELECT id, user, host, db, time, state, info 
FROM information_schema.PROCESSLIST 
WHERE command != 'Sleep' 
AND time > 10
ORDER BY time DESC;

-- 2. 해당 쿼리가 어떤 테이블 락 걸고 있는지 확인
SELECT r.trx_id waiting_trx_id, r.trx_mysql_thread_id waiting_thread,
       b.trx_id blocking_trx_id, b.trx_mysql_thread_id blocking_thread,
       b.trx_query blocking_query,
       TIMESTAMPDIFF(SECOND, r.trx_wait_started, CURRENT_TIMESTAMP) wait_time,
       TIMESTAMPDIFF(SECOND, b.trx_started, CURRENT_TIMESTAMP) blocking_time
FROM information_schema.innodb_lock_waits w
JOIN information_schema.innodb_trx b ON b.trx_id = w.blocking_trx_id
JOIN information_schema.innodb_trx r ON r.trx_id = w.requesting_trx_id;

-- 3. 문제의 프로세스 종료
-- KILL [blocking_thread]; -- 실제로는 주의해서 사용
```


# 현업 꿀팁 

## 1. DB 모니터링 스크립트에 넣어두기 
```sql
-- 1시간 이상 실행 중인 트랜잭션 발견하면 Slack/이메일로 알림 보내는 스크립트
SELECT * FROM information_schema.INNODB_TRX
WHERE TIMESTAMPDIFF(MINUTE, trx_started, NOW()) > 60;
```

## 2. 테이블 크기 추이 모니터링 (갑자기 커지면 문제있는거임)
```sql
-- 매일 테이블 크기 기록해서 증가율 체크
-- 갑자기 크기 폭증하면 문제 있는 거임
SELECT table_schema, table_name, table_rows, data_length, index_length
FROM information_schema.TABLES 
WHERE table_schema NOT IN ('mysql', 'performance_schema', 'information_schema');
```

## 3. 자동 정리 쿼리 
```sql
-- 장시간 실행 중인 쿼리 자동 종료 (위험함 ㅋㅋㅋ)
SELECT CONCAT('KILL ', id, ';') 
FROM information_schema.PROCESSLIST 
WHERE time > 3600 AND command != 'Sleep';
```
