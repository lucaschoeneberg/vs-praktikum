import java.rmi.RemoteException;
import java.rmi.server.UnicastRemoteObject;

public class ClientProxyImpl extends UnicastRemoteObject implements ClientProxy {
    public ClientProxyImpl() throws RemoteException {
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