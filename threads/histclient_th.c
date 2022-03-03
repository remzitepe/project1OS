     
#include <stdlib.h>
#include <mqueue.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "shareddefs.h"
int main( int argc, char* argv[])
{
    mqd_t mq;
    struct item item;
	int n;

	mq = mq_open(MQNAME, O_RDWR);
   
    char *p;
    int intervalCount = strtol(argv[1], &p, 10);
    int intervalWidth = strtol(argv[2], &p, 10);
    int intervalStart = strtol(argv[3], &p, 10);

     if (mq == -1) {
		perror("can not open msg queue\n");
		exit(1);
	}
	printf("mq opened, mq id = %d\n", (int) mq);

		item.intCount = intervalCount;
        item.intWidth = intervalWidth;
        item.intStart = intervalStart;
        item.fileNum = 0;
        for( int i = 0; i< intervalCount; i++)
            item.histogram_data[i] = 0;
		n = mq_send(mq, (char *) &item, sizeof(struct item), 0);

		if (n == -1) {
			perror("mq_send failed\n");
			exit(1);
		}

		printf("mq_send success, item size = %d\n",
		       (int) sizeof(struct item));

		//printf("item->id  = %d\n", item.id);
		printf("item->intCount  = %d\n", item.intCount);
		printf("item->intWidth = %d\n", item.intWidth);
        printf("item->intStart = %d\n", item.intStart);
		printf("\n");
    mqd_t mq2;
	struct mq_attr mq_attr2;
	struct item *itemptr2;
	
	int n2;
	char *bufptr2;
	int buflen2;
	mq2 = mq_open(MQNAME, O_RDWR | O_CREAT, 0666, NULL);
	if (mq2 == -1) {
		perror("can not create msg queue\n");
		exit(1);
	}
	printf("mq2 created, mq2 id = %d\n", (int) mq2);

	mq_getattr(mq2, &mq_attr2);
	printf("mq maximum msgsize = %d\n", (int) mq_attr2.mq_msgsize);

	/* allocate large enough space for the buffer to store 
        an incoming message */
        buflen2 = mq_attr2.mq_msgsize;
		bufptr2 = (char *) malloc(buflen2);

        while(1){
		n2 = mq_receive(mq2, (char *) bufptr2, buflen2, NULL);
		if (n2 == -1) {
			perror("mq_receive failed\n");
			exit(1);
		}

		printf("mq_receive success, message size = %d\n", n2);

		itemptr2 = (struct item *) bufptr2;
        printf("%dfile", itemptr2->fileNum);
        printf("%dcount", itemptr2->intCount);
        
        int countInt = itemptr2->intCount;
		int widthInt = itemptr2->intWidth;
		int startInt = itemptr2->intStart;
        for( int j = 0; j < countInt; j++)
            printf("%d\n", itemptr2->histogram_data[j]);
		//printf("received item->id = %d\n", itemptr->id);
		printf("received item->intCount = %d\n", itemptr2->intCount);
		printf("received item->intWidth = %d\n", itemptr2->intWidth);
        printf("received item->intStart = %d\n", itemptr2->intStart);
		printf("\n");
        }
	free(bufptr2);
    mq_close(mq);
	mq_close(mq2);
}
