all: histclient histserver

histclient: histclient.c
	gcc -Wall -o histclient histclient.c -lrt

histserver: histserver.c
	gcc -Wall -o histserver histserver.c -lrt

clean:
	rm -fr *~ histclient histserver