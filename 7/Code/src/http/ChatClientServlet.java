package http;

import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.ServletException;
import javax.servlet.http.HttpSession;
import java.io.IOException;
import java.rmi.Naming;
import java.util.Set;

import rmi.ChatServer;
import rmi.ChatProxy;
import rmi.ClientProxyImpl;

public class ChatClientServlet extends HttpServlet {
    static String rmiURL = "rmi://localhost:2000/rmi.ChatServer";
    private ChatServer chatServer;

    @Override
    public void init() throws ServletException {
        try {
            // Verbindung zum RMI-Server herstellen
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
            System.out.println("Connected to the server.");
        } catch (Exception e) {
            e.printStackTrace();
            throw new ServletException("Fehler bei der Initialisierung des Chat-Servers", e);
        }
    }

    private static ChatServer connectToServer(String rmiURL) {
        try {
            return (ChatServer) Naming.lookup(rmiURL);
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }

    @Override
    protected void doPost(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        String operation = request.getParameter("operation");

        switch (operation) {
            case "subscribe" -> {
                String username = request.getParameter("username");
                // Chat-Abonnement für den Benutzer durchführen
                try {
                    HttpSession session = request.getSession(true);
                    ChatProxy chatProxy = chatServer.subscribeUser(username, new ClientProxyImpl(session.getId()));
                    session.setAttribute("chatProxy", chatProxy);
                    session.setAttribute("username", username);
                } catch (Exception e) {
                    e.printStackTrace();
                    throw new ServletException("Fehler beim Abonnement des Benutzers", e);
                }
            }
            case "unsubscribe" -> {
                HttpSession session = request.getSession(false);
                if (session != null) {
                    String username = (String) session.getAttribute("username");
                    // Benutzer vom Chat abmelden
                    try {
                        chatServer.unsubscribeUser(username);
                    } catch (Exception e) {
                        e.printStackTrace();
                        throw new ServletException("Fehler beim Abmelden des Benutzers", e);
                    }
                    session.invalidate();
                }
            }
            case "send" -> {
                HttpSession session = request.getSession(false);
                if (session != null) {
                    String sender = (String) session.getAttribute("username");
                    String recipient = request.getParameter("recipient");
                    String message = request.getParameter("message");
                    // Private Nachricht senden
                    try {
                        chatServer.sendPrivateMessage(sender, recipient, message);
                    } catch (Exception e) {
                        e.printStackTrace();
                        throw new ServletException("Fehler beim Senden der Nachricht", e);
                    }
                }
            }
        }

        response.sendRedirect(request.getContextPath() + "/chat.jsp");
    }

    @Override
    protected void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        HttpSession session = request.getSession(false);

        if (session != null) {
            String operation = request.getParameter("operation");

            if (operation.equals("show")) {
                // Liste der aktiven Benutzer abrufen
                try {
                    Set<String> activeUsers = chatServer.getActiveUsers();
                    request.setAttribute("activeUsers", activeUsers);
                } catch (Exception e) {
                    e.printStackTrace();
                    throw new ServletException("Fehler beim Abrufen der aktiven Benutzer", e);
                }
            }
        }

        request.getRequestDispatcher("/chat.jsp").forward(request, response);
    }
}
