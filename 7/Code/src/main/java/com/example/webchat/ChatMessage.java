package com.example.webchat;

import java.io.Serializable;

public record ChatMessage(String sender, String message, String senderAvatarUrl) implements Serializable {
}
