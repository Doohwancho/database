# Q. `redis-cli info` 모니터링 메트릭 어떻게 해석해야 함?
A.
1. `used_memory`: 출력에서 "Memory" 섹션에 있습니다.
	1. `total_system_memory_human:16.00G`
	2. `used_memory:1315552 used_memory_human:1.25M`
	3. Redis가 사용 중인 메모리는 1.25MB입니다.
2. `connected_clients`: "Clients" 섹션에 있습니다.
	1. `connected_clients:1`
	2. 현재 1개의 클라이언트가 연결되어 있습니다.
3. `total_commands_processed`: "Stats" 섹션에 있습니다.
	1. `total_commands_processed:12235`
	2. 지금까지 총 12,235개의 명령이 처리되었습니다.
4. `keyspace_hits` 와 `keyspace_misses`: 역시 "Stats" 섹션에 있습니다.
	1. `keyspace_hits:12228`
	2. `keyspace_misses:2`
	3. 캐시 히트가 12,228번, 미스가 2번 있었습니다. 캐시 히트율 = hits / (hits + misses) = 12228 / (12228 + 2) ≈ 99.98%
5. 네트워크 대역폭 사용량: "Stats" 섹션의 다음 항목들을 볼 수 있습니다
	1. `total_net_input_bytes:918791`
	2. `total_net_output_bytes:1088386`
	3. `instantaneous_input_kbps:0.00`
	4. `instantaneous_output_kbps:0.00`
	5. 총 입력 바이트와 출력 바이트, 그리고 현재의 초당 입출력 킬로바이트를 보여줍니다.
