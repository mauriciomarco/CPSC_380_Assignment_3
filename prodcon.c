#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include "shared_buffer.c"
#include "ip_checksum.c"

const char *keyfile; /* name of shared memory key */
int nitems; /* number of items in bounded buffer */
sharedBuffer* shm_buffer; /* shared buffer */
sem_t empty, full; /* semaphores */
pthread_mutex_t mutex; /* mutex */
unsigned short cksum1, cksum2; /* checksums */

void *producer_thread(void *arg);
void *consumer_thread(void *arg);

int main(int argc, char *argv[]) {
	
	if(argc != 3) { /* ensure correct amount of arguments */
		printf("Error: number of arguments passed...{FORMAT: ./prodcon <keyfile> <nitems>}...\n[Exiting]\n");
		return -1;
	}
	if(atoi(argv[2]) > 1024 || atoi(argv[2]) <= 0) {
		printf("Erorr: maximum number of items is 1024\n");
		return -1;
	}
	
	keyfile = argv[1]; 
	nitems = atoi(argv[2]);
	
	int shm_fd; /* shared memory file descriptor */
	void *ptr; /* pointer to shared memory */
	
	shm_fd = shm_open(keyfile, O_CREAT | O_RDWR, 0666); /* shared memory object */
	if(shm_fd == -1) {
		perror("Error: shared memory failed");
		return -1;
	}
	
	ftruncate(shm_fd, MMAP_SIZE); /* size of shared memory object */
	
	ptr = mmap(0, MMAP_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0); /* memory map of shared object */
	
	shm_buffer = (sharedBuffer*)ptr; /* allocate memory for buffer */
	shm_buffer->buffer = (data *) malloc(nitems * sizeof(data));
	shm_buffer->buffer_size = nitems;
	shm_buffer->in = 0;
	shm_buffer->out = 0;
	
	if(sem_init(&empty, 0, nitems) != 0) { /* create empty semaphore */
		perror("Error: semaphore initialization");
		return -1;
	}
	
	if(sem_init(&full, 0, 0) != 0) { /* create full semaphore */
		perror("Error: semaphore initialization");
		return -1;
	}
	
	if(pthread_mutex_init(&mutex, 0)) { /* create mutex */
		perror("Error: semaphore initialization");
		return -1;
	}
	
	pthread_t prod_id, con_id; /* thread id's */ 
	
	pthread_create(&prod_id, NULL, &producer_thread, NULL); /* create threads */
	pthread_create(&con_id, NULL, &consumer_thread, NULL);
	
	pthread_join(prod_id, NULL); /* join threads */
	pthread_join(con_id, NULL);
	
	free(shm_buffer); /* deallocation */ 
	sem_unlink(keyfile);
	sem_destroy(&empty);
	sem_destroy(&full);	
	pthread_mutex_destroy(&mutex);
	
	return 0;
	
}

void *producer_thread(void *arg){ /* producer function */
	int i;
	data produced;
	produced.num = -1;
	
	while(1) {
		produced.num += 1; /* increment buffer */
		
		for(i = 0; i < 34; i++) { /* generate random data */
			produced.r_data[i] = (unsigned char)rand() % 265;
		}
		
		produced.cksum = ip_checksum(produced.r_data, RAND_DATA_SIZE); /* calculate checksum */
		
		sem_wait(&empty); /* wait for empty buffer */ 
		
		pthread_mutex_lock(&mutex); /* lock section critical section */
		
		shm_buffer->buffer[shm_buffer->in] = produced; /* queue data to buffer  */
		
		if(shm_buffer->buffer[shm_buffer->in].cksum = ip_checksum(produced.r_data, RAND_DATA_SIZE)) { /* check the checksum */
			printf("[Produced: %d] [cksum: 0x%x]\n", shm_buffer->buffer[shm_buffer->in].num, shm_buffer->buffer[shm_buffer->in].cksum);
		}
		else{
			printf("Error: checksum");
		}
		
		shm_buffer->in = (shm_buffer->in + 1) % shm_buffer->buffer_size; /* increment */
		
		sem_post(&full); /* signal for consumer */
		
		pthread_mutex_unlock(&mutex); /* unlock critical section */ 
		sleep(0.5);
	}
}

void *consumer_thread(void *arg) { /* consumer thread */ 
	data consumed;
	consumed.num = -1;
	int prev_num;
	
	while(1) {
		prev_num = consumed.num; /* used to verify checksum */
		
		sem_wait(&full); /* wait for buffer to have produced */
		
		pthread_mutex_lock(&mutex); /* lock critical section */ 
		
		for(int i = 0; i < 34; i++) {
			consumed = shm_buffer->buffer[shm_buffer->out]; /* dequeue data */
		}
		
		if(consumed.num != prev_num + 1) {
			printf("Error: Buffer index");
			break;
		}
		
		cksum1 = ip_checksum(consumed.r_data, RAND_DATA_SIZE); /* check checksums */
		cksum2 = consumed.cksum;
		
		if(cksum1 != cksum2) {
			printf("Error: checksum");
			break;
		}
		
		printf("[Consumed: %d] [cksum received: 0x%x] [cksum expected: 0x%x]\n", shm_buffer->buffer[shm_buffer->out].num, cksum2, cksum1);
		
		shm_buffer->out = (shm_buffer->out + 1) & shm_buffer->buffer_size; /* increment */
		
		sem_post(&empty); /* signal for producer */
		
		pthread_mutex_unlock(&mutex); /* unlock critical section */
		sleep(0.5);	
	}
}
