<%@ page import="com.example.webchat.ChatMessage" %>
<%@ page import="java.util.List" %>
<%@ page import="java.util.Set" %>
<%@ page language="java" contentType="text/html; charset=UTF-8" pageEncoding="UTF-8" %>
<!DOCTYPE html>
<html>
<head>
    <title>Chat</title>
</head>
<body>
<h1>Webchat</h1>

<h2>Logged in as: </h2>
<% String username = (String) request.getAttribute("username");
    if (username != null) { %>
<div>
    <h3><%= username %></h3>
</div>
<% } %>

<form action="chat" method="POST">
    <input type="hidden" name="operation" value="unsubscribe">
    <input type="submit" value="Logout">
</form>

<h2>Active Users</h2>
<% Set<String> activeUsers = (Set<String>) request.getAttribute("activeUsers");
    if (activeUsers != null) {
        for (String user : activeUsers) { %>
<div>
    <%= user %>
</div>
<% }
} %>

<h2>Chat Messages</h2>
<% List<ChatMessage> messages = (List<ChatMessage>) request.getAttribute("chatMessages");
    if (messages != null) {
        for (ChatMessage message : messages) { %>
<div>
    <strong><%= message.sender() %></strong>: <%= message.message() %>
</div>
<% }
} %>

<form action="chat" method="post">
    <label>
        <input type="text" name="message" placeholder="type your message here..."/>
    </label>
    <input type="hidden" name="operation" value="send">
    <input type="submit" value="Send">
</form>
</body>
</html>



<%--
<%@ page import="com.example.webchat.ChatMessage" %>
<%@ page import="java.util.List" %>
<%@ page import="java.util.Set" %>
<%@ page language="java" contentType="text/html; charset=UTF-8" pageEncoding="UTF-8" %>
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Chat</title>
    <link href="css/index.css" type="text/css" rel="stylesheet">
</head>
<body>
<div class="container mx-auto shadow-lg rounded-lg">
    <!-- header -->
    <div class="px-5 py-5 flex justify-between items-center bg-white border-b-2">
        <div class="font-semibold text-2xl">AB07 Webchat</div>
        <!-- Logout button -->
        <form action="?" method="POST">
            <input type="hidden" name="operation" value="unsubscribe">
            <input type="submit" value="Logout" class="logout-button">
        </form>
    </div>
    <!-- end header -->
    <!-- Chatting -->
    <div class="flex flex-row justify-between bg-white">
        <!-- chat list -->
        <div class="flex flex-col w-2/5 border-r-2 overflow-y-auto">
            <!-- end search compt -->
            <!-- user list -->
            &lt;%&ndash; Dynamically iterate over active users and display them &ndash;%&gt;
            <% Set<String> activeUsers = (Set<String>) request.getAttribute("activeUsers");
                if (activeUsers != null) {
                    for (String user : activeUsers) { %>
            <div class="flex flex-row py-4 px-2 justify-center items-center border-b-2">
                <div class="w-1/4">
                    <img src="https://source.unsplash.com/_7LbC5J-jw4/600x600"
                         class="object-cover h-12 w-12 rounded-full" alt=""/>
                </div>
                <div class="w-full">
                    <div class="text-lg font-semibold"><%= user %>
                    </div>
                </div>
            </div>
            <% }
            } %>
            <!-- end user list -->
        </div>
        <!-- end chat list -->
        <!-- message -->
        <div class="w-full px-5 flex flex-col justify-between">
            <div class="flex flex-col mt-5">
                &lt;%&ndash; Dynamically iterate over chat messages and display them &ndash;%&gt;
                <% List<ChatMessage> messages = (List<ChatMessage>) request.getAttribute("chatMessages");
                    if (messages != null) {
                        for (ChatMessage message : messages) { %>
                <div class="flex justify-end mb-4">
                    <div class="mr-2 py-3 px-4 bg-blue-400 rounded-bl-3xl rounded-tl-3xl rounded-tr-xl text-white">
                        <%= message.message() %>
                    </div>
                    <img src="https://source.unsplash.com/vpOeXr5wmR4/600x600" class="object-cover h-8 w-8 rounded-full"
                         alt=""/>
                </div>
                <% }
                } %>
            </div>
            <div class="py-5">
                <form action="?" method="post">
                    <label>
                        <input class="w-full bg-gray-300 py-5 px-3 rounded-xl" type="text" name="message" placeholder="type your message here..."/>
                    </label>
                    <input type="hidden" name="operation" value="send">
                    <input type="submit" value="Send" class="send-button">
                </form>
            </div>
        </div>
        <!-- end message -->
    </div>
</div>
</body>
</html>--%>