#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <stdint.h>
 
int main(int argc, char *argv[]) {
	char *content = "**New Content**";
	char buffer[30];
	struct stat st;
	void *map;
	
	int f = open("/somereadonlyfile", O_RDONLY);
	fstat(f, &st);
	
	map = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, f, 0);
	
	// Open the process's memory pseudo-file
	int fm = open("/proc/self/mem", O_RDWR);
	
	// Start at the 5th byte from the beginning.
	lseek(fm, (off_t) map + 5, SEEK_SET);
	
	// Write to the memory
	write(fm, content, strlen(content));
	
	// Check whether the write is successful
	memcpy(buffer, map, 29);
	printf("Content after write: %s\n", buffer);

	close(f);
	
	// Check content after madvise
	madvise(map, st.st_size, MADV_DONTNEED);
	memcpy(buffer, map, 29);
	printf("Content after madvise: %s\n", buffer);
	
	// gcc COW.c -o COW
	
	return 0;
}