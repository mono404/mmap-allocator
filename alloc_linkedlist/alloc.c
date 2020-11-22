#include "alloc.h"

char *page, *list;
mem *allocMem;

void init_page(char* ptr, int size)
{
    allocMem = (mem*)list;
    allocMem->ptr = page;
    allocMem->size = size;
    allocMem->prev = NULL;
    allocMem->next = NULL;
}

// function declarations
int init_alloc()
{
    if(( page = mmap(NULL, // void* start
                    PAGESIZE, // size_t length
                    PROT_READ | PROT_WRITE , // int prot
                    MAP_ANONYMOUS | MAP_PRIVATE, // int flags
                    -1, // int fd
                    0) ) == MAP_FAILED){ // size_t offset
        return -1;
    }
    else{

        if(( list = mmap(NULL, // void* start
                        PAGESIZE, // size_t length
                        PROT_READ | PROT_WRITE , // int prot
                        MAP_ANONYMOUS | MAP_PRIVATE, // int flags
                        -1, // int fd
                        0) ) == MAP_FAILED){ // size_t offset
            return -1;
        }

        init_page(page, PAGESIZE);

        return 0;
    }
}

int cleanup()
{
    if(munmap(page, PAGESIZE) == -1 && munmap(list, PAGESIZE) == -1)
        return -1;
    else
        return 0;
}

mem find_free(int size)
{
    mem temp;
    
    memcpy(&temp, allocMem, sizeof(mem));

    while(temp.ptr != NULL)
    {
        if(size <= temp.size && temp.status == Free)
            break;

        if(temp.next == NULL)
        {
            temp.ptr = NULL;
            temp.size = 0;
            temp.prev = NULL;
            temp.next = NULL;
            return temp;
        }
        else
            memcpy(&temp, temp.next, sizeof(mem));
    }

    return temp;
}

void add_list(mem temp, int size)
{
    mem *new = (struct mem*)(list + (temp.ptr - page));
    mem *empty = (struct mem*)(list + (temp.ptr - page + size));

    if(temp.ptr == allocMem->ptr)
    {
        empty->ptr = temp.ptr + size;
        empty->size = temp.size - size;
        empty->prev = allocMem;
        empty->next = NULL;

        allocMem->size = size;
        allocMem->prev = NULL;
        allocMem->next = empty;
        allocMem->status = Taken;
    }
    else
    {
        if(temp.size - size != 0)
        {
            new->ptr = temp.ptr;
            new->size = size;
            new->prev = temp.prev;
            new->next = empty;
            new->status = Taken;

            empty->ptr = temp.ptr + size;
            empty->size = temp.size - size;
            empty->prev = new;
            empty->next = temp.next;

            if(temp.next != NULL)
                temp.next->prev = empty;

        }
        else
        {
            new->ptr = temp.ptr;
            new->size = size;
            new->prev = temp.prev;
            new->next = temp.next;
            new->status = Taken;

            if(temp.prev != NULL);
            temp.prev->next = new;
            if(temp.next != NULL)
                temp.next->prev = new;
        }
    }

}

char *alloc(int size)
{
    if(size % 8 != 0)
        return NULL;

    mem temp = find_free(size);

    if(temp.ptr == NULL)
        return NULL;

    char *allocptr = temp.ptr;
    mem *new = (struct mem*)(list + (temp.ptr - page));
    mem *empty = (struct mem*)(list + (temp.ptr - page + size));
    
    add_list(temp, size);

    return allocptr;
}

void dealloc_list(mem temp)
{
    mem* deallocMem = (struct mem*)(list + (temp.ptr - page));
    deallocMem->ptr = temp.ptr;
    deallocMem->size = temp.size;
    deallocMem->prev = temp.prev;
    deallocMem->next = temp.next;
    deallocMem->status = temp.status;


    if(deallocMem == NULL)
        return;

    mem* prev = NULL;
    mem* next = NULL;;
    if(deallocMem->prev != NULL)
        prev = deallocMem->prev;
    if(deallocMem->prev != NULL)
        next = deallocMem->next;
    
    if(deallocMem->ptr == allocMem->ptr)
    {
        if(allocMem->next->status == Free)
        {
            allocMem->size += allocMem->next->size;
            allocMem->next = allocMem->next->next;
            allocMem->status = Free;
        }
        else
        {
            allocMem->status = Free;
        }

        return;
    }
   
    if(next == NULL)
    {
        if(prev->status == Free)
        {
            prev->size += deallocMem->size;
            prev->next = NULL;
            prev->status = Free;
        }
        else
        {
            deallocMem->status = Free;
        }
    }
    else
    {
        if(prev->status == Free && next->status == Free)
        {
            prev->size += deallocMem->size;
            prev->size += next->size;

            if(next->next == NULL)
            {
                prev->next = NULL;
            }
            else
            {
                prev->next = next->next;
                next->next->prev = prev;
            }
            prev->status = Free;
        }
        else if(prev->status == Free && next->status == Taken)
        {
            prev->size += deallocMem->size;
            prev->next = next;
            next->prev = prev;
            prev->status = Free;
        }
        else if(prev->status == Taken && next->status == Free)
        {
            deallocMem->size += next->size;
            
            if(next->next == NULL)
            {
                deallocMem->next = NULL;
            }
            else
            {
                deallocMem->next = next->next;
                next->next->prev = deallocMem;
            }
            deallocMem->status = Free;
        }
        else
        {
            deallocMem->status = Free;
        }
    }
}

void dealloc(char *ptr)
{
    mem temp;

    memcpy(&temp, allocMem, sizeof(mem));

    while(temp.ptr != NULL)
    {
        if(temp.ptr == ptr)
            break;

        if(temp.next == NULL)
            return;
        else
            memcpy(&temp, temp.next, sizeof(mem));
    }

    dealloc_list(temp);
}
