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
};
struct Job new_job;

struct Job quit_job = { -1, -1};

struct Job queue[15];

sem_t lock_sem;
sem_t read_sem;
sem_t write_sem;
int start = 0;
int end = 0;
int stop = 0;
int size = 0;

void signal_handler(int signal)
{
	if(signal == SIGINT)
	{
		printf("Shutting down...\n");
		stop = 1;
	}
}

void addJob(int jobsize, int id)
{
	sem_wait(&write_sem);
	sem_wait(&lock_sem);
	struct Job job = new_job;
	job.size = jobsize;
	job.submitter = id;
	queue[end] = job;
	end++;
	end %= 15;
	size++;
	if(size < 15)sem_post(&write_sem);
	sem_post(&lock_sem);
	sem_post(&read_sem);
}

int printJob(int id)
{
	int ret = -1;
	printf("Thread %i is waiting to read\n", num);
	sem_wait(&read_sem);
	sem_wait(&lock_sem);
	printf("queue[start].submitter : %i\n", queue[start].submitter);
	printf("Thread %i processing job size %i\n", id, queue[start].size);
	if(queue[start].submitter != -1)
	{
		ret = queue[start].size;
		start++;
		start %= 15;
		size--;
	}
	if(size > 0) sem_post(&read_sem);
	sem_post(&lock_sem);
	sem_post(&write_sem);
	return ret;
}

void consumer_func(int num)
{
	int flag = 1;
	if(stop)return;
	while(flag == 1&& !stop)
	{
		int var = printJob(num);
		printf("Consumer %i just processed a job\n", num);
		if(var == -1)flag = 0;
		usleep(var*1000);
	}
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
		addJob(random_range(100, 1000), num);
		printf("Producer %i added a job\n", num);
		usleep(random_range(100, 1000)*1000);
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
	sem_init(&read_sem, 0, 1);
	sem_init(&write_sem, 0, 1);
	int i;
	for(i = 0; i < k; i++)
	{
		pthread_create(&consumers[i], NULL, consumer_func, (void*)(i+1));
	}
	for(i = 0; i < j; i++)
	{
		pthread_create(&producers[i], NULL, producer_func, (void*)(i+1));
	}
	for(i = 0; i < k; i++)
	{
		pthread_join(producers[i], NULL);
	}
	addJob(-1, -1);
	for(i = 0; i < j; i++)
	{
		pthread_join(consumers[i], NULL);
	}
	return 0;
}
