package org.example.dto;

/**
 * 3. page-create 스트림 이벤트
 * URL: /v2/stream/page-create
 * 설명: 새 페이지가 생성될 때 발생합니다.
 * 참고: 스키마가 revision-create와 동일합니다. 같은 클래스를 사용할 수 있습니다.
 */
public class PageCreateEvent extends RevisionCreateEvent {
    // 스키마가 RevisionCreateEvent와 동일하므로 상속받아 그대로 사용합니다.
    // 추가적인 필드나 로직이 필요하다면 여기에 작성할 수 있습니다.
}
