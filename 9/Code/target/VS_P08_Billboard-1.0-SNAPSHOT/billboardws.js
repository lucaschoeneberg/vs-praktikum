let ws;
let session;

function $(id) {
    return document.getElementById(id);
}

function startWebSocket() {
    ws = new WebSocket("ws://" + location.host + "/VS_P08_Billboard_war_exploded/billboard");

    ws.onopen = function () {
        console.log("Connected to the WebSocket server");
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
                if (!msg.content.hasOwnProperty("message") || !msg.content.hasOwnProperty("id")) {
                    console.log("Invalid 'set' message received: missing required fields.");
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
        ws.send(ackData);
    };

    ws.onerror = function (err) {
        console.log("Error: ", err);
    };
}

function setWebSocket(url) {
    const messageContent = $('contents').value;
    const data = JSON.stringify({
        type: 'set',
        content: {
            message: messageContent
        },
        sender: 'client'
    });
    ws.send(data);
    setTimeout(function () {
        getHtmlHttpRequest('BillBoardServer');
    }, 100);
}

function putWebSocket(url, id) {
    const messageContent = $('input_field_' + id).value;
    const data = JSON.stringify({
        type: 'update',
        content: {
            id: id,
            message: messageContent
        },
        sender: 'client'
    });
    ws.send(data);
    setTimeout(function () {
        getHtmlHttpRequest('BillBoardServer');
    }, 100);
}

function deleteWebSocket(url, id) {
    const data = JSON.stringify({
        type: 'delete',
        content: {
            id: id
        },
        sender: 'client'
    });
    ws.send(data);
    setTimeout(function () {
        getHtmlHttpRequest('BillBoardServer');
    }, 100);
}

function formatResponse(response) {
    for (let i = 0; i < response.length; i++) {
        const disabled = !(session === response[i].sessionId);
        const input = $('input_field_' + response[i].id);
        input.value = response[i].text;
        input.disabled = disabled;
        $('entry' + response[i].id + '-put').disabled = disabled;
        $('entry' + response[i].id + '-delete').disabled = disabled;
    }
}

// Starten Sie den WebSocket, wenn das Dokument geladen wird
window.onload = function () {
    startWebSocket();
};
