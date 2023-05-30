package com.example.ab07.rmi;

import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;

public class ServerMain {
    public static void main(String[] args) {
        try {
            ChatServerImpl chatServer = new ChatServerImpl();
            Registry registry = LocateRegistry.createRegistry(2000);
            registry.rebind("rmi.ChatServer", chatServer);
            System.out.println("Chat Server is running...");
        } catch (Exception e) {
            e.printStackTrace();
            System.exit(1);
        }
    }
}
