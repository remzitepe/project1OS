#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <mqueue.h>
#include <unistd.h>
#include <errno.h>

#include "shareddefs.h"

#define MAXTHREADS  10		/* max number of threads */
#define MAXFILENAME 50		/* max length of a filename */


/* 
   thread function will take a pointer to this structure
*/
struct arg {
    int count;
    int width;
    int start;
    char* file;
    int hist_arr[1000];
    int fileNumber;			
	int file_index;	
};


/* this is function to be executed by the threads */
static void *do_task(void *arg_ptr)
{   
	char *retreason; 
    int hist_arr[((struct arg *) arg_ptr)->count];
    for( int i = 0; i < ((struct arg *) arg_ptr)->count; i++)
        hist_arr[i] = 0;
	printf("thread %d started\n", ((struct arg *) arg_ptr)->file_index);

	char const* const fileName = ((struct arg *) arg_ptr)->file; /* should check that argc > 1 */
    FILE* file = fopen(fileName, "r"); /* should check the result */
    char line[256];

    while (fgets(line, sizeof(line), file)) {
        /* note that fgets don't strip the terminating \n, checking its
           presence would allow to handle lines longer that sizeof(line) */
		hist_arr[(atoi(line) - ((struct arg *) arg_ptr)->start) / ((struct arg *) arg_ptr)->width]++;
    }
    /* may check feof here to make a difference between eof and io failure -- network
       timeout for instance */
    mqd_t mq2;
    struct item item2;
	int n;

	mq2 = mq_open(MQNAME, O_RDWR);
   
     if (mq2 == -1) {
		perror("can not open msg queue\n");
		exit(1);
	}
	printf("mq2 opened, mq2 id = %d\n", (int) mq2);

		item2.intCount = ((struct arg *) arg_ptr)->count;
        item2.intWidth = ((struct arg *) arg_ptr)->width;
        item2.intStart = ((struct arg *) arg_ptr)->start;
        item2.fileNum = ((struct arg *) arg_ptr)->fileNumber;
        for( int i = 0; i <((struct arg *) arg_ptr)->count; i++)
            item2.histogram_data[i] = hist_arr[i];
		n = mq_send(mq2, (char *) &item2, sizeof(struct item), 0);

		if (n == -1) {
			perror("mq_send failed\n");
			exit(1);
		}

		printf("mq_send success, item size = %d\n",
		       (int) sizeof(struct item));

		//printf("item->id  = %d\n", item.id);
		printf("item->intCount  = %d\n", item2.intCount);
		printf("item->intWidth = %d\n", item2.intWidth);
        printf("item->intStart = %d\n", item2.intStart);
		printf("\n");
	
	mq_close(mq2);

    fclose(file);
	retreason = malloc (200); 
	strcpy (retreason, "normal termination of thread"); 
	pthread_exit(retreason);  // just tell a reason to the thread that is waiting in join
}

int main(int argc, char *argv[])
{
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

        int countInt = itemptr->intCount;
		int widthInt = itemptr->intWidth;
		int startInt = itemptr->intStart;
		//printf("received item->id = %d\n", itemptr->id);
		printf("received item->intCount = %d\n", itemptr->intCount);
		printf("received item->intWidth = %d\n", itemptr->intWidth);
        printf("received item->intStart = %d\n", itemptr->intStart);
		printf("\n");

	pthread_t tids[MAXTHREADS];	/*thread ids*/
	struct arg t_args[MAXTHREADS];	/*thread function arguments*/
	
	int i;
	int ret;
    int numOfFiles;
	char *retmsg; 

	numOfFiles = atoi(argv[1]);	/* number of threads (files) to create */
	for (i = 0; i < numOfFiles; ++i) {

        int* hist_arr;
        t_args[i].file_index = i;
        t_args[i].file = argv[i+2];
        t_args[i].count = countInt;
        t_args[i].start = startInt;
        t_args[i].width = widthInt;
        t_args[i].fileNumber = numOfFiles;
        for( int j = 0; j < 1000; j++)
            t_args[i].hist_arr[j] = 0;
		ret = pthread_create(&(tids[i]),
				     NULL, do_task, (void *) &(t_args[i]));

		if (ret != 0) {
			printf("thread create failed \n");
			exit(1);
		}
		printf("thread %i with tid %u created\n", i,
		       (unsigned int) tids[i]);
	}
        for( int j = 0; j < countInt; j++)
            printf( "%d", t_args[i].hist_arr[j]);

	printf("main: waiting all threads to terminate\n");
	for (i = 0; i < numOfFiles; ++i) {
	    ret = pthread_join(tids[i], (void **)&retmsg);
		if (ret != 0) {
			printf("thread join failed \n");
			exit(1);
		}
		printf ("thread terminated, msg = %s\n", retmsg);
		// we got the reason as the string pointed by retmsg
		// space for that was allocated in thread function; now freeing. 
		free (retmsg); 
	}

	printf("main: all threads terminated\n");
    free(bufptr);
	mq_close(mq);
	return 0;
}