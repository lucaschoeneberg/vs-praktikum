import java.rmi.Remote;
import java.rmi.RemoteException;
import java.util.Set;

public interface ChatServer extends Remote {
    public ChatProxy subscribeUser(String username, ClientProxy handle) throws RemoteException;

    public boolean unsubscribeUser(String username) throws RemoteException;

    public void sendPrivateMessage(String sender, String recipient, String message) throws RemoteException;

    public Set<String> getActiveUsers() throws RemoteException;
}
