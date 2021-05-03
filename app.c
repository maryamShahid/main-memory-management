#include <unistd.h>
#include <stdlib.h>

#define SEGSIZE 32768

#include "sbmem.h"

int main()
{
    int i, ret;
    char *p;

    sbmem_init(SEGSIZE);

    ret = sbmem_open();
    if (ret == -1)
        exit (1);

    p = sbmem_alloc (256); // allocate space to hold 1024 characters
    for (i = 0; i < 256; ++i)
        p[i] = 'a'; // init all chars to ‘a’
    sbmem_free (p);

    sbmem_close();

    return (0);
}
