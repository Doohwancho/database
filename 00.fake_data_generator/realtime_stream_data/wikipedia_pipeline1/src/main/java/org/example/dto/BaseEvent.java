package org.example.dto;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * 모든 이벤트 클래스의 기반이 되는 기본 클래스입니다.
 * 공통 필드인 $schema와 meta를 포함합니다.
 */
abstract class BaseEvent {
    @JsonProperty("$schema")
    private String schema;

    @JsonProperty("meta")
    private Meta meta;

    // Getters and Setters
    public String getSchema() { return schema; }
    public void setSchema(String schema) { this.schema = schema; }
    public Meta getMeta() { return meta; }
    public void setMeta(Meta meta) { this.meta = meta; }
}