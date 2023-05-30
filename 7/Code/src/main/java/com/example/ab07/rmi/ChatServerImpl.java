package com.example.ab07.rmi;

import com.example.ab07.ChatMessage;

import java.lang.reflect.Array;
import java.rmi.RemoteException;
import java.rmi.server.UnicastRemoteObject;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;

public class ChatServerImpl extends UnicastRemoteObject implements ChatServer {
    private final Map<String, ClientProxy> clients;
    private final Map<String, List<ChatMessage>> chatMessages;

    public ChatServerImpl() throws RemoteException {
        System.out.println("Starting Server and init storage");
        clients = new ConcurrentHashMap<>();
        chatMessages = new ConcurrentHashMap<>();
    }

    @Override
    public ChatProxy subscribeUser(String username, ClientProxy handle) throws RemoteException {
        // Check if user exists
        if (clients.containsKey(username)) {
            System.out.println("User already exists");
            return new ChatProxyImpl(this, username);
        }

        clients.put(username, handle);
        System.out.println("Joined the Server: " + username);
        broadcastMessage(username, "Joined the Server");
        return new ChatProxyImpl(this, username);
    }

    @Override
    public boolean unsubscribeUser(String username) throws RemoteException {
        if (!clients.containsKey(username)) {
            System.out.println("User does not exist");
            throw new RemoteException("User does not exist");
        }
        return clients.remove(username) != null;
    }

    @Override
    public void broadcastMessage(String sender, String message) {
        ChatMessage chatMessage = new ChatMessage(sender, message, "https://www.gravatar.com/avatar/205e460b479e2e5b48aec07710c08d50?f=y");
        chatMessages.computeIfAbsent(sender, k -> new ArrayList<>()).add(chatMessage);

        clients.forEach((username, client) -> {
            if (!username.equals(sender)) {
                try {
                    client.receiveMessage(sender, message);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
        });
    }

    @Override
    public void sendPrivateMessage(String sender, String recipient, String message) throws RemoteException {
        ClientProxy recipientClient = clients.get(recipient);
        if (recipientClient != null) {
            recipientClient.receivePrivMessage(sender, message);
            ChatMessage chatMessage = new ChatMessage(sender, message, "https://www.gravatar.com/avatar/205e460b479e2e5b48aec07710c08d50?f=y");
            chatMessages.computeIfAbsent(recipient, k -> new ArrayList<>()).add(chatMessage);
        } else {
            throw new RemoteException("Recipient does not exist");
        }
    }

    @Override
    public void sendPrivateMessage(String sender, String recipient, String message, String senderAvatarUrl) throws RemoteException {
        ClientProxy recipientClient = clients.get(recipient);
        if (recipientClient != null) {
            recipientClient.receivePrivMessage(sender, message);
            ChatMessage chatMessage = new ChatMessage(sender, message, senderAvatarUrl);
            chatMessages.computeIfAbsent(recipient, k -> new ArrayList<>()).add (chatMessage);
        } else {
            throw new RemoteException("Recipient does not exist");
        }
    }

    @Override
    public List<ChatMessage> getChatMessages(String username) throws RemoteException {
        if (username == null || username.isEmpty()) {
            ArrayList<ChatMessage> allMessages = new ArrayList<>();
            chatMessages.forEach((user, messages) -> allMessages.addAll(messages));
            return allMessages;
        }
        return chatMessages.getOrDefault(username, new ArrayList<>());
    }


    @Override
    public Set<String> getActiveUsers() throws RemoteException {
        return new HashSet<>(clients.keySet());
    }
}
