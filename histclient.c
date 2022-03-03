     
#include <stdlib.h>
#include <mqueue.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "shareddefs.h"
int main( int argc, char* argv[])
{ 
    mqd_t client_server;
	mqd_t server_client;
    struct item item;
	int n;
	client_server = mq_open(CLIENT_SERVER, O_RDWR); // send
	//server_client = mq_open(SERVER_CLIENT, O_RDWR | O_CREAT, 0666, NULL); // receive

    char *p;
    int intervalCount = strtol(argv[1], &p, 10);
    int intervalWidth = strtol(argv[2], &p, 10);
    int intervalStart = strtol(argv[3], &p, 10);

	item.intCount = intervalCount;
    item.intWidth = intervalWidth;
    item.intStart = intervalStart;
	item.value = 0;
	item.id = 0;


	n = mq_send(client_server, (char *) &item, sizeof(struct item), 0);
	printf("item->intCount  = %d\n", item.intCount);
	printf("item->intWidth = %d\n", item.intWidth);
    printf("item->intStart = %d\n", item.intStart);
	printf("\n");
	
	mq_close(server_client);
	mq_close(client_server);
	return 0;
}
