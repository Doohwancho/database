package org.example.dto;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * 2. revision-create 스트림 이벤트
 * URL: /v2/stream/revision-create
 * 설명: 문서의 새 리비전(수정/편집)이 생성될 때 발생합니다.
 */
class RevisionCreateEvent extends BaseEvent {
    @JsonProperty("page_id")
    private int pageId;

    @JsonProperty("page_title")
    private String pageTitle;

    @JsonProperty("page_namespace")
    private int pageNamespace;

    @JsonProperty("page_is_redirect")
    private boolean pageIsRedirect;

    @JsonProperty("rev_id")
    private long revId;

    @JsonProperty("rev_timestamp")
    private String revTimestamp; // ISO 8601

    @JsonProperty("rev_sha1")
    private String revSha1;

    @JsonProperty("rev_minor_edit")
    private boolean revMinorEdit;

    @JsonProperty("rev_len")
    private int revLen;

    @JsonProperty("rev_content_model")
    private String revContentModel;

    @JsonProperty("rev_content_format")
    private String revContentFormat;

    @JsonProperty("performer")
    private Performer performer;

    @JsonProperty("comment")
    private String comment;

    @JsonProperty("database")
    private String database;

    // Getters and Setters
    public int getPageId() { return pageId; }
    public void setPageId(int pageId) { this.pageId = pageId; }
    public String getPageTitle() { return pageTitle; }
    public void setPageTitle(String pageTitle) { this.pageTitle = pageTitle; }
    public int getPageNamespace() { return pageNamespace; }
    public void setPageNamespace(int pageNamespace) { this.pageNamespace = pageNamespace; }
    public boolean isPageIsRedirect() { return pageIsRedirect; }
    public void setPageIsRedirect(boolean pageIsRedirect) { this.pageIsRedirect = pageIsRedirect; }
    public long getRevId() { return revId; }
    public void setRevId(long revId) { this.revId = revId; }
    public String getRevTimestamp() { return revTimestamp; }
    public void setRevTimestamp(String revTimestamp) { this.revTimestamp = revTimestamp; }
    public String getRevSha1() { return revSha1; }
    public void setRevSha1(String revSha1) { this.revSha1 = revSha1; }
    public boolean isRevMinorEdit() { return revMinorEdit; }
    public void setRevMinorEdit(boolean revMinorEdit) { this.revMinorEdit = revMinorEdit; }
    public int getRevLen() { return revLen; }
    public void setRevLen(int revLen) { this.revLen = revLen; }
    public String getRevContentModel() { return revContentModel; }
    public void setRevContentModel(String revContentModel) { this.revContentModel = revContentModel; }
    public String getRevContentFormat() { return revContentFormat; }
    public void setRevContentFormat(String revContentFormat) { this.revContentFormat = revContentFormat; }
    public Performer getPerformer() { return performer; }
    public void setPerformer(Performer performer) { this.performer = performer; }
    public String getComment() { return comment; }
    public void setComment(String comment) { this.comment = comment; }
    public String getDatabase() { return database; }
    public void setDatabase(String database) { this.database = database; }
}

