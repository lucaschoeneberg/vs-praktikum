<%@ page import="http.ChatMessage" %>
<%@ page import="java.util.List" %>
<%@ page import="java.util.Set" %>
<%@ page language="java" contentType="text/html; charset=UTF-8" pageEncoding="UTF-8" %>
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Chat</title>
</head>
<body>
<div class="flex flex-row h-full bg-gray-200">
    <!-- Sidebar for active users -->
    <div class="bg-white w-64 px-4 py-6 h-full overflow-y-auto">
        <h2 class="text-2xl font-semibold mb-2">Aktive Benutzer</h2>
        <ul>
            <%-- Dynamically iterate over active users and display them --%>
            <% Set<String> activeUsers = (Set<String>) request.getAttribute("activeUsers");
                if (activeUsers != null) {
                    for (String user : activeUsers) { %>
            <li class="my-2"><%= user %>
            </li>
            <% }
            } %>
        </ul>
    </div>

    <!-- Chat window -->
    <div class="flex flex-col w-full">
        <!-- Chat messages -->
        <div class="overflow-y-auto flex-grow">
            <div class="p-4">
                <%-- Dynamically iterate over chat messages and display them --%>
                <%-- Replace "messages" with your actual list of chat messages --%>
                <% List<ChatMessage> messages = (List<ChatMessage>) request.getAttribute("chatMessages");
                    if (messages != null) {
                        for (ChatMessage message : messages) { %>
                <div class="flex items-end mb-2">
                    <div class="flex flex-col space-y-2 text-xs max-w-xs mx-2 order-2 items-start">
                        <div>
                                        <span class="px-4 py-2 rounded-lg inline-block rounded-bl-none bg-blue-600 text-white">
                                            <%= message.getSender() %>: <%= message.getMessage() %>
                                        </span>
                        </div>
                    </div>
                    <img src="<%= message.getSenderAvatarUrl() %>" alt="Avatar of <%= message.getSender() %>"
                         class="w-6 h-6 rounded-full order-1"/>
                </div>
                <% }
                } %>
            </div>
        </div>

        <!-- Input for new message -->
        <div class="border-t-2 border-gray-200 px-4 pt-4 mb-2 sm:mb-0">
            <div class="relative flex">
                <input type="text" placeholder="Schreibe eine Nachricht..."
                       class="w-full focus:outline-none focus:placeholder-gray-400 text-gray-600 placeholder-gray-600 pl-12 bg-gray-200 rounded-full py-3">
                <div class="absolute right-0 items-center inset-y-0 hidden sm:flex">
                    <button type="button"
                            class="inline-flex items-center justify-center rounded-full h-12 w-12 transition duration-500 ease-in-out text-white bg-blue-500 hover:bg-blue-400 focus:outline-none">
                        <svg xmlns="http://www.w3.org/2000/svg" fill="none" stroke="currentColor" stroke-linecap="round"
                             stroke-linejoin="round" stroke-width="2" viewBox="0 0 24 24" class="w-6 h-6">
                            <line x1="22" y1="2" x2="11" y2="13"></line>
                            <polygon points="22 2 15 22 11 13 2 9 22 2"></polygon>
                        </svg>
                    </button>
                </div>
            </div>
        </div>
    </div>
</div>
</body>
</html>
