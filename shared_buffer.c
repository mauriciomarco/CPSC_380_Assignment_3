#define MMAP_SIZE 32000
#define RAND_DATA_SIZE 34

typedef struct {
	unsigned char r_data[RAND_DATA_SIZE]; /* randomly generated data */
	int num; /* num of data produced */
	unsigned short cksum; /* checksum */
} data;

typedef struct {
	data *buffer; 
	int buffer_size;
	int in; /* queue data */
	int out; /* dequeue data */
} sharedBuffer;