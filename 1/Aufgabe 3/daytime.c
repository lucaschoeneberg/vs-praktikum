/* 
 * File:   daytime.c
 * Author: Heinz-Josef Eikerling
 *
 *         Implementierung des Daytime-Service.
 *		   Compile: gcc -o daytime daytime.c
 *		   Aufruf: daytime oder daytime <host address> <host port>
 *
 * Created on 10. März 2012, 21:54
 */

#include <stdio.h>  
#include <unistd.h>  
#include <stdlib.h>  
#include <errno.h>  
#include <string.h>  
#include <time.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <netdb.h>  

/* 
 * Fehler ausgeben und Programm beenden. 
 */  
static void handleError(const char *cause) {  
	if ( errno != 0 ) {  
		fputs(strerror(errno), stderr);  
		fputs(": ", stderr);  
	}  
	fputs(cause,stderr);  
	fputc('\n',stderr);  
	exit(1);  
}  

/* 
 * main-Funktion
 */
int main(int argc,char **argv) {  
	int z;  
	char *srvr_addr = NULL;  
	char *srvr_port = "9013";  
	struct sockaddr_in adr_srvr; // AF_INET  
	struct sockaddr_in adr_clnt; // AF_INET  
	int addr_len;                // length  
	int s;                       // Socket  
	int c;                       // Client socket  
	int n;                       // bytes  
	time_t td;                   // Current date&time  
	char tstamp[128];            // Date/Time info  

	/* 
	* Server-Adresse von der Kommandozeile lesen,
	* falls vorhanden.
	*/  
	if ( argc >= 2 ) {  
		srvr_addr = argv[1]; // Addr on cmdline  
	}  
	else {  
		srvr_addr = "127.0.0.1";  // Use default address  
	}  

	/* 
	* Server-Port von der Kommandozeile lesen,
	* falls vorhanden.
	*/ 
	if ( argc >= 3 ) {  
		srvr_port = argv[2];  
	}  

	/* 
	* Socket anlegen. 
	*/  
	s = socket(PF_INET,SOCK_STREAM,0);  
	if ( s == -1 ) {  
		handleError("socket()");  
	}  
	memset(&adr_srvr,0,sizeof adr_srvr);  
	adr_srvr.sin_family = AF_INET;  
	adr_srvr.sin_port = htons(atoi(srvr_port));  
	if ( strcmp(srvr_addr,"*") != 0 ) {  
		adr_srvr.sin_addr.s_addr = inet_addr(srvr_addr); // Normal Address  
		if ( adr_srvr.sin_addr.s_addr == INADDR_NONE ) {  
			handleError("bad address.");  
		}  
	}  
	else {  
		adr_srvr.sin_addr.s_addr = INADDR_ANY; // Wild Address  
	}  

	/* Adresse an Server binden. */  
	addr_len = sizeof adr_srvr;  
	z = bind(s,(struct sockaddr *)&adr_srvr, addr_len);
    if ( z == -1 ) {
		handleError("bind(2)");  
	}  

	z = listen(s,10);  
	if ( z == -1 ) {  
		handleError("listen(2)");  
	}  

	for (;;) {  
		/* Server Loop. Auf Anfragen warten und diese auswerten */
		addr_len = sizeof adr_clnt;  
		c = accept(s,  (struct sockaddr *)&adr_clnt,  &addr_len);  

		if ( c == -1 ) {  
			handleError("accept(2)");  
		}  

		/* Zeitstempel erzeugen */
		time(&td);  
		n = (int) strftime(tstamp,sizeof tstamp,  
			"%A %b %d %H:%M:%S %Y\n",  
			localtime(&td));  

		/* Ergebnis an Client geben */
		z = write(c,tstamp ,n);  
		if ( z == -1 ) {  
			handleError("write(2)");  
		}  

		/* Verbindung schlie�en */ 
		close(c);  
	}  
	return 0;  
}  