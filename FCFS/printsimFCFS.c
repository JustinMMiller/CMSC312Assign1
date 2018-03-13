#include<stdio.h>
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

struct Job 
{
	int size;
	int submitter;
	time_t submitted;
};
struct Job new_job;

struct Job quit_job = { -1, -1};

struct Job queue[15];

sem_t lock_sem;
sem_t read_sem;
sem_t write_sem;
int start = 0;
int qsize = 0;
int stop = 0;


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
		struct Job job = new_job;
		job.size = jobsize;
		job.submitter = id;
		queue[(start+qsize)%15] = job;
		qsize++;
		printf("Producer %i adding a job\nStart index : %i End index %i\n", id, start, start+qsize);
	}
	sem_post(&read_sem);
	sem_post(&lock_sem);
}

int printJob(int id)
{
	int ret = -1;
	printf("Consumer %i is waiting to read\n", id);
	sem_wait(&read_sem);
	sem_wait(&lock_sem);
	printf("Consumer %i processing job size %i qsize %i\n", id, queue[start%15].size, qsize);
	printf("Job Submitter : %i\n", queue[start%15].submitter);
	if(queue[start%15].size > 0)
	{
		ret = queue[start%15].size;
		start++;
		qsize--;
	}
	printf("Consumer %i removed a job\nStart index : %i End index %i\n", id, start, start+qsize);
	sem_post(&lock_sem);
	sem_post(&write_sem);
	return ret;
}

void consumer_func(int num)
{
	int flag = 1;
	while(flag == 1 && !stop)
	{
		if(stop)return;
		int var = printJob(num);
		if(var == -1)flag = 0;
		else{sleep((var % 5)+1);}
	}
	sem_post(&read_sem);
	sem_post(&write_sem);
	printf("Consumer %i is DYING\n\n", num);
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
		sleep(random_range(1, 3));
	}
}

int main(int argc, char ** argv)
{
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
	addJob(-1, -1);
	for(i = 0; i < k; i++)
	{
		pthread_join(consumers[i], NULL);
	}
	return 0;
}
