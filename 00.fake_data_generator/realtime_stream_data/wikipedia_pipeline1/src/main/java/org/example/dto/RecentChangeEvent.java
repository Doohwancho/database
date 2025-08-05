package org.example.dto;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * 1. recentchange 스트림 이벤트
 * URL: /v2/stream/recentchange
 * 설명: 모든 위키미디어 프로젝트의 최근 변경 사항을 나타냅니다. (편집, 새 글, 로그 등)
 */
public class RecentChangeEvent extends BaseEvent {
    @JsonProperty("id")
    private Long id;

    @JsonProperty("type")
    private String type;

    @JsonProperty("namespace")
    private int namespace;

    @JsonProperty("title")
    private String title;

    @JsonProperty("comment")
    private String comment;

    @JsonProperty("timestamp")
    private long timestamp; // Unix 타임스탬프

    @JsonProperty("user")
    private String user;

    @JsonProperty("bot")
    private boolean bot;

    @JsonProperty("minor")
    private boolean minor;

    @JsonProperty("patrolled")
    private Boolean patrolled; // Optional

    @JsonProperty("length")
    private ChangeSize length;

    @JsonProperty("revision")
    private ChangeRevision revision;

    @JsonProperty("server_url")
    private String serverUrl;

    @JsonProperty("server_name")
    private String serverName;

    @JsonProperty("server_script_path")
    private String serverScriptPath;

    @JsonProperty("wiki")
    private String wiki;

    @JsonProperty("parsedcomment")
    private String parsedcomment;

    // Nested classes for length and revision
    public static class ChangeSize {
        @JsonProperty("old")
        private Long old;
        @JsonProperty("new")
        private Long newSize;
        // Getters and Setters
        public Long getOld() { return old; }
        public void setOld(Long old) { this.old = old; }
        public Long getNewSize() { return newSize; }
        public void setNewSize(Long newSize) { this.newSize = newSize; }
    }

    public static class ChangeRevision {
        @JsonProperty("old")
        private Long old;
        @JsonProperty("new")
        private Long newRevision;
        // Getters and Setters
        public Long getOld() { return old; }
        public void setOld(Long old) { this.old = old; }
        public Long getNewRevision() { return newRevision; }
        public void setNewRevision(Long newRevision) { this.newRevision = newRevision; }
    }

    // Getters and Setters for RecentChangeEvent
    public Long getId() { return id; }
    public void setId(Long id) { this.id = id; }
    public String getType() { return type; }
    public void setType(String type) { this.type = type; }
    public int getNamespace() { return namespace; }
    public void setNamespace(int namespace) { this.namespace = namespace; }
    public String getTitle() { return title; }
    public void setTitle(String title) { this.title = title; }
    public String getComment() { return comment; }
    public void setComment(String comment) { this.comment = comment; }
    public long getTimestamp() { return timestamp; }
    public void setTimestamp(long timestamp) { this.timestamp = timestamp; }
    public String getUser() { return user; }
    public void setUser(String user) { this.user = user; }
    public boolean isBot() { return bot; }
    public void setBot(boolean bot) { this.bot = bot; }
    public boolean isMinor() { return minor; }
    public void setMinor(boolean minor) { this.minor = minor; }
    public Boolean getPatrolled() { return patrolled; }
    public void setPatrolled(Boolean patrolled) { this.patrolled = patrolled; }
    public ChangeSize getLength() { return length; }
    public void setLength(ChangeSize length) { this.length = length; }
    public ChangeRevision getRevision() { return revision; }
    public void setRevision(ChangeRevision revision) { this.revision = revision; }
    public String getServerUrl() { return serverUrl; }
    public void setServerUrl(String serverUrl) { this.serverUrl = serverUrl; }
    public String getServerName() { return serverName; }
    public void setServerName(String serverName) { this.serverName = serverName; }
    public String getServerScriptPath() { return serverScriptPath; }
    public void setServerScriptPath(String serverScriptPath) { this.serverScriptPath = serverScriptPath; }
    public String getWiki() { return wiki; }
    public void setWiki(String wiki) { this.wiki = wiki; }
    public String getParsedcomment() { return parsedcomment; }
    public void setParsedcomment(String parsedcomment) { this.parsedcomment = parsedcomment; }
}
