#include "syscall.h"


int printf(const char *fmt, ...);
void *  memset(void *dst, int c, size_t len);
#define item_num 10	//缓冲区的数目

int in ;		//where the producer put the item
int out;		//where the consumer take the item
int items[item_num];
sem_t empty;		//if the items is full, 'empty' blocks producer
sem_t full;		//if the items is empty, 'full' blocks producer
sem_t mutex;

void print()
{
	int i;
	for(i = 0; i < item_num; i ++)
		printf("%d ", items[i]);
	printf("\n");
}

void producer()
{
	while(1)
	{
		sem_wait(&empty);
		sem_wait(&mutex);
		in = in % item_num;
		printf("producer:\n", in);
		items[in] = 1;
		print();
		in++;
		sem_post(&mutex);
		sem_post(&full);
	}
}

void consumer()
{
	while(1)
	{
		sem_wait(&full);
		sem_wait(&mutex);
		out = out % item_num;
		printf("consumer:\n", out);
		items[out] -= 1;
		print();
		out++;
		sem_post(&mutex);
		sem_post(&empty);
	}
}

void pc_main()
{
	in = 0;
	out = 0;
	memset(items, 0, sizeof(items));
	sem_init(&mutex, 1, 1);
	sem_init(&empty, item_num, 0);
	sem_init(&full, 0, 0);
	printf("sem_init success\n");
	create_thread((uint32_t *)producer);
	create_thread((uint32_t *)consumer);
	while(1);
}
