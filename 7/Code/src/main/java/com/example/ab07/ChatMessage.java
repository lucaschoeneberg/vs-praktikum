package com.example.ab07;

import java.io.Serializable;

public record ChatMessage(String sender, String message, String senderAvatarUrl) implements Serializable {
}
