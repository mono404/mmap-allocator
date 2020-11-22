#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>

#define PAGESIZE 4096 //size of memory to allocate from OS
#define MINALLOC 8 //allocations will be 8 bytes or multiples of it

enum flag {Free, Taken};

struct mem
{
    int size;
    char* ptr;
    struct mem *next;
    struct mem *prev;
    int status;
};
typedef struct mem mem;

// function declarations
void init_page(char*, int);
int init_alloc();
int cleanup();
mem find_free(int);
void add_list(mem, int);
void dealloc_list(mem);
char *alloc(int);
void dealloc(char *);
