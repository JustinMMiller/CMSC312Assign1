#include<stdio.h>
#include<time.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<signal.h>
#include<semaphore.h>
#include<pthread.h>
#include<limits.h>

//SJF VERSION

typedef struct Jobs 
{
	int size;
	int submitter;
	time_t submitted;
}Job;

typedef struct Vec
{
	int proccount;
	int waittime;
}V;

Job new_job;

Job dummy_job = { INT_MAX, -2};

Job queue[15];

sem_t lock_sem;
sem_t read_sem;
sem_t write_sem;
int start = 0;
int qsize = 0;
int stop = 0;

void init_queue()
{
	int i;
	for(i = 0; i < 15; i++)
	{
		queue[i].size = dummy_job.size;
		queue[i].submitter = dummy_job.submitter;
		printf("i %i size %i submitter %i\n", i, queue[i].size, queue[i].submitter);
	}
}

void insert_into_queue(int jsize, int submit)
{
	int i;
	for(i = 0; i < 15; i++)
	{
		if(queue[i].submitter == -2)
		{
			printf("Found empty slot for %i job size %i\n", submit, jsize);
			queue[i].submitter = submit;
			queue[i].size = jsize;
			queue[i].submitted = time(NULL);
			qsize++;
			printf("i %i size %i submitter %i\n", i, queue[i].size, queue[i].submitter);
			return;
		}
	}
}

Job remove_from_queue(int id)
{
	Job ret;
	int i, ind = 0;
	for(i = 0; i < 15; i++)
	{
		if(queue[i].size < queue[ind].size) ind = i;
	}
	printf("Consumer %i removing a job submitted by Consumer %i size : %i\n", id, queue[ind].submitter, queue[ind].size);
	ret.size = queue[ind].size;
	ret.submitter = queue[ind].submitter;
	ret.submitted = queue[ind].submitted;
	int joblen = queue[ind].size;
	if(queue[ind].submitter != -1)
	{
		queue[ind].size = INT_MAX;
		queue[ind].submitter = -2;
		qsize--;
	}
	printf("qsize after removing job : %i\n", qsize);
	return ret;
}


void signal_handler(int signal)
{
	if(signal == SIGINT)
	{
		printf("Shutting down...\n");
		stop = 1;
		sem_post(&read_sem);
		sem_post(&write_sem);
		sem_post(&lock_sem);
	}
}

void addJob(int jobsize, int id)
{	
	printf("Producer %i wants to add a job\n", id);
	sem_wait(&write_sem);
	sem_wait(&lock_sem);
	if(!stop)
	{
		insert_into_queue(jobsize, id);
	}
	sem_post(&read_sem);
	sem_post(&lock_sem);
}

Job printJob(int id)
{
	Job ret;
	printf("Consumer %i is waiting to read\n", id);
	sem_wait(&read_sem);
	sem_wait(&lock_sem);
	ret = remove_from_queue(id);
	sem_post(&lock_sem);
	sem_post(&write_sem);
	return ret;
}

V consumer_func(int num)
{
	V ret;
	ret.proccount = 0;
	ret.waittime = 0;
	int flag = 1;
	Job process;
	while(flag == 1 && !stop)
	{
		process = printJob(num);
		if(process.size == 2000)
		{
			flag = 0;
		}
		else{
			ret.waittime += (int)difftime(time(NULL),process.submitted);
			ret.proccount++;
			sleep(((process.size % 100)/10)+1);
		}
		if(stop)return;
	}
	sem_post(&read_sem);
	printf("Consumer %i is DYING\n\n", num);
	return ret;
}
//https://stackoverflow.com/questions/7797664/what-is-the-most-correct-way-to-generate-random-numbers-in-c-with-pthread
//STACK OVERFLOW
size_t random_range(int low, int high)
{
	unsigned short state[3];
	unsigned int seed = time(NULL) + (unsigned int)pthread_self();
	memcpy(state, &seed, sizeof(seed));
	return low + nrand48(state) % (high - low);
}

void producer_func(int num)
{
	int i = 0;
	int althor = random_range(1, 20);
	for(; i < althor && !stop; i++)
	{
		if(stop)return;
		addJob(random_range(100, 1000), num);
	}
}

int main(int argc, char ** argv)
{
	init_queue();
	signal(SIGINT, signal_handler);
	srand(time(0));
	int j = atoi(argv[1]);
	int k = atoi(argv[2]);
	pthread_t producers[j];
	pthread_t consumers[k];
	sem_init(&lock_sem, 0, 1);
	sem_init(&read_sem, 0, 0);
	sem_init(&write_sem, 0, 15);
	int i;
	for(i = 0; i < k; i++)
	{
		pthread_create(&consumers[i], NULL, consumer_func, (void*)(i+1));
	}
	for(i = 0; i < j; i++)
	{
		pthread_create(&producers[i], NULL, producer_func, (void*)(i+1));
	}
	for(i = 0; i < j; i++)
	{
		pthread_join(producers[i], NULL);
	}
	addJob(2000, -1);
	V vector;
	int totalwait=0;
	int totaljobs=0;
	for(i = 0; i < k; i++)
	{
		pthread_join(consumers[i], &vector);
		totalwait += vector.waittime;
		totaljobs += vector.proccount;
	}
	printf("%i %i", totalwait, totaljobs);
	printf("\nAverage wait : %d", totalwait/totaljobs);
	return 0;
}
