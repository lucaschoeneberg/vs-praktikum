import rmi.ChatClient;
import rmi.ChatServerImpl;

import http.ChatClientServlet;

public class ChatHTTP {

    public static void main(String[] args) {
        try {
            ChatClientServlet chatClient = new ChatClientServlet();
            chatClient.init();
        } catch (Exception e) {
            e.printStackTrace();
            System.exit(1);
        }
    }
}
