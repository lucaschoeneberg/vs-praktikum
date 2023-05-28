package com.example.webchat.rmi;

import com.example.webchat.ChatMessage;

import java.rmi.Remote;
import java.rmi.RemoteException;
import java.util.List;
import java.util.Set;

public interface ChatServer extends Remote {
    public ChatProxy subscribeUser(String username, ClientProxy handle) throws RemoteException;

    public boolean unsubscribeUser(String username) throws RemoteException;

    public void sendPrivateMessage(String sender, String recipient, String message) throws RemoteException;

    public void sendPrivateMessage(String sender, String recipient, String message, String senderAvatarUrl) throws RemoteException;

    public void broadcastMessage(String sender, String message) throws RemoteException;

    public Set<String> getActiveUsers() throws RemoteException;

    List<ChatMessage> getChatMessages(String username) throws RemoteException;
}

