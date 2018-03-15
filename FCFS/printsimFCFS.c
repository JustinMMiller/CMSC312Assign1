#include<stdio.h>
#include<sys/types.h>
#include<time.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<signal.h>
#include<semaphore.h>
#include<pthread.h>


// FCFS VERSION

typedef struct Jobs
{
	int size;
	int submitter;
	time_t submitted;
} Job;
typedef struct Vec
{
	int proccount;
	int waittime;
}V;


Job queue[15];

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
	Job job;
	sem_wait(&write_sem);
	sem_wait(&lock_sem);
	if(!stop)
	{
		job.size = jobsize;
		job.submitter = id;
		job.submitted = time(NULL);
		queue[(start+qsize)%15] = job;
		qsize++;
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
	printf("Consumer %i processing job size %i qsize %i\n", id, queue[start%15].size, qsize);
	printf("Job Submitter : %i\n", queue[start%15].submitter);
	ret = queue[start%15];
	if(ret.submitter != -1){
		start++;
		qsize--;
	}
	printf("Consumer %i removed a job\nStart index : %i End index %i\n", id, start, start+qsize);
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
		if(process.size == -1)flag = 0;
		else{
			ret.waittime += (int)difftime(time(NULL), process.submitted);
			ret.proccount++;
			sleep(((process.size % 100)/10)+1);
		}
		if(stop)return;
	}
	sem_post(&read_sem);
	sem_post(&write_sem);
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
	int totalwait = 0;
	int totaljobs = 0;
	V vector;
	for(i = 0; i < k; i++)
	{
		pthread_join(consumers[i], &vector);
		totalwait += vector.waittime;
		totaljobs += vector.proccount;
	}
	printf("total wait %i total jobs %i\n", totalwait, totaljobs);
	printf("Average wait : %d\n", totalwait/totaljobs);
	return 0;
}
