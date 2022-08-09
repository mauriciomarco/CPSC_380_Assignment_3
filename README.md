# CPSC_380_Assignment_3
The objective of this assignment is to use semaphores to protect the critical section between two  competing threads. 

## Idendifying Information

* Name: Marco Mauricio
* StudentID: 2344979
* Email: mauricio@chapman.edu
* Course: CPSC 380
* Assignment: 3

## Source Files

* prodcon.c
* shared_buffer.c
* ip_checksum.c
	
## References

* https://www.geeksforgeeks.org/posix-shared-memory-api/
* https://linux.die.net/man/2/ftruncate
* https://man7.org/linux/man-pages/man2/mmap.2.html
* https://docs.oracle.com/cd/E19683-01/806-6867/6jfpgdcnj/index.html
* https://linux.die.net/man/3/pthread_mutex_init
* https://pubs.opengroup.org/onlinepubs/007908799/xsh/pthread_mutex_lock.html
	
## Known Errors

* na

## Build Instructions

* <code> gcc -c ip_checksum.c </code>
* <code> ar -rc libip_checksum.a ip_checksum.o </code>
* <code> gcc prodcon.c -L. -lip_checksum -o prodcon -lrt -lpthread </code>
	
## Execution Instructions

* <code> ./prodcon {keyfile} {nitems} </code>