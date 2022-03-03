#include <stdlib.h>
#include <mqueue.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "shareddefs.h"

int main( int argc, char* argv[])
{
	// First message queue for receiving inputs from client
   	mqd_t client_server;
	mqd_t server_client;
	mqd_t child_parent;
	client_server = mq_open(CLIENT_SERVER, O_RDWR | O_CREAT, 0666, NULL); //receive
	server_client = mq_open(SERVER_CLIENT, O_RDWR); // send
	child_parent = mq_open(CLIENT_SERVER, O_RDWR | O_CREAT, 0666, NULL);
	
	struct item item;
	struct mq_attr mq_attr;
	struct item *itemptr;
	int n;
	char *bufptr;
	int buflen;
	itemptr = (struct item *) bufptr;

	// Receiving inputs from client
	mq_getattr(client_server, &mq_attr);
	buflen = mq_attr.mq_msgsize;
	bufptr = (char *) malloc(buflen);
	printf("mq1 maximum msgsize = %d\n", (int) mq_attr.mq_msgsize);

	/* allocate large enough space for the buffer to store 
        an incoming message */
        
		mq_receive(client_server, (char *) bufptr, buflen, NULL);
		printf("mq_receive success from client, message size = %d\n", n);
		
        int count = itemptr->intCount;
		int width = itemptr->intWidth;
		int start = itemptr->intStart;

		printf("received item->intCount = %d\n", itemptr->intCount);
		printf("received item->intWidth = %d\n", itemptr->intWidth);
        printf("received item->intStart = %d\n", itemptr->intStart);
		printf("\n");

	free(bufptr);


	// Histogram Calculation 
	int numberOfFiles = atoi(argv[1]);
	printf("Number Of Files: %d\n",numberOfFiles);
	
	// Creating n child processes
	pid_t child_process;
	int file_index = 2;

	
	for(int i = 0; i < numberOfFiles; i++){
		child_process = fork();
		if(child_process < 0){
			printf("Fork failed");
			exit(-1);
		}
		else if(child_process == 0){
			
			// Creating a message queue for child and parent 
			// Child is a producer
		
			//Sending
			int histogram_array[count];
			for(int i = 0; i < count; i++)
			histogram_array[i] = 0;
		    char const* const fileName = argv[file_index]; /* should check that argc > 1 */
			FILE* file = fopen(fileName, "r"); /* should check the result */
			char line[256];

			
			while (fgets(line, sizeof(line), file)) {
				/* note that fgets don't strip the terminating \n, checking its
				presence would allow to handle lines longer that sizeof(line) */
				histogram_array[(atoi(line) - start) / width]++;
			}
			/* may check feof here to make a difference between eof and io failure -- network
			timeout for instance */

    		fclose(file);
			file_index++;
			item.intCount = count;
			item.intWidth = width;
			item.intStart = start;
			item.id = getpid();
			printf("Ben pid yim: %d\n",item.id);
			printf("Count: %d",count);
			for(int i = 0; i < count; i++){
				item.value = histogram_array[i];
				mq_send(child_parent, (char *) &item, sizeof(struct item), 0);
				if (n == -1) {
					perror("mq_send failed in mq2\n");
					exit(1);
				}
			}
			file_index++;
			
			printf("mq_send success in mq2, item size = %d\n",
		       (int) sizeof(struct item));
		}
		else{
			wait(numberOfFiles);
			printf("Child complete\n");
			printf("mq2 maximum msgsize = %d\n", (int) mq_attr.mq_msgsize);
			/* allocate large enough space for the buffer to store 
			an incoming message */
			mq_getattr(child_parent, &mq_attr);
			buflen = mq_attr.mq_msgsize;
			bufptr = (char *) malloc(buflen);
			printf("Break1\n");
			mq_receive(child_parent, (char *) bufptr, buflen, NULL);
			printf("Break2\n");
			
			int file_id;
			pid_t id;
			for(int j = 0; j < count * numberOfFiles; j++){
				start = itemptr->intStart;
				count = itemptr->intCount;
				width = itemptr->intWidth;
				id = itemptr->id;
				printf("[%d,%d): %d\n",start + (j*width),(start + ((j +1)*width)),itemptr->value);
			}
			printf("mq_receive success for mq2, message size = %d\n", n);			
			printf("received item for mq2->intCount = %d\n", itemptr->intCount);
			printf("Child id: %d\n", itemptr->id);
			printf("Histogram Data:\n");
			free(bufptr);	
			exit(0);
		}
	}
	item.intCount = count;
	item.intStart = start;
	item.intWidth = width;
	n = mq_send(server_client, (char *) &item, sizeof(struct item), 0);
	mq_close(client_server);
	mq_close(server_client);
	mq_close(child_parent);
	return 0;
}
