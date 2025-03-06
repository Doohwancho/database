# 이미 준비된 대규모 database list

## 1. MySQL Employees Sample Database

정보

GitHub 링크: https://github.com/datacharmer/test_db
크기: 약 400만 행 (모든 테이블 합산)
테이블: 6개 테이블로 구성

테이블 구성 및 크기

employees: 약 300,000 행 - 직원 기본 정보
salaries: 약 2,800,000 행 - 직원별 급여 이력
titles: 약 440,000 행 - 직원별 직책 이력
dept_emp: 약 331,000 행 - 부서별 직원 배치 이력
departments: 9 행 - 부서 정보
dept_manager: 24 행 - 부서별 관리자 이력

설치 방법

### 저장소 복제
```
git clone https://github.com/datacharmer/test_db.git
cd test_db
```

### MySQL에 데이터 로드
```
mysql -u root -p < employees.sql
```

## 2. Wikipedia Sample Database (실제 대규모 데이터 사이즈)

목적

파티셔닝, 샤딩 같은 고급 기법 연습 가능
하지만 설정과 로딩에 상당한 시간과 디스크 공간 필요


정보

다운로드 링크: https://en.wikipedia.org/wiki/Wikipedia:Database_download
크기: 수십 GB (선택에 따라 다양함)
특징: 실제 운영 데이터에 가까운 대용량 데이터

설치 방법

Wikimedia 덤프 페이지에서 원하는 크기의 덤프 파일 다운로드
변환 도구를 사용하여 MySQL용 SQL 파일로 변환
MySQL에 로드

```
# 예시 (간소화됨)
wget https://dumps.wikimedia.org/enwiki/latest/enwiki-latest-pages-articles-multistream.xml.bz2
bzcat enwiki-latest-pages-articles-multistream.xml.bz2 | php mwdumper.php --format=sql > wikipedia.sql
mysql -u root -p < wikipedia.sql
```


