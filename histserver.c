#include <stdlib.h>
#include <mqueue.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "shareddefs.h"


//Calculating Histogram
int* calculateHistogram(char* inputFile, int count, int width, int start){
	char const* const fileName = inputFile; /* should check that argc > 1 */
    FILE* file = fopen(fileName, "r"); /* should check the result */
    char line[256];

	int* histogram_array = malloc(sizeof(int) * count);
	for(int i = 0; i < count; i++){
		histogram_array[i] = 0;
	}

    while (fgets(line, sizeof(line), file)) {
        /* note that fgets don't strip the terminating \n, checking its
           presence would allow to handle lines longer that sizeof(line) */
		histogram_array[(atoi(line) - start) / width]++;
    }
    /* may check feof here to make a difference between eof and io failure -- network
       timeout for instance */

    fclose(file);

    return histogram_array;
}

int main( int argc, char* argv[])
{
	// First message queue for receiving inputs from client
   	mqd_t mq;
	struct mq_attr mq_attr;
	struct item *itemptr;
	int n;
	char *bufptr;
	int buflen;

	mq = mq_open(MQNAME, O_RDWR | O_CREAT, 0666, NULL);
	if (mq == -1) {
		perror("can not create msg queue\n");
		exit(1);
	}
	printf("mq created, mq id = %d\n", (int) mq);

	mq_getattr(mq, &mq_attr);
	printf("mq maximum msgsize = %d\n", (int) mq_attr.mq_msgsize);

	/* allocate large enough space for the buffer to store 
        an incoming message */
        buflen = mq_attr.mq_msgsize;
		bufptr = (char *) malloc(buflen);

	
		n = mq_receive(mq, (char *) bufptr, buflen, NULL);
		if (n == -1) {
			perror("mq_receive failed\n");
			exit(1);
		}

		printf("mq_receive success, message size = %d\n", n);

		itemptr = (struct item *) bufptr;

        int count = itemptr->intCount;
		int width = itemptr->intWidth;
		int start = itemptr->intStart;
		//printf("received item->id = %d\n", itemptr->id);
		printf("received item->intCount = %d\n", itemptr->intCount);
		printf("received item->intWidth = %d\n", itemptr->intWidth);
        printf("received item->intStart = %d\n", itemptr->intStart);
		printf("\n");

	free(bufptr);
	mq_close(mq);


	// Histogram Calculation 
	int numberOfFiles = atoi(argv[1]);
	printf("Number Of Files: %d\n",numberOfFiles);
	
	// Creating n child processes
	pid_t child_process[numberOfFiles];
	pid_t parent;
	int file_index = 2;
	mqd_t mq2;
	struct item item2;
	mq2 = mq_open(MQNAME, O_RDWR);
	
	int** histogram_data = malloc(numberOfFiles * sizeof(int*));
	for(int i = 0; i < numberOfFiles; i++){
		histogram_data[i] = malloc(count * sizeof(int));
	}
	for(int i = 0; i < numberOfFiles; i++){
		child_process[i] = fork();
		if(child_process[i] < 0){
			printf("Fork failed");
			exit(-1);
		}
		else if(child_process[i] == 0){
			
			histogram_data[i] = calculateHistogram(argv[i + 2], count, width, start);
			// Creating a message queue for child and parent 
			// Child is a producer
			
			if (mq2 == -1) {
				perror("can not open msg queue\n");
				exit(1);
			}
			printf("mq opened, mq2 id = %d\n", (int) mq2);
			item2.intCount = count;
			item2.intWidth = width;
			item2.intStart = start;
			item2.pid = getpid();
			item2.histogram_data = calculateHistogram(argv[file_index], count, width, start);
			file_index++;
			n = mq_send(mq, (char *) &item2, sizeof(struct item), 0);
			if (n == -1) {
				perror("mq_send failed\n");
				exit(1);
			}
			printf("mq_send success, item size = %d\n",
		       (int) sizeof(struct item));
			printf("Child id: %d\n", item2.pid);
			printf("Histogram Data:\n");
			for(int i = 0; i < count; i++){
				printf("[%d,%d): %d\n",start + (i*width),(start + ((i +1)*width)),item2.histogram_data[i]);
			}
		}
		else{
			wait(NULL);
			printf("Child complete");
			exit(0);
		}
	}

	mq_close(mq);
	return 0;
}
