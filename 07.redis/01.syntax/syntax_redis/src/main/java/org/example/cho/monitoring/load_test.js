import http from 'k6/http';
import { check, sleep } from 'k6';
import { randomIntBetween } from 'https://jslib.k6.io/k6-utils/1.2.0/index.js';
import { SharedArray } from 'k6/data';

/*
  Q. how to run?
  A. k6 run load_test.js
     docker run --rm -i --net=host grafana/k6 run - <./load_test.js

  Q. how to monitor redis?
  A. 1. run RedisMonitoringApplication.java
     2. GET http://localhost:8080/api/users/bulkinsert/10000
     3. docker run --rm -i --net=host grafana/k6 run - <./load_test.js
     4. `bpytop` (same for htop) on console to monitor if local computer is doing okay
     5. `redis-cli monitor` on console to monitor what redis-sql-command is being executed
     6. `redis-cli info` on console to monitor current redis-stat
     7. change bulkinsert_amount and experiment with it

  Q. what is this code?
  A. read:write 7:3 인, User(id, name) 객체를 redis로부터 GET/POST 하는 load test script
     write할 떄, 여러 쓰레드가 각자 thread-unique한 아이디를 만들어 write하는데, read로 이를 읽진 않는다.
     read는 그냥 기존에 bulkinsert 한 만큼만 읽는다.
*/

const bulkinsert_amount = 10000;

//ver1 - k6가 local machine에 설치되있는 경우
//const BASE_URL = 'http://localhost:8080/api/users';
//ver2 - k6가 docker에 설치되있는 경우
const BASE_URL = 'http://host.docker.internal:8080/api/users';

export const options = {
  stages: [
    { duration: '3m', target: 100 },
    { duration: '4m', target: 200 },
    { duration: '5m', target: 300 },
    { duration: '1m', target: 0 },
  ],
  thresholds: {
    http_req_duration: ['p(95)<500'], // 95% of requests should be below 500ms
  },
};

// VU별 로컬 카운터
let localCounter = 0;

export default function () {
  const createUser = () => {
    // VU ID와 로컬 카운터를 사용하여 고유한 ID 생성
    const id = (__VU * 1000000) + (++localCounter);

    const payload = JSON.stringify({
      id: id,
      name: `New User ${id}`,
    });

    const params = {
      headers: {
        'Content-Type': 'application/json',
      },
    };

    const res = http.post(BASE_URL, payload, params);
    check(res, {
      'status is 200': (r) => r.status === 200,
      'operation successful': (r) => r.body.includes(`"id":${id}`),
    });
  };

  const getUser = () => {
    // 1부터 bulkinsert_amount 사이의 랜덤 ID 선택
    const userId = randomIntBetween(1, bulkinsert_amount);
    const res = http.get(`${BASE_URL}/${userId}`);
    check(res, {
      'status is 200': (r) => r.status === 200,
      'operation successful': (r) => r.body.includes('"id":'),
    });
  };

  // 70% 확률로 getUser, 30% 확률로 createUser 실행
  if (Math.random() < 0.7) {
    getUser();
  } else {
    createUser();
  }

  sleep(1);
}