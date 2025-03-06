# 0. what 

OS 수준에서 MySQL이 사용하는 자원을 모니터링

# 1. 단 쳐봐 

```sql
# Linux 기준
# top 명령어로 CPU, 메모리 사용량 확인
top -p $(pidof mysqld)

# iostat으로 디스크 I/O 확인
iostat -xm 5

# vmstat으로 시스템 자원 사용량 확인
vmstat 5
```

