package org.example.dto;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * 모든 이벤트에 공통적으로 포함되는 메타데이터 객체입니다.
 */
class Meta {
    @JsonProperty("uri")
    private String uri;

    @JsonProperty("request_id")
    private String requestId;

    @JsonProperty("id")
    private String id;

    @JsonProperty("dt")
    private String dt; // ISO 8601 형식의 시간 문자열

    @JsonProperty("domain")
    private String domain;

    @JsonProperty("stream")
    private String stream;

    @JsonProperty("topic")
    private String topic;

    @JsonProperty("partition")
    private int partition;

    @JsonProperty("offset")
    private long offset;

    // Getters and Setters
    public String getUri() { return uri; }
    public void setUri(String uri) { this.uri = uri; }
    public String getRequestId() { return requestId; }
    public void setRequestId(String requestId) { this.requestId = requestId; }
    public String getId() { return id; }
    public void setId(String id) { this.id = id; }
    public String getDt() { return dt; }
    public void setDt(String dt) { this.dt = dt; }
    public String getDomain() { return domain; }
    public void setDomain(String domain) { this.domain = domain; }
    public String getStream() { return stream; }
    public void setStream(String stream) { this.stream = stream; }
    public String getTopic() { return topic; }
    public void setTopic(String topic) { this.topic = topic; }
    public int getPartition() { return partition; }
    public void setPartition(int partition) { this.partition = partition; }
    public long getOffset() { return offset; }
    public void setOffset(long offset) { this.offset = offset; }
}
