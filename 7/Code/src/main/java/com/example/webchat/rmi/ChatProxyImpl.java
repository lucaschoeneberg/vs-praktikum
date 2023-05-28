package com.example.webchat.rmi;

import java.rmi.RemoteException;
import java.rmi.server.UnicastRemoteObject;
import java.util.Set;

public class ChatProxyImpl extends UnicastRemoteObject implements ChatProxy {
    private final ChatServerImpl server;
    private final String username;

    public ChatProxyImpl(ChatServerImpl server, String username) throws RemoteException {
        this.server = server;
        this.username = username;
    }

    @Override
    public void sendMessage(String message) throws RemoteException {
        server.broadcastMessage(username, message);
    }

    @Override
    public void sendPrivateMessage(String recipient, String message) throws RemoteException {
        server.sendPrivateMessage(username, recipient, message);
    }

    @Override
    public Set<String> getActiveUsers() throws RemoteException {
        return server.getActiveUsers();
    }

}

