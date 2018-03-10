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
}struct Job new_job;

struct Job quit_job = { -1, 0};

struct Job queue[15];

sem_t lock;
sem_t read;
sem_t write;
int start = 0;
int end = 0;
int flag = 1;

int main(int argc, char ** argv)
{
	srand(time(0));
	pthread_t producers[atoi(argv[1])];
	pthread_t consumers[atoi(argv[2])];
	sem_init(&lock, 0, 1);
	sem_init(&read, 0, 1);
	sem_init(&write, 0, 1);
	int i;

}
