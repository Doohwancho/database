# 0. what 
innodb 상태 확인할 떄 씀 

# 1. 단 쳐봐 

```sql 
SHOW ENGINE INNODB STATUS\G
```

# 2. 해석 

## 2-1. transaction 
```
------------
TRANSACTIONS
------------
Trx id counter 12345
Purge done for trx's n:o < 12340 undo n:o < 0 state: running
...
---TRANSACTION 12344, ACTIVE 600 sec
...
```

현업에서는:

- 오래 실행 중인 트랜잭션(ACTIVE 시간이 긴 트랜잭션) 식별
- 데드락 발생 시 'LATEST DETECTED DEADLOCK' 섹션으로 원인 분석
- 트랜잭션이 보유한 락과 대기 중인 락 확인

## 2-2. 버퍼 풀 사용량 분석
'BUFFER POOL AND MEMORY' 섹션을 주목

```
----------------------
BUFFER POOL AND MEMORY
----------------------
Total memory allocated 137428992; in additional pool allocated 0
Dictionary memory allocated 276332
Buffer pool size   8191
Free buffers       1
...
```

현업에서는:

- 버퍼 풀 크기와 사용량 모니터링
- Free buffers가 지나치게 적으면 메모리 압박 상태로 판단
- 버퍼 풀 히트율 확인

## 2-3. I/O 활동 분석

'FILE I/O' 섹션을 확인합니다:
 
```
--------
FILE I/O
--------
I/O thread 0 state: waiting for completed aio requests
...
Pending normal aio reads: 0, aio writes: 0
Pending ibuf aio reads: 0, aio writes: 0
...
```

현업에서는:

- 보류 중인(Pending) I/O 작업 수로 I/O 병목 진단
- 디스크 I/O 문제 시 IOPS와 함께 분석

## 2-4. 로그 활동 및 체크포인트 분석
'LOG' 섹션을 확인합니다: 

```
---
LOG
---
Log sequence number 123456789
Log flushed up to   123456700
...
```

현업에서는:

- 로그 시퀀스 번호(LSN)와 플러시된 LSN 사이의 차이로 미처리 로그 양 추정
- 체크포인트 지연 문제 발견

## 2-5. 세마포어 및 락 경합 분석
'SEMAPHORES' 섹션을 확인합니다: 

```
----------
SEMAPHORES
----------
OS WAIT ARRAY INFO: reservation count 123
...
```


현업에서는:

- 높은 스핀 락 대기 시간이 있는지 확인
- 동시성 문제 진단에 활용

