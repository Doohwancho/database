package org.example.dto;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * 6. page-move 스트림 이벤트
 * URL: /v2/stream/page-move
 * 설명: 페이지 이름이 변경(이동)될 때 발생합니다.
 */
public class PageMoveEvent extends BaseEvent {
    @JsonProperty("page_id")
    private int pageId;

    @JsonProperty("page_title")
    private String pageTitle;

    @JsonProperty("page_namespace")
    private int pageNamespace;

    @JsonProperty("prior_state")
    private PriorState priorState;

    @JsonProperty("performer")
    private Performer performer;

    @JsonProperty("comment")
    private String comment;

    @JsonProperty("database")
    private String database;

    public static class PriorState {
        @JsonProperty("page_title")
        private String pageTitle;

        @JsonProperty("page_namespace")
        private int pageNamespace;

        // Getters and Setters
        public String getPageTitle() { return pageTitle; }
        public void setPageTitle(String pageTitle) { this.pageTitle = pageTitle; }
        public int getPageNamespace() { return pageNamespace; }
        public void setPageNamespace(int pageNamespace) { this.pageNamespace = pageNamespace; }
    }

    // Getters and Setters
    public int getPageId() { return pageId; }
    public void setPageId(int pageId) { this.pageId = pageId; }
    public String getPageTitle() { return pageTitle; }
    public void setPageTitle(String pageTitle) { this.pageTitle = pageTitle; }
    public int getPageNamespace() { return pageNamespace; }
    public void setPageNamespace(int pageNamespace) { this.pageNamespace = pageNamespace; }
    public PriorState getPriorState() { return priorState; }
    public void setPriorState(PriorState priorState) { this.priorState = priorState; }
    public Performer getPerformer() { return performer; }
    public void setPerformer(Performer performer) { this.performer = performer; }
    public String getComment() { return comment; }
    public void setComment(String comment) { this.comment = comment; }
    public String getDatabase() { return database; }
    public void setDatabase(String database) { this.database = database; }
}