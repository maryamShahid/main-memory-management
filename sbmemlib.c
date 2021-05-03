#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <semaphore.h>
#include "sbmem.h"
#include "buddy.c"

#define MAX_SEGMENT_SIZE 4194304
#define MIN_SEGMENT_SIZE 32768
#define MAX_PROCESS_COUNT 10

char sharedMemoryName[] = "/sharedMemoryName";

sem_t semaphore;

int shm_fd;
void* shm_fd_ptr;
void* buddyPtr;
int address = 0;
int memorySize = -1;
int processCount = 0;
struct Buddy current;

struct process {
    pid_t pid;
    void *ptr;
};
struct process pList[10];
signed char currentProcesses[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

// For testing purposes
void printFreeList() {
    struct Buddy *bud = &current;
    printf("List:\n");
    while (bud != NULL) {
        printf("(%d,free:%d)", bud->space, bud->free);
        bud = bud->pair;
    }
    printf("\n");
}

int sbmem_init(int segmentsize) {
    if (segmentsize < MIN_SEGMENT_SIZE || segmentsize > MAX_SEGMENT_SIZE) {
        perror("Segment size is not within bounds\n");
        return -1;
    }

    int power = 1;
    while (power < segmentsize) {
        power = power << 1;
    }
    if (power != segmentsize) {
        perror("Segment size must be a power of 2.\n");
        return (-1);
    }

    shm_fd = shm_open(sharedMemoryName, O_CREAT | O_EXCL | O_RDWR, 0666);
    if (shm_fd < 0) {
        if (errno == EEXIST) {
            shm_unlink(sharedMemoryName);
            shm_fd = shm_open(sharedMemoryName, O_CREAT | O_EXCL | O_RDWR, 0666);
        } else {
            perror("Error using shm_open.\n");
            return (-1);
        }
    }
    int f = ftruncate(shm_fd, segmentsize);
    if (f == -1) {
        perror("Error using ftruncate.\n");
        return (-1);
    }
    shm_fd_ptr = mmap(NULL, segmentsize, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if(shm_fd_ptr == MAP_FAILED){
        perror("Error using mmap.\n");
        return (-1);
    }

    memorySize = segmentsize;
    current.free = 1;
    current.pair = NULL;
    current.space = segmentsize;

    sem_init(&semaphore, 1, 1);

    return (0);
}

int sbmem_remove() {
    processCount = 0;
    for (int i = 0; i < MAX_PROCESS_COUNT; i++) {
        currentProcesses[i] = -1;
    }
    memorySize = -1;

    sem_destroy(&semaphore);
    shm_unlink(sharedMemoryName);

    return (0);
}

int sbmem_open() {
    sem_wait(&semaphore);

    if (processCount < MAX_PROCESS_COUNT) {
        for (int i = 0; i < MAX_PROCESS_COUNT; i++) {
            if (currentProcesses[i] < 0) {
                pList[i].pid = getpid();
                pList[i].ptr = mmap(NULL, memorySize, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

                if (pList[i].ptr != MAP_FAILED) {
                    currentProcesses[i] = 1;
                    processCount++;

                    sem_post(&semaphore);

                    return 0;
                }
                sem_post(&semaphore);

                return -1;
            }
        }
    }
    sem_post(&semaphore);
    return (-1);
}

void *sbmem_alloc (int size) {
    if (size < 128 || size > 4096) {
        perror("Size is not within bounds\n");
        exit (1);
    }

    int allocateSize = 2;
    while (1) {
        allocateSize = allocateSize * 2;
        if (size <= allocateSize) {
            break;
        }
    }
    sem_wait(&semaphore);

    // get the memory ptr of the process
    pid_t requestingProcessID = getpid();
    void *processMemoryPtr = NULL;
    // check if process id is valid
    for (int i = 0; i < MAX_PROCESS_COUNT; i++) {
        if (currentProcesses[i] > 0 && pList[i].pid == requestingProcessID) {
            processMemoryPtr = pList[i].ptr;
            break;
        }
    }
    if (processMemoryPtr == NULL) {
        sem_post(&semaphore);
        return NULL;
    }
    int buddyMem = buddyAlloc(&current, allocateSize, address);

    // Remove this - for testing purposes
    printFreeList();

    if (buddyMem < 0) {
        return (NULL);
    }
    sem_post(&semaphore);
    return (shm_fd_ptr + buddyMem);
}

void sbmem_free (void *p) {
    sem_wait(&semaphore);
    int addr;
    addr = p - shm_fd_ptr;

    buddyFree(&current, addr);
    mergeBuddy(&current);

    // Remove this - for testing purposes
    printFreeList();

    sem_post(&semaphore);
}

int sbmem_close() {
    sem_wait(&semaphore);

    munmap(shm_fd_ptr, memorySize);
    pid_t processIDToClose = getpid();
    for (int i = 0; i < MAX_PROCESS_COUNT; i++) {
        if (currentProcesses[i] > 0 && pList[i].pid == processIDToClose) {
            munmap(pList[i].ptr, memorySize);
        }
    }
    sem_post(&semaphore);

    return (0);
}
