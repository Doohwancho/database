package org.example.dto;

import com.fasterxml.jackson.annotation.JsonProperty;

import java.util.List;
import java.util.Map;

/**
 * 7. page-properties-change 스트림 이벤트
 * URL: /v2/stream/page-properties-change
 * 설명: 페이지 속성이 변경될 때 발생합니다.
 */
public class PagePropertiesChangeEvent extends BaseEvent {
    @JsonProperty("page_id")
    private int pageId;

    @JsonProperty("page_title")
    private String pageTitle;

    @JsonProperty("page_namespace")
    private int pageNamespace;

    @JsonProperty("performer")
    private Performer performer;

    @JsonProperty("comment")
    private String comment;

    @JsonProperty("database")
    private String database;

    @JsonProperty("added_properties")
    private Map<String, String> addedProperties;

    @JsonProperty("removed_properties")
    private List<String> removedProperties;

    @JsonProperty("changed_properties")
    private Map<String, PropertyChange> changedProperties;

    public static class PropertyChange {
        @JsonProperty("old_value")
        private Object oldValue;

        @JsonProperty("new_value")
        private Object newValue;

        // Getters and Setters
        public Object getOldValue() { return oldValue; }
        public void setOldValue(Object oldValue) { this.oldValue = oldValue; }
        public Object getNewValue() { return newValue; }
        public void setNewValue(Object newValue) { this.newValue = newValue; }
    }

    // Getters and Setters
    public int getPageId() { return pageId; }
    public void setPageId(int pageId) { this.pageId = pageId; }
    public String getPageTitle() { return pageTitle; }
    public void setPageTitle(String pageTitle) { this.pageTitle = pageTitle; }
    public int getPageNamespace() { return pageNamespace; }
    public void setPageNamespace(int pageNamespace) { this.pageNamespace = pageNamespace; }
    public Performer getPerformer() { return performer; }
    public void setPerformer(Performer performer) { this.performer = performer; }
    public String getComment() { return comment; }
    public void setComment(String comment) { this.comment = comment; }
    public String getDatabase() { return database; }
    public void setDatabase(String database) { this.database = database; }
    public Map<String, String> getAddedProperties() { return addedProperties; }
    public void setAddedProperties(Map<String, String> addedProperties) { this.addedProperties = addedProperties; }
    public List<String> getRemovedProperties() { return removedProperties; }
    public void setRemovedProperties(List<String> removedProperties) { this.removedProperties = removedProperties; }
    public Map<String, PropertyChange> getChangedProperties() { return changedProperties; }
    public void setChangedProperties(Map<String, PropertyChange> changedProperties) { this.changedProperties = changedProperties; }
}
