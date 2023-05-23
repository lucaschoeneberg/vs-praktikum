package http;

public class ChatMessage {
    private final String sender;
    private final String message;
    private final String senderAvatarUrl;

    public ChatMessage(String sender, String message, String senderAvatarUrl) {
        this.sender = sender;
        this.message = message;
        this.senderAvatarUrl = senderAvatarUrl;
    }

    public String getSender() {
        return sender;
    }

    public String getMessage() {
        return message;
    }

    public String getSenderAvatarUrl() {
        return senderAvatarUrl;
    }
}
