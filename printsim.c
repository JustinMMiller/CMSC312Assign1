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
	int valid;
	int submitter;
};
struct Job new_job;

struct Job quit_job = { -1, 0, -1};

struct Job queue[15];

sem_t lock_sem;
sem_t read_sem;
sem_t write_sem;
int start = 0;
int end = 0;
int flag = 1;
int size = 0;

void addJob(int jobsize, int id)
{
	sem_wait(&write_sem);
	sem_wait(&lock_sem);
	struct Job job = new_job;
	job.size = jobsize;
	job.valid = 1;
	job.submitter = id;
	queue[end] = job;
	end++;
	end %= 15;
	size++;
	if(size < 15)sem_post(&write_sem);
	sem_post(&lock_sem);
	sem_post(&read_sem);
}

void printJob()
{
	sem_wait(&read_sem);
	sem_wait(&lock_sem);
	usleep(queue[start].size);
	start++;
	start %= 15;
	size--;
	if(size > 0) sem_post(&read_sem);
	sem_post(&lock_sem);
	sem_post(&write_sem);
	
}

void consumer_func(int num)
{

}

void producer_func(int num)
{

}

int main(int argc, char ** argv)
{
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
		pthread_create(&consumers[i], NULL, consumer_func, (void*)i);
	}
	return 0;
}
