package rmi;

import java.rmi.RemoteException;
import java.rmi.server.UnicastRemoteObject;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

public class ChatServerImpl extends UnicastRemoteObject implements ChatServer {
    private final Map<String, ClientProxy> clients;

    public ChatServerImpl() throws RemoteException {
        System.out.println("Starting Server and init storage");
        clients = new ConcurrentHashMap<>();
    }

    @Override
    public ChatProxy subscribeUser(String username, ClientProxy handle) throws RemoteException {
        // Check if user exists
        if (clients.containsKey(username)) {
            System.out.println("User already exists");
            throw new RemoteException("User already exists");
        }

        clients.put(username, handle);

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

    void broadcastMessage(String sender, String message) {
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
        } else {
            throw new RemoteException("Recipient does not exist");
        }
    }

    @Override
    public Set<String> getActiveUsers() throws RemoteException {
        return new HashSet<>(clients.keySet());
    }
}
