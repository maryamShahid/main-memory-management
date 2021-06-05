# Main Memory Management Library with Buddy Allocation

Main memory simulator is a C application that provides a memory management library for processes. The library provides dynamic memory allocation and deallocation for processes that wish to use main memory. Allocation is made according to the buddy memory allocation algorithm.

#### To Run the Program:

```
$ ./create_memory_sb
$ ./app
$ ./destroy_memory_sb
```

### How to Use:

Compile your application with sbmemlib.a. Import "sbmem.h" to use the library. Here is the basic documentation:

* **`int sbmem_init (int segsize):`** This function will create and initialize a shared memory segment of the given size. The given size is in bytes and must be a power of 2. Memory will be allocated from that segment to the requesting. If operation is successful, the function will return 0, otherwise, it will return -1. 

* **`sbmem_remove ():`** This function removes the shared memory segment from the system.

* **`int sbmem_open ():`** This function indicates to the library that the process would like to use the library. If there are too many processes using the library at the moment, sem_open will return -1. Otherwise, if the process can use the library, sem_open will return 0.

* **`void *sbmem_alloc (int reqsize):`** This function allocates memory of size n, which is the next power of 2 greater than or equal to reqsize. If allocation succeeds, the function will return a pointer to the allocated space. It is up to the program what to store in the allocated space. NULL will be returned if memory could not be allocated. This can happen, for example, when there is not enough memory.

* **`void sbmem_free (void *ptr):`** This function will deallocate the memory space allocated earlier and pointed by the pointer ptr. The deallocated memory space will be part of the free memory in the segment.

* **`int sbmem_close ():`** When a process has finished using the library it should call smem_close(), hence shared segment can be unmapped from the virtual address space of the process. If the process would like to use the library again, it should call smem_open again.
