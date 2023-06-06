package de.hsos.vs;

import jakarta.websocket.*;
import jakarta.websocket.server.ServerEndpoint;

import java.io.IOException;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;

import org.json.JSONArray;
import org.json.JSONObject;

/**
 * Websocket Endpoint für das Billboard.
 * <p>
 * Dieser Endpoint wird von der Klasse {@link BillBoardWebsocket} implementiert.
 * Er ist für die Kommunikation zwischen dem Billboard und den Clients zuständig.
 * Die Kommunikation erfolgt über JSON-Objekte.
 * <br>
 * Die JSON-Objekte haben folgenden Aufbau:
 * <pre>
 *         {
 *         "type": "messageType",
 *         "content": "messageContent",
 *         "sender": "messageSender"
 *         }
 *         </pre>
 * <b>type</b>: Der Typ der Nachricht. Dieser kann folgende Werte annehmen:
 * <ul>
 *     <li>connection: Wird beim Verbindungsaufbau gesendet.</li>
 *     <li>set: Wird beim Setzen einer Nachricht gesendet.</li>
 *     <li>error: Wird bei einem Fehler gesendet.</li>
 *     <li>info: Wird bei einer Information gesendet.</li>
 *     <li>update: Wird bei einem Update gesendet.</li>
 *     <li>delete: Wird beim Löschen einer Nachricht gesendet.</li>
 *     <li>deleteAll: Wird beim Löschen aller Nachrichten gesendet.</li>
 *     <li>ping: Wird beim Senden eines Pings gesendet.</li>
 *     <li>pong: Wird beim Senden eines Pongs gesendet.</li>
 *     <li>close: Wird beim Schließen der Verbindung gesendet.</li>
 *     <li>unknown: Wird bei einer unbekannten Nachricht gesendet.</li>
 *     </ul>
 * <b>content</b>: Der Inhalt der Nachricht. Dieser kann folgende Werte annehmen:
 * <ul>
 *     <li>string: Der Inhalt ist ein String.</li>
 *     <li>json: Der Inhalt ist ein JSON-Objekt.</li>
 *     </ul>
 *     <b>sender</b>: Der Sender der Nachricht. Dieser kann folgende Werte annehmen:
 *     <ul>
 *         <li>server: Die Nachricht wurde vom Server gesendet.</li>
 *         <li>client: Die Nachricht wurde von einem Client gesendet.</li>
 *         </ul>
 */

@ServerEndpoint(value = "/billboard")
public class BillBoardWebsocket {
    private static ArrayList<Session> sessions = new ArrayList<>();
    private static BillBoardHtmlAdapter billBoardHtmlAdapter = new BillBoardHtmlAdapter("BillBoardServer");
    private static ArrayList<String> stateSessionShort = new ArrayList<>();
    private static ArrayList<String> stateSessionLong = new ArrayList<>();
    private static ConcurrentMap<String, Long> stateSessionPendingAck = new ConcurrentHashMap<>();


    @OnOpen
    public void onOpen(Session session, EndpointConfig config) {
        sessions.add(session);
        session.setMaxIdleTimeout(5 * 60 * 1000); // 5 minutes
        stateSessionShort.add(session.getId());

        try {
            sendJsonMessage(session, "connection", new JSONObject().put("session", session.getId()));
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    @OnMessage
    public void onMessage(Session session, String message) throws IOException {
        // messageType: set, update, delete, deleteAll
        // messageType set: {"type":"set","content":{"message":"messageContent"},"sender":"messageSender"}
        // messageType update: {"type":"update","content":{"id":"messageContent","message":"messageContent"},"sender":"messageSender"}
        // messageType delete: {"type":"delete","content":{"id":"messageContent"},"sender":"messageSender"}
        // messageType deleteAll: {"type":"deleteAll","content":{},"sender":"messageSender"}

        // get Type:
        JSONObject jsonObject = new JSONObject(message);
        String type = jsonObject.getString("type");
        String sender = jsonObject.getString("sender");

        if (!sender.equals("client")) {
            sendJsonMessage(session, "error", "Unknown sender.");
            return;
        }

        if (!jsonObject.has("content")) {
            sendJsonMessage(session, "error", "Missing content.");
            return;
        }

        System.out.println("Message received: " + message);

        JSONObject content = jsonObject.getJSONObject("content");

        switch (type) {
            case "get": {
                System.out.println("Entry requested. Command received from session " + session.getId());

                JSONArray jsonArray = getEntries();
                if (session.isOpen())
                    sendJsonMessage(session, "get", jsonArray);
                break;
            }
            case "set": {
                if (!content.has("message") || content.getString("message").isEmpty()) {
                    sendJsonMessage(session, "error", "Missing or empty message content.");
                    return;
                }
                System.out.println("Entry created. Command received from session " + session.getId());

                String messageContent = content.getString("message");
                Integer messageId = billBoardHtmlAdapter.createEntry(messageContent, session.getId());

                sendToAllClients(new JSONObject()
                        .put("type", "set")
                        .put("content", new JSONObject()
                                .put("message", messageContent)
                                .put("id", messageId)
                                .put("sessionId", session.getId()))
                        .put("sender", "server"));
                break;
            }
            case "update": {
                if (!content.has("message") || content.getString("message").isEmpty()) {
                    sendJsonMessage(session, "error", "Missing or empty message content.");
                    return;
                }
                System.out.println("Entry " + content.getInt("id") + " updated. Command received from session " + session.getId());

                Integer id = content.getInt("id");
                String messageContent = content.getString("message");
                billBoardHtmlAdapter.updateEntry(id, messageContent, session.getId());
                sendToAllClients(new JSONObject()
                        .put("type", "update")
                        .put("content", new JSONObject()
                                .put("id", id)
                                .put("message", messageContent)
                                .put("sessionId", session.getId()))
                        .put("sender", "server"));
                break;
            }
            case "delete": {
                Integer id = content.getInt("id");
                System.out.println("Entry " + id + " deleted. Command received from session " + session.getId());
                billBoardHtmlAdapter.deleteEntry(id);
                sendToAllClients(new JSONObject()
                        .put("type", "delete")
                        .put("content", new JSONObject()
                                .put("id", id))
                        .put("sender", "server"));
                break;
            }
            case "deleteAll": {
                billBoardHtmlAdapter.deleteAllEntries();
                System.out.println("All entries deleted. Command received from session " + session.getId());

                sendToAllClients(new JSONObject()
                        .put("type", "deleteAll")
                        .put("content", "")
                        .put("sender", "server"));
                break;
            }
            case "ack": {
                System.out.println("Ack received from session " + session.getId());
                stateSessionPendingAck.remove(session.getId());
                break;
            }
            case "ping": {
                System.out.println("Ping received from session " + session.getId());
                sendJsonMessage(session, "pong", "");
                break;
            }
            default: {
                // unknown message type
                sendJsonMessage(session, "unknown", "Unknown message type.");
                break;
            }
        }
    }

    /**
     * Sendet eine Nachricht an alle Clients.
     *
     * @param org.json.JSONObject Das JSON-Objekt, welches gesendet werden soll.
     * @throws IOException
     */
    private void sendToAllClients(JSONObject jsonObject) throws IOException {
        // Send and check if client received message (if not, add to stateSessionLong)
        long currentTimestamp = System.currentTimeMillis();
        for (Session endpoint : sessions) {
            if (stateSessionLong.contains(endpoint.getId())) {
                try {
                    JSONArray jsonArray = getEntries();
                    if (endpoint.isOpen())
                        endpoint.getBasicRemote().sendText(new JSONObject().put("type", "set").put("content", jsonArray).put("sender", "server").toString());
                } catch (IOException e) {
                    stateSessionLong.add(endpoint.getId());
                }
            } else {
                try {
                    endpoint.getBasicRemote().sendText(jsonObject.toString());
                    stateSessionLong.remove(endpoint.getId());
                    stateSessionPendingAck.put(endpoint.getId(), currentTimestamp);
                } catch (IOException e) {
                    stateSessionShort.remove(endpoint.getId());
                    stateSessionLong.add(endpoint.getId());
                    stateSessionPendingAck.remove(endpoint.getId());
                }
            }
        }
        checkPendingAcks();
    }

    private void checkPendingAcks() {
        long currentTimestamp = System.currentTimeMillis();
        List<String> sessionsToRemove = new ArrayList<>();

        for (Map.Entry<String, Long> entry : stateSessionPendingAck.entrySet()) {
            if (currentTimestamp - entry.getValue() > 10000) {
                String sessionId = entry.getKey();
                sessionsToRemove.add(sessionId);
                stateSessionLong.add(sessionId);
            }
        }

        for (String sessionId : sessionsToRemove) {
            stateSessionShort.remove(sessionId);
            stateSessionPendingAck.remove(sessionId);
        }
    }


    @OnClose
    public void onClose(Session session) throws IOException {
        if (session.isOpen()) {
            session.getBasicRemote().sendText(new JSONObject().put("type", "close").put("content", new JSONObject()).put("sender", "server").toString());
        }
        stateSessionShort.remove(session.getId());
        stateSessionLong.remove(session.getId());
        sessions.remove(this);
    }

    @OnError
    public void onError(Session session, Throwable throwable) {
        try {
            session.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private JSONArray getEntries() {
        JSONArray jsonArray = new JSONArray();
        // get all messages and add to jsonArray Map<Integer, String>
        for (Map.Entry<Integer, JSONObject> entry : billBoardHtmlAdapter.readEntriesListJson().entrySet()) {
            jsonArray.put(entry.getValue());
        }
        return jsonArray;
    }

    private void sendJsonMessage(Session session, String type, Object content) throws IOException {
        JSONObject jsonObject = new JSONObject().put("type", type).put("content", content).put("sender", "server");
        if (session.isOpen()) {
            session.getBasicRemote().sendText(jsonObject.toString());
        }
    }
}