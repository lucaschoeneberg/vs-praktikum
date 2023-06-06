let ws = new WebSocket("ws://" + location.host + "/billboard");
let session;
let keepAliveId;

function $(id) {
    return document.getElementById(id);
}

function startWebSocket() {
    ws.onopen = function () {
        console.log("Connected to the WebSocket server");

        // Set up a periodic ping/keep-alive message every 30 seconds
        keepAliveId = setInterval(function () {
            const pingData = JSON.stringify({
                type: 'ping',
                content: {},
                sender: 'client'
            });
            if (ws.readyState === WebSocket.OPEN) {
                ws.send(pingData);
            }
        }, 30000);  // 30000 milliseconds = 30 seconds
    };


    ws.onmessage = function (evt) {
        let msg = JSON.parse(evt.data);
        console.log(msg);

        if (!msg.hasOwnProperty("type") || !msg.hasOwnProperty("content") || !msg.hasOwnProperty("sender")) {
            console.log("Invalid message received: missing required fields.");
            return;
        }

        // Verarbeiten der verschiedenen Nachrichtentypen
        switch (msg.type) {
            case "set":
                console.log(msg.content);
                if (!msg.content[0].hasOwnProperty("id") || !msg.content[0].hasOwnProperty("message")) {
                    console.log("Invalid 'set' message received: missing required fields.");
                    return;
                }

                formatResponse(msg.content);
                break;
            case "get":
                if (!msg.content[0].hasOwnProperty("id") || !msg.content[0].hasOwnProperty("message")) {
                    console.log("Invalid 'get' message received: missing required fields.");
                    return;
                }
                formatResponse(msg.content);
                break;
            case "update":
                if (!msg.content.hasOwnProperty("id") || !msg.content.hasOwnProperty("message")) {
                    console.log("Invalid 'update' message received: missing required fields.");
                    return;
                }
                formatResponse(msg.content);
                break;
            case "delete":
                if (!msg.content.hasOwnProperty("id")) {
                    console.log("Invalid 'delete' message received: missing required fields.");
                    return;
                }
                formatResponse(msg.content);
                break;
            case "deleteAll":
                formatResponse(msg.content);
                break;
            case "connection":
                if (!msg.content.hasOwnProperty("session")) {
                    console.log("Invalid 'connection' message received: missing required fields.");
                    return;
                }
                session = msg.content.session;
                break;
            default:
                console.log("Unknown message type: " + msg.type);
        }

        // Senden einer Bestätigungsnachricht (Ack)
        const ackData = JSON.stringify({
            type: 'ack',
            content: {
                id: msg.id
            },
            sender: 'client'
        });
        if (ws.readyState === WebSocket.OPEN) {
            ws.send(ackData);
        } else {
            console.log("WebSocket not open");
        }
    };

    ws.onerror = function (err) {
        console.log("Error: ", err);
    };

    ws.onclose = function () {
        console.log("Disconnected from the WebSocket server");

        // Clear the keep-alive interval if the WebSocket is closed
        clearInterval(keepAliveId);
    };
}

function setWebSocket(url) {
    console.log("setWebSocket");
    const messageContent = $('contents').value;
    const data = JSON.stringify({
        type: 'set',
        content: {
            message: messageContent
        },
        sender: 'client'
    });
    console.log(data);
    if (ws.readyState === WebSocket.OPEN) {
        ws.send(data);
    } else {
        console.log("WebSocket not open");
    }
}

function putWebSocket(url, id) {
    console.log("putWebSocket");
    const messageContent = $('input_field_' + id).value;
    const data = JSON.stringify({
        type: 'update',
        content: {
            id: id,
            message: messageContent
        },
        sender: 'client'
    });
    if (ws.readyState === WebSocket.OPEN) {
        ws.send(data);
    } else {
        console.log("WebSocket not open");
    }
}

function deleteWebSocket(url, id) {
    console.log("deleteWebSocket");
    const data = JSON.stringify({
        type: 'delete',
        content: {
            id: id
        },
        sender: 'client'
    });
    if (ws.readyState === WebSocket.OPEN) {
        ws.send(data);
    } else {
        console.log("WebSocket not open");
    }
}

function formatResponse(response) {
    for (let i = 0; i < response.length; i++) {
        const disabled = !(session === response[i].sessionId);
        const input = $('input_field_' + response[i].id);
        input.value = response[i].message;
        input.disabled = disabled;
        $('entry' + response[i].id + '-put').disabled = disabled;
        $('entry' + response[i].id + '-delete').disabled = disabled;
    }
}

// Starten Sie den WebSocket, wenn das Dokument geladen wird
window.onload = function () {
    startWebSocket();
    // Senden einer Anfrage nach allen Nachrichten
    const data = JSON.stringify({
        type: 'get',
        content: {},
        sender: 'client'
    });
    if (ws.readyState === WebSocket.OPEN) {
        ws.send(data);
    } else {
        console.log("WebSocket not open");
    }
};
