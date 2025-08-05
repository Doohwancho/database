package org.example.dto;

import com.fasterxml.jackson.annotation.JsonProperty;

import java.util.List;

/**
 * 이벤트를 발생시킨 사용자의 정보를 담는 객체입니다.
 */
public class Performer {
    @JsonProperty("user_text")
    private String userText;

    @JsonProperty("user_groups")
    private List<String> userGroups;

    @JsonProperty("user_is_bot")
    private boolean userIsBot;

    @JsonProperty("user_id")
    private int userId;

    @JsonProperty("user_registration_dt")
    private String userRegistrationDt;

    @JsonProperty("user_edit_count")
    private int userEditCount;

    // Getters and Setters
    public String getUserText() { return userText; }
    public void setUserText(String userText) { this.userText = userText; }
    public List<String> getUserGroups() { return userGroups; }
    public void setUserGroups(List<String> userGroups) { this.userGroups = userGroups; }
    public boolean isUserIsBot() { return userIsBot; }
    public void setUserIsBot(boolean userIsBot) { this.userIsBot = userIsBot; }
    public int getUserId() { return userId; }
    public void setUserId(int userId) { this.userId = userId; }
    public String getUserRegistrationDt() { return userRegistrationDt; }
    public void setUserRegistrationDt(String userRegistrationDt) { this.userRegistrationDt = userRegistrationDt; }
    public int getUserEditCount() { return userEditCount; }
    public void setUserEditCount(int userEditCount) { this.userEditCount = userEditCount; }
}