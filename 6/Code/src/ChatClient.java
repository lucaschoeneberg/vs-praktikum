import java.rmi.Naming;
import java.util.Scanner;
import java.util.Set;

public class ChatClient {

    static String rmiURL = "rmi://192.168.0.205:2000/ChatServer";

    public static void main(String[] args) {
        try {
            ChatServer chatServer;
            while (true) {
                // Connect to the server
                chatServer = connectToServer(rmiURL);
                if (chatServer == null) {
                    System.out.println("Could not connect to the server. Retrying in 5 seconds...");
                    Thread.sleep(5000);
                } else {
                    break;
                }
            }

            Scanner scanner = new Scanner(System.in);
            String username;
            while (true) {
                System.out.print("Enter your username: ");
                username = scanner.nextLine();
                if (isValidUsername(username)) {
                    try {
                        if (chatServer.getActiveUsers().contains(username)) {
                            System.out.println("Username already in use. Please choose another one.");
                        } else {
                            break;
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                } else {
                    System.out.println("Invalid username. Use only letters, numbers and underscores.");
                }
            }

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

    private static boolean isValidUsername(String username) {
        return username != null && !username.trim().isEmpty() && username.matches("^[a-zA-Z0-9_]*$");
    }

    private static ChatServer connectToServer(String rmiURL) {
        try {
            return (ChatServer) Naming.lookup(rmiURL);
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }

}
