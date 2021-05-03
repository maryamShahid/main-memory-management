#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "sbmem.h"

int main() {

    sbmem_init(32768); // segment of size 32 KB
    printf ("memory segment is created and initialized \n");

    return (0);
}
