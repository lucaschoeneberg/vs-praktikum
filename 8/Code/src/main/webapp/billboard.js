globGetMethod = 1; /* 0: html; 1: xyz */

function setGetMethod(val) {
    globGetMethod = val;
}

function $(id) {
    return document.getElementById(id);
}

function getXMLHttpRequest() {
    // XMLHttpRequest for Firefox, Opera, Safari
    if (window.XMLHttpRequest) {
        return new XMLHttpRequest();
    }
    if (window.ActveObject) { // Internet Explorer
        try { // for IE new
            return new ActiveXObject("Msxml2.XMLHTTP");
        } catch (e) {  // for IE old
            try {
                return new ActiveXObject("Microsoft.XMLHTTP");
            } catch (e) {
                alert("Your browser does not support AJAX!");
                return null;
            }
        }
    }
    return null;
}

function getHttpRequest(url) {
    if (globGetMethod === 0)
        getHtmlHttpRequest(url);
    else
        /* xyz = JSON oder XML .... */
        getxyzHttpRequest(url);
}

function getHtmlHttpRequest(url) {
    var xmlhttp = getXMLHttpRequest();
    xmlhttp.open("GET", url, true);
    xmlhttp.onreadystatechange = function () {
        if (xmlhttp.readyState !== 4) {
            $('posters').innerHTML = 'Seite wird geladen ...';
        }
        if (xmlhttp.readyState === 4 && xmlhttp.status === 200) {
            $('posters').innerHTML = xmlhttp.responseText;
        }
        $('timestamp').innerHTML = new Date().toString();
    };
    xmlhttp.setRequestHeader("Accept", "text/html");
    xmlhttp.setRequestHeader("Accept-Encoding", "gzip");
    xmlhttp.send(null);
}

function getxyzHttpRequest(url) {
    const xmlhttp = new XMLHttpRequest();
    xmlhttp.open("GET", url, true);
    xmlhttp.onreadystatechange = function () {
        if (xmlhttp.readyState === 4 && xmlhttp.status === 200) {
            // Die Antwort wird als JSON verarbeitet
            const response = JSON.parse(xmlhttp.responseText);
            // Das Element 'posters' wird mit der formatierten Antwort aktualisiert
            document.getElementById('posters').innerHTML = formatResponse(response);
        }
        document.getElementById('timestamp').innerHTML = new Date().toString();
    };
    xmlhttp.setRequestHeader("Accept", "application/json");
    xmlhttp.setRequestHeader("Accept-Encoding", "gzip");
    xmlhttp.send(null);
}

function formatResponse(response) {
    // Erstellt eine Tabelle aus dem JSON-Array
    let html = '<table border="1" rules="none" cellspacing="4" cellpadding="5">\n';
    for (let i = 0; i < response.entries.length; i++) {
        html += '<tr>\n';
        html += '<td>' + response.entries[i].id + '</td>\n';
        html += '<td>' + response.entries[i].text + '</td>\n';
        html += '</tr>\n';
    }
    html += '</table>\n';
    return html;
}

function postHttpRequest(url) {
    const xmlhttp = getXMLHttpRequest();
    xmlhttp.open("POST", url, true);
    xmlhttp.setRequestHeader("Content-Type", "text/plain;charset=UTF-8");
    const data = $('contents').value;
    xmlhttp.send(data);
    getHtmlHttpRequest('BillBoardServer');
}

function putHttpRequest(url, id) {
    const xmlhttp = getXMLHttpRequest();
    xmlhttp.open("PUT", url + '/' + id, true);
    xmlhttp.setRequestHeader("Content-Type", "text/plain;charset=UTF-8");
    const data = $('input_field_' + id).value;
    xmlhttp.send(data);
    getHtmlHttpRequest('BillBoardServer');
}

function deleteHttpRequest(url, id) {
    const xmlhttp = getXMLHttpRequest();
    xmlhttp.open("DELETE", url + '/' + id, true);
    xmlhttp.send(null);
    getHtmlHttpRequest('BillBoardServer');
}