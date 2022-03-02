     
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
	
		
	
	mq_close(mq);

	return 0;
}
