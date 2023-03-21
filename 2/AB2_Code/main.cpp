#include <iostream>

/*
1. Ihr Programm soll als mehrstufiger Server ausgelegt sein.
2. Der Server wird unter Angabe eines Ports und eines Verzeichnisses, in dem html-
Dokumente gespeichert sind, gestartet:
HTTPserv <docroot> <port>
3. Bekanntlich wird üblicherweise der Port 80 verwendet, dessen Benutzung aber spezielle
Privilegien erfordert. Verwenden Sie deshalb Port 8080 oder 8008.
4. Der Server muss nur die Verarbeitung von GET-Requests unterstützen:
a. Die in dem mit dem Request übermittelten URI angegebene Datei wird in dem Arbeitsverzeichnis gesucht, ausgelesen und an den aufrufenden Client zurückgegeben (Statuscode 200). Kann die in der URI angegebene Datei nicht gefunden werden, wird der Statuscode 404 zurückgegeben.
b. Wird in der URI ein Verzeichnis (Unterverzeichnis im Arbeitsverzeichnis des Servers) referenziert, so wird das Verzeichnis in Form eines html-Dokumentes ausgegeben. Die in dem Verzeichnis vorhandenen html-Dateien und Unterverzeichnisse sind in dem Dokument verlinkt.
5. Sehen Sie die korrekte Behandlung von Dateitypen (z.B. Bilddateien im jpg-Format) vor. Dazu muss der Content-Type im Response-Header angepasst werden und Sie müssen die Dateien binär übertragen.
6. Im Dateibereich der Veranstaltung finden Sie ein Beispiel-Verzeichnis, welches durch Ihren Server abrufbar sein soll.
7. Sehen Sie Testausgaben zwecks Kontrolle der Verarbeitung der Client-Anfragen vor und achten Sie auf eine angemessene Fehlerbehandlung.
8. Testen Sie Ihren Server mit mehr als einem Browser und notieren Sie die Unterschiede bei der Verarbeitung von Anfragen.
*/

int main() {
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
