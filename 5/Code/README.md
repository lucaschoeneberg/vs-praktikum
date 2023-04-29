    # VS Praktikum: Pub-Sub-System mit gRPC

Das Client-/Server-System wurde als Makefile-Projekt mit VS Code unter Linux angelegt. Kommentare zur Umsetzung befinden sich in den Source-Dateien. In der Header-Datei pub_sub_config.h können Einstellungen vorgenommen werden. Diese muss in der Regel angepasst werden.

Mit `make` werden die Stubs aus der .proto Datei erzeugt, das System übersetzt und gebunden. 

Mit `make clean` werden die generierten Artefakte gelöscht.

# Starten von Server und Client

Der Pub-Sub-Server wird gestartet mit:

> pub_sub_server

Der Client kann unter Angabe einer Server-Adresse gestartet werden:

> pub_sub_client --target=0.0.0.0:40040

Der Delivery-Server wird gestartet mit:

> pub_sub_receiver

In dieser Version des Systems wird der Receiver automatisch gestartet, wenn 
im Client subscribe eingegeben wird. Bei unsubscribe wird der entsprechende Prozess auch 
wieder beendet. Auf ein manuelles Starten sollte man deshalb verzichten, da es sonst
zu Kollisionen bei der Port-Nutzung kommen kann. 

2022/04/29, hje