package com.example.ab07.rmi;

import java.rmi.Remote;
import java.rmi.RemoteException;
import java.util.Set;

public interface ChatProxy extends Remote {
    void sendMessage(String message) throws RemoteException;

    public void sendPrivateMessage(String recipient, String message) throws RemoteException;

    public Set<String> getActiveUsers() throws RemoteException;
}
