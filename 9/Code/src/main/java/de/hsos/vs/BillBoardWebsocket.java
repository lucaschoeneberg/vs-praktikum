package de.hsos.vs;

import javax.websocket.*;
import javax.websocket.server.PathParam;
import javax.websocket.server.ServerEndpoint;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;

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

@ServerEndpoint(value = "/billboard/*")
public class BillBoardWebsocket {
    private Session session;
    private static final Set<BillBoardWebsocket> chatEndpoints = new CopyOnWriteArraySet<>();
    private BillBoardHtmlAdapter billBoardHtmlAdapter = new BillBoardHtmlAdapter("billboard.html");

    // Sessions die letzte Nachricht erhalten haben
    private ArrayList<String> stateSessionShort = new ArrayList<>();
    // Sessions die mehr als eine nachricht nicht erhalten haben
    private ArrayList<String> stateSessionLong = new ArrayList<>();

    @OnOpen
    public void onOpen(Session session, @PathParam("username") String username) throws IOException {
        this.session = session;
        chatEndpoints.add(this);
        try {
            // JSON: {"type":"messageType","content":"messageContent","sender":"messageSender"}
            JSONObject jsonObject = new JSONObject();
            jsonObject.put("type", "connection");
            JSONObject content = new JSONObject();
            content.put("session", session.getId());
            content.put("username", username);
            jsonObject.put("content", content);
            jsonObject.put("sender", "server");
            session.getBasicRemote().sendText(jsonObject.toString());
            stateSessionShort.add(session.getId());
        } catch (IOException e) {
            e.printStackTrace();
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
        JSONObject content = jsonObject.getJSONObject("content");
        String sender = jsonObject.getString("sender");
        if (!sender.equals("client")) {
            // unknown sender
            jsonObject = new JSONObject();
            jsonObject.put("type", "error");
            jsonObject.put("content", "Unknown sender.");
            jsonObject.put("sender", "server");
            session.getBasicRemote().sendText(jsonObject.toString());
            return;
        }
        switch (type) {
            case "set": {
                // set message
                String messageContent = content.getString("message");
                Integer messageId = billBoardHtmlAdapter.createEntry(messageContent, session.getId());
                JSONObject setMessage = new JSONObject();
                setMessage.put("type", "set");
                JSONObject setContent = new JSONObject();
                setContent.put("message", messageContent);
                setContent.put("id", messageId);
                setMessage.put("content", setContent);
                setMessage.put("sender", "server");
                sendToAllClients(setMessage);
                break;
            }
            case "update": {
                // update message
                String id = content.getString("id");
                String messageContent = content.getString("message");
                billBoardHtmlAdapter.updateEntry(Integer.parseInt(id), messageContent, session.getId());
                JSONObject updateMessage = new JSONObject();
                updateMessage.put("type", "update");
                JSONObject updateContent = new JSONObject();
                updateContent.put("id", id);
                updateContent.put("message", messageContent);
                updateMessage.put("content", updateContent);
                updateMessage.put("sender", "server");
                sendToAllClients(updateMessage);
                break;
            }
            case "delete": {
                // delete message
                String id = content.getString("id");
                billBoardHtmlAdapter.deleteEntry(Integer.parseInt(id));
                JSONObject deleteMessage = new JSONObject();
                deleteMessage.put("type", "delete");
                JSONObject deleteContent = new JSONObject();
                deleteContent.put("id", id);
                deleteMessage.put("content", deleteContent);
                deleteMessage.put("sender", "server");
                sendToAllClients(deleteMessage);
                break;
            }
            case "deleteAll": {
                // delete all messages
                billBoardHtmlAdapter.deleteAllEntries();
                JSONObject deleteAllMessage = new JSONObject();
                deleteAllMessage.put("type", "deleteAll");
                JSONObject deleteAllContent = new JSONObject();
                deleteAllMessage.put("content", deleteAllContent);
                deleteAllMessage.put("sender", "server");
                sendToAllClients(deleteAllMessage);
                break;
            }
            default: {
                // unknown message type
                jsonObject = new JSONObject();
                jsonObject.put("type", "unknown");
                jsonObject.put("content", "Unknown message type.");
                jsonObject.put("sender", "server");
                session.getBasicRemote().sendText(jsonObject.toString());
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
        for (BillBoardWebsocket endpoint : chatEndpoints) {
            synchronized (endpoint) {
                if (stateSessionShort.contains(endpoint.session.getId())) {
                    try {
                        endpoint.session.getBasicRemote().sendText(jsonObject.toString());
                        stateSessionLong.remove(endpoint.session.getId());
                    } catch (IOException e) {
                        stateSessionShort.remove(endpoint.session.getId());
                        stateSessionLong.add(endpoint.session.getId());
                    }
                } else {
                    try {
                        JSONObject jsonObject1 = new JSONObject();
                        jsonObject1.put("type", "set");
                        JSONArray jsonArray = new JSONArray();
                        // get all messages and add to jsonArray Map<Integer, String>
                        for (Map.Entry<Integer, String> entry : billBoardHtmlAdapter.readEntriesList().entrySet()) {
                            JSONObject jsonObject2 = new JSONObject();
                            jsonObject2.put("id", entry.getKey());
                            jsonObject2.put("message", entry.getValue());
                            jsonArray.put(jsonObject2);
                        }
                        jsonObject1.put("content", jsonArray);
                        jsonObject1.put("sender", "server");
                        endpoint.session.getBasicRemote().sendText(jsonObject1.toString());
                    } catch (IOException e) {
                        stateSessionLong.add(endpoint.session.getId());
                    }
                }
            }
        }
    }

    @OnClose
    public void onClose(Session session) throws IOException {
        JSONObject jsonObject = new JSONObject();
        jsonObject.put("type", "close");
        jsonObject.put("content", "Connection closed.");
        jsonObject.put("sender", "server");
        session.getBasicRemote().sendText(jsonObject.toString());
        stateSessionShort.remove(session.getId());
        stateSessionLong.remove(session.getId());
        chatEndpoints.remove(this);
    }

    @OnError
    public void onError(Session session, Throwable throwable) {
        try {
            JSONObject jsonObject = new JSONObject();
            jsonObject.put("type", "error");
            jsonObject.put("content", throwable.toString());
            jsonObject.put("sender", "server");
            session.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}