#include "alloc.h"

char* page;
int data[PAGESIZE];

int init_alloc()
{
    int i;

    if(( page = mmap(NULL, // void* start
                    PAGESIZE, // size_t length
                    PROT_READ | PROT_WRITE , // int prot
                    MAP_ANONYMOUS | MAP_PRIVATE, // int flags
                    -1, // int fd
                    0) ) == MAP_FAILED){ // size_t offset
        return -1;
    }
    else{
        for(i = 0; i < PAGESIZE; i++)
            data[i] = 0;

        return 0;
    }
}

int cleanup()
{
    if(munmap(page, PAGESIZE) == -1)
        return -1;
    else
        return 0;
}

char *alloc(int size)
{
    if(size % MINALLOC != 0)
        return NULL;

    int i, j, space = 0, start = 0;

    for(int i = 0; i < PAGESIZE; i++)
    {
        if(data[i] == 0)
        {
            space++;
            if(space == size)
                break;
        }
        else
        {
            start = i + 1;
            space = 0;
        }
    }

    for(i = start; i < start + size; i++)
        data[i] = size;

    return page + start;
}

void dealloc(char *ptr)
{

    int i;
    int start = ptr - page;
    int size = data[ptr - page];

    for(i = 0; i < size; i++)
        data[start + i] = 0;
}
