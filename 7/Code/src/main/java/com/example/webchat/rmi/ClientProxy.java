package com.example.webchat.rmi;

import java.rmi.Remote;
import java.rmi.RemoteException;

public interface ClientProxy extends Remote {
    void receiveMessage(String username, String message) throws RemoteException;

    public void receivePrivMessage(String username, String message) throws RemoteException;
}