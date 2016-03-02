#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdint.h>
#include <time.h>

sem_t rw_mutex; /*used for access to the shared variable*/
sem_t mutex; /*used for access to the read_count*/
int read_count=0; /*how many threads are currently reading the object*/
int shared_var=0;
int max_access_attempts=0;
double reader_wait_times[100]; /*in milliseconds*/
double writer_wait_times[10]; /*in milliseconds*/

void * writer(void *arg){
	int i;
	int writer_id=(intptr_t) arg;
	struct timeval start,end;
	gettimeofday(&start, NULL);
	for(i=0;i<max_access_attempts;i++){		
		printf("Writer %d is waiting for access\n",writer_id);
		sem_wait(&rw_mutex);
		printf("Writer %d is writing, incrementing shared_var from %d",writer_id, shared_var);
		shared_var+=10;
		printf(" to %d\n", shared_var);
		sem_post(&rw_mutex);
		int random_sleep_time= rand()%100000;
		double random_sleep_time_mseconds= ((double)random_sleep_time)/1000;
		printf("Writer %d is going to sleep for %f milliseconds\n", writer_id, random_sleep_time_mseconds);
		usleep(random_sleep_time);	
	}
	gettimeofday(&end, NULL);
	writer_wait_times[writer_id]=((double)(end.tv_sec*1000000+end.tv_usec)-(start.tv_sec*1000000+start.tv_usec))/1000; 
}

void * reader(void *arg){
	int i;
	int reader_id=(intptr_t) arg;
	struct timeval start,end;
	gettimeofday(&start, NULL);
	for(i=0;i<max_access_attempts;i++){
		printf("Reader %d wants to read!\n",reader_id);
		sem_wait(&mutex);
		read_count++;
		if(read_count==1){
			sem_wait(&rw_mutex); 
		} 
		sem_post(&mutex);
		printf("Reader %d is reading, it sees shared_var is %d\n",reader_id, shared_var);
		sem_wait(&mutex);
		read_count--;
		if(read_count==0){
			sem_post(&rw_mutex);
		}
		sem_post(&mutex);
		/* RAND_MAX = 2147483647 */
		int random_sleep_time= rand()%100000;
		double random_sleep_time_mseconds= ((double)random_sleep_time)/1000;
		printf("Reader %d is going to sleep for %f milliseconds\n", reader_id, random_sleep_time_mseconds);
		usleep(random_sleep_time);
	}
	gettimeofday(&end, NULL);
	reader_wait_times[reader_id]=((double)(end.tv_sec*1000000+end.tv_usec)-(start.tv_sec*1000000+start.tv_usec))/1000; 
}

int compare(const void *arg1, const void *arg2){
	double temp_arg1=*(const double*)arg1; /*use const here because we have no intention of modifying these values, just comparing*/
	double temp_arg2=*(const double*)arg2;
	return (temp_arg1>temp_arg2)-(temp_arg1<temp_arg2); /*returns 1 if arg1 greater, 0 if equal and -1 if arg2 greater*/
}

double average(double array[], int size){
	int i;
	double sum=0;
	for(i=0;i<size;i++){
		sum+=array[i];
	}
	return sum/size;
}

int main(int argc, char *argv[]){

	int i, num_reader_threads, num_writer_threads;
	num_reader_threads=100;
	num_writer_threads=10;
	pthread_t readers[num_reader_threads], writers[num_writer_threads];
	printf("Enter the maximum number of shared variable access attempts:\n");
	scanf("%d",&max_access_attempts);
	printf("You have selected %d access attempts\n",max_access_attempts);
	sem_init(&rw_mutex, 0,1);
	sem_init(&mutex, 0,1);
	srand(time(NULL));
	for(i=0;i<num_reader_threads;i++){
		pthread_create(&readers[i],NULL, reader, (void *) (intptr_t) i);
	}
	for(i=0;i<num_writer_threads;i++){
		pthread_create(&writers[i],NULL, writer, (void *) (intptr_t) i);
	}
	for(i=0;i<num_writer_threads;i++){
		pthread_join(writers[i],NULL);
	}
	for(i=0;i<num_reader_threads;i++){
		pthread_join(readers[i],NULL);
	}
	printf("Shared variable: %d \n", shared_var);
	qsort(writer_wait_times,num_writer_threads,sizeof(double),compare);
	qsort(reader_wait_times,num_reader_threads,sizeof(double),compare);
	printf("Reader wait times: MIN %f MAX %f AVG %f\n", reader_wait_times[0], reader_wait_times[num_reader_threads-1],average(reader_wait_times, num_reader_threads));
	printf("Writer wait times: MIN %f MAX %f AVG %f\n", writer_wait_times[0], writer_wait_times[num_writer_threads-1],average(writer_wait_times, num_writer_threads));
	sem_destroy(&mutex);
	sem_destroy(&rw_mutex);
	return 0;
}
