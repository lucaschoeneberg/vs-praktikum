package com.example.webchat;

import com.example.webchat.rmi.*;

import jakarta.servlet.ServletException;
import jakarta.servlet.annotation.WebServlet;
import jakarta.servlet.http.*;

import java.io.IOException;
import java.rmi.Naming;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;

@WebServlet(name = "ChatServlet", urlPatterns = {"/chat"})
public class ChatServlet extends HttpServlet {
    private ChatServer chatServer;

    private final ArrayList<String> activeUsers = new ArrayList<>();

    public String rmiURL = "rmi://localhost:2000/rmi.ChatServer";

    @Override
    public void init() throws ServletException {
        super.init();
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
            activeUsers.addAll(chatServer.getActiveUsers());

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
    protected void doGet(HttpServletRequest req, HttpServletResponse resp) throws ServletException, IOException {
        try {
            String username = (String) req.getSession().getAttribute("username");

            if (username == null) {
                resp.sendRedirect(req.getContextPath() + "/");
                return;
            }
            req.setAttribute("username", username);

            Set<String> activeUsers = chatServer.getActiveUsers();
            req.setAttribute("activeUsers", activeUsers);

            List<ChatMessage> chatMessages = chatServer.getChatMessages(req.getParameter(null));
            System.out.println(chatMessages);
            req.setAttribute("chatMessages", chatMessages);

            req.getRequestDispatcher("chat.jsp").forward(req, resp);
        } catch (Exception e) {
            e.printStackTrace();
            throw new ServletException("Fehler beim Abrufen der Chat-Nachrichten", e);
        }
    }

    @Override
    protected void doPost(HttpServletRequest req, HttpServletResponse resp) throws ServletException, IOException {
        String message = req.getParameter("message");
        String operation = req.getParameter("operation");

        switch (operation) {
            case "send" -> {
                try {
                    HttpSession session = req.getSession(false);
                    if (session != null) {
                        String username = (String) session.getAttribute("username");
                        if (username != null && !username.isEmpty() && message != null && !message.isEmpty()) {
                            chatServer.broadcastMessage(username, message);
                        }
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                    throw new ServletException("Fehler beim Senden der Nachricht", e);
                }
            }
            case "unsubscribe" -> {
                HttpSession session = req.getSession(false);
                if (session != null) {
                    String username = (String) session.getAttribute("username");
                    try {
                        chatServer.unsubscribeUser(username);
                        session.invalidate();
                        resp.sendRedirect(req.getContextPath() + "/");
                        return;
                    } catch (Exception e) {
                        e.printStackTrace();
                        throw new ServletException("Fehler beim Abmelden des Benutzers", e);
                    }
                }
            }
            case "subscribe" -> {
                String username = req.getParameter("username");
                if (username != null && !username.isEmpty()) {
                    ClientProxy clientProxy = new ClientProxyImpl();
                    try {
                        ChatProxy chatProxy = chatServer.subscribeUser(username, clientProxy);
                        HttpSession session = req.getSession(true);
                        session.setAttribute("username", username);
                        session.setAttribute("chatProxy", chatProxy);
                        req.setAttribute("username", username);
                    } catch (Exception e) {
                        e.printStackTrace();
                        throw new ServletException("Fehler beim Anmelden des Benutzers", e);
                    }
                }
            }
        }

        resp.sendRedirect(req.getContextPath() + "/chat");
    }

    void updatMessages(HttpServletRequest req, HttpServletResponse resp) throws ServletException {
        try {
            String username = (String) req.getSession().getAttribute("username");
            List<ChatMessage> chatMessages = chatServer.getChatMessages(null);
            req.setAttribute("chatMessages", chatMessages);
        } catch (Exception e) {
            e.printStackTrace();
            throw new ServletException("Fehler beim Abrufen der Chat-Nachrichten", e);
        }
    }

    @Override
    public void destroy() {
        super.destroy();
        try {
            chatServer.unsubscribeUser("");
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}


/*
package com.example.webchat;

import com.example.webchat.rmi.*;

import jakarta.servlet.ServletException;
import jakarta.servlet.annotation.WebServlet;
import jakarta.servlet.http.*;

import java.io.IOException;
import java.rmi.Naming;
import java.util.List;
import java.util.Set;

@WebServlet(name = "ChatServlet", urlPatterns = {"/chat"})
public class ChatServlet extends HttpServlet {
    private ChatServer chatServer;

    public String rmiURL = "rmi://localhost:2000/rmi.ChatServer";

    @Override
    public void init() throws ServletException {
        super.init();
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
    protected void doGet(HttpServletRequest req, HttpServletResponse resp) throws ServletException, IOException {
        Cookie[] cookies = req.getCookies();
        String username = null;
        if (cookies != null) {
            for (Cookie cookie : cookies) {
                if (cookie.getName().equals("username")) {
                    username = cookie.getValue();
                    break;
                }
            }
        }

        if (username == null) {
            // If the username cookie does not exist, redirect to the root page
            resp.sendRedirect(req.getContextPath() + "/");
            return;
        }
        try {
            HttpSession session = req.getSession(true);
            chatServer.unsubscribeUser(username);
            ChatProxy chatProxy = chatServer.subscribeUser(username, new ClientProxyImpl(session.getId()));
            session.setAttribute("chatProxy", chatProxy);
            session.setAttribute("username", username);

            Set<String> activeUsers = chatServer.getActiveUsers();
            req.setAttribute("activeUsers", activeUsers);

            List<ChatMessage> chatMessages = chatServer.getChatMessages("");
            req.setAttribute("chatMessages", chatMessages);

            req.getRequestDispatcher("/chat.jsp").forward(req, resp);
        } catch (Exception e) {
            e.printStackTrace();
            throw new ServletException("Fehler beim Abonnement des Benutzers", e);
        }


    }

    @Override
    protected void doPost(HttpServletRequest req, HttpServletResponse resp) throws ServletException, IOException {
        String reciver = req.getParameter("reciver");
        String message = req.getParameter("message");
        String operation = req.getParameter("operation");

        switch (operation) {
            case "subscribe": {
                if (!operation.equals("subscribe")) break;
                String username = req.getParameter("username");
                // Chat-Abonnement für den Benutzer durchführen
                if (!username.isEmpty() && isValidUsername(username)) {
                    try {
                        HttpSession session = req.getSession(true);
                        ChatProxy chatProxy = chatServer.subscribeUser(username, new ClientProxyImpl(session.getId()));
                        session.setAttribute("chatProxy", chatProxy);
                        session.setAttribute("username", username);

                        Cookie usernameCookie = new Cookie("username", username);
                        usernameCookie.setMaxAge(60 * 60 * 24); // Setzen Sie die Lebensdauer des Cookies auf 24 Stunden
                        resp.addCookie(usernameCookie); // Fügen Sie den Cookie zur Antwort hinzu
                    } catch (Exception e) {
                        e.printStackTrace();
                        // HTML
                        req.setAttribute("error", "Der Benutzername ist bereits vergeben");
                    }
                }
            }
            case "unsubscribe": {
                if (!operation.equals("unsubscribe")) break;
                HttpSession session = req.getSession(false);
                if (session != null) {
                    String username = (String) session.getAttribute("username");
                    // Benutzer vom Chat abmelden
                    try {
                        chatServer.unsubscribeUser(username);
                        resp.sendRedirect(req.getContextPath() + "/");
                    } catch (Exception e) {
                        e.printStackTrace();
                        // HTML
                        req.setAttribute("error", "Fehler beim Abmelden des Benutzers");
                    }
                    session.invalidate();
                }
            }
            case "send": {
                if (!operation.equals("send")) break;
                HttpSession session = req.getSession(false); // false bedeutet, dass keine neue Session erstellt wird, wenn keine existiert
                if (session != null) {
                    String username = (String) session.getAttribute("username"); // Benutzername extrahieren
                    ChatProxy chatProxy = (ChatProxy) session.getAttribute("chatProxy");
                    if (chatProxy == null) {
                        chatProxy = chatServer.subscribeUser(username, new ClientProxyImpl());
                        session.setAttribute("chatProxy", chatProxy);
                    }
                    // Check if the user want to exit the Chat
                    try {
                        if (username != null && !username.isEmpty() && message != null && !message.isEmpty()) {
                            chatServer.sendPrivateMessage(username, reciver, message);
                        } else if (message != null && !message.isEmpty()) {
                            chatProxy.sendMessage(message);
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                        // HTML
                        req.setAttribute("error", "Fehler beim Senden der Nachricht");
                    }
                }
            }
            default: break;
        }
        resp.sendRedirect(req.getContextPath() + "/chat");
    }

    private static boolean isValidUsername(String username) {
        return username != null && !username.trim().isEmpty() && username.matches("^[a-zA-Z0-9_]*$");
    }
}
*/
