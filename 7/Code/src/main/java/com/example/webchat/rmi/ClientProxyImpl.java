package com.example.webchat.rmi;

import java.rmi.RemoteException;
import java.rmi.server.UnicastRemoteObject;

public class ClientProxyImpl extends UnicastRemoteObject implements ClientProxy {
    private String sessionId;

    public ClientProxyImpl() throws RemoteException {
    }

    public ClientProxyImpl(String sessionId) throws RemoteException {
        this.sessionId = sessionId;
    }

    @Override
    public void receiveMessage(String username, String message) throws RemoteException {
        System.out.println(username + ": " + message);
    }

    @Override
    public void receivePrivMessage(String username, String message) throws RemoteException {
        System.out.println(username + " whispers: " + message);
    }
}