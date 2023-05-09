import java.rmi.Naming;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.util.Scanner;
import java.util.Set;

public class Main {

    static String rmiURL = "rmi://192.168.0.205:2000/ChatServer";

    public static void main(String[] args) {
        try {
            ChatServer chatServer = (ChatServer) Naming.lookup(rmiURL);

            Scanner scanner = new Scanner(System.in);
            System.out.print("Enter your username: ");
            String username = scanner.nextLine();

            ClientProxy clientProxy = new ClientProxyImpl();
            ChatProxy chatProxy = chatServer.subscribeUser(username, clientProxy);

            System.out.println("You have joined the chat. Type 'exit' to leave.");

            while (true) {
                String message = scanner.nextLine();
                if (message.equalsIgnoreCase("exit")) {
                    break;
                } else if (message.startsWith("/msg")) {
                    String[] parts = message.split(" ", 3);
                    if (parts.length == 3) {
                        String recipient = parts[1];
                        String privateMessage = parts[2];
                        chatProxy.sendPrivateMessage(recipient, privateMessage);
                    } else {
                        System.out.println("Invalid private message format. Use: /msg <recipient> <message>");
                    }
                } else if (message.equalsIgnoreCase("/users")) {
                    Set<String> activeUsers = chatProxy.getActiveUsers();
                    System.out.println("Active users: " + String.join(", ", activeUsers));
                } else {
                    chatProxy.sendMessage(message);
                }
            }

            chatServer.unsubscribeUser(username);
            System.out.println("You have left the chat.");
            scanner.close();

        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
