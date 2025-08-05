package org.example.dto;

/**
 * 5. page-undelete 스트림 이벤트
 * URL: /v2/stream/page-undelete
 * 설명: 삭제된 페이지가 복구될 때 발생합니다.
 * 참고: 스키마가 page-delete와 동일합니다. 같은 클래스를 사용할 수 있습니다.
 */
class PageUndeleteEvent extends PageDeleteEvent {
    // 스키마가 PageDeleteEvent와 동일하므로 상속받아 그대로 사용합니다.
}
