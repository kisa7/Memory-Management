#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"

#include "memlib.h"
#define MAX_HEAP (64*(1<<20))  /* 64 MB */

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name : Your student ID */
    "2016-81755",
    /* Your full name */
    "Kozar Anastasia",
    /* Your student ID, again  */
    "2016-81755",
    /* leave blank */
    "",
    /* leave blank */
    ""
};

#define N 26

static char *heap_start;
static char *free_lists[N];

/* DON'T MODIFY THIS VALUE AND LEAVE IT AS IT WAS */
static range_t **gl_ranges;

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
#define SIZE 4


/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(SIZE))

/*
 * remove_range - manipulate range lists
 * DON'T MODIFY THIS FUNCTION AND LEAVE IT AS IT WAS
 */
static void remove_range(range_t **ranges, char *lo)
{
    range_t *p;
    range_t **prevpp = ranges;
    
    if (!ranges)
        return;
    
    for (p = *ranges;  p != NULL; p = p->next) {
        if (p->lo == lo) {
            *prevpp = p->next;
            free(p);
            break;
        }
        prevpp = &(p->next);
    }
}

//add block in free lists

void add_free_block(char* p) {
    size_t size = *(size_t*)p & -2;
    int i = 0;
    char* index;
    *(size_t*)p = size;
    while(size > 1) {
        size >>= 1;
        i++;
    }
    index = free_lists[i];
    if (index == NULL) {
        free_lists[i] = p;
        *(size_t*)(p + 4) = 0;
        return;
    }
    else {
        while((char*)(*(size_t*)(index + 4)) != NULL) {
            index = (char*)(*(size_t*)(index + 4));
        }
        *(size_t*)(index + 4) = (size_t*)p;
        *(size_t*)(p + 4) = 0;
    }
}

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(range_t **ranges)
{
    int i = 0;
    heap_start = mem_heap_lo();
    mem_sbrk(MAX_HEAP);
    size_t size = MAX_HEAP;
    *(size_t*)(heap_start + SIZE) = 9;
    *(size_t*)(heap_start + 2 * SIZE) = 9;
    *(size_t*)(heap_start + size - SIZE) = 1;
    *(size_t*)(heap_start + 3 * SIZE) =  size - 16;
    *(size_t*)(heap_start + size - 2 * SIZE) = size - 16;
    heap_start += 12;
    
    for(i = 0; i < N; i++) {
        free_lists[i] = NULL;
    }
    add_free_block(heap_start);
    /* DON't MODIFY THIS STAGE AND LEAVE IT AS IT WAS */
    gl_ranges = ranges;
    
    return 0;
}

//remove block from lists

void remove_block(char* p)
{
    size_t size = *(size_t*)p & -2;
    int i = 0;
    char* index;
    *(size_t*)p = size;
    while(size > 1) {
        size >>= 1;
        i++;
    }
    index = free_lists[i];
    if (index == NULL)
        return;
    else {
        if(index == p) {
            free_lists[i] = (char*)(*(size_t*)(index + 4));
            return;
        }
        while((char*)(*(size_t*)(index + 4)) != NULL) {
            if((char*)(*(size_t*)(index + 4)) == p) {
                *(size_t*)(index + 4) = *(size_t*)(p + 4);
                return;
            }
            index = (char*)(*(size_t*)(index + 4));
        }
    }
}

//split current block in 2, one with size = size and
//other on what is left

char* split(char* ptr, size_t size) {
    remove_block(ptr);
    size_t old_size = *(size_t*)ptr & -2;
    if(*(size_t*)ptr == 1)                       //end of heap
        return NULL;
    if(old_size - size <= 8) {                  //not too small
        *(size_t*)ptr = old_size | 1;
        *(size_t*)(ptr + old_size - SIZE) = old_size | 1;
        return ptr;
    }
    *(size_t*)ptr = size | 1;
    *(size_t*)(ptr + size) = old_size - size;
    *(size_t*)(ptr + old_size - SIZE) = old_size  - size;
    *(size_t*)(ptr + size  - SIZE) = size | 1;
    add_free_block(ptr + size);
    return ptr;
}

//finds appropriate block in list

char* find_block(size_t size)
{
    size_t init_size = size;
    
    int i = 0;
    char* index;
    while(size > 1) {
        size >>= 1;
        i++;
    }
    
    while (i < N){
        index = free_lists[i];
        while (index != NULL && *(size_t*)index < init_size) {
            index = (char*)(*(size_t*)(index + 4));
        }
        if (index != NULL) {
            return split(index, init_size);
        }
        i++;
    }
    
    return NULL;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */

void* mm_malloc(size_t size)
{
    if(size == 0)
        return NULL;
    size_t new_size = ALIGN(size + SIZE_T_SIZE);
    if(new_size > MAX_HEAP)
        return NULL;
    char *p = find_block(new_size);
    return p + SIZE;
}

//Checks if prev or next block is free and merge if it is
void merge(char* ptr) {
    size_t new_size = 0;
    if(!(*(size_t*)(ptr - 4) % 2)) {   //prev block is also free
        remove_block(ptr - *(size_t*)(ptr - 4));
        remove_block(ptr);
        new_size = *(size_t*)ptr + *(size_t*)(ptr - 4);
        *(size_t*)(ptr - *(size_t*)(ptr - 4)) = new_size;
        *(size_t*)(ptr + *(size_t*)ptr - 4) = new_size;
        add_free_block(ptr - *(size_t*)(ptr - 4));
        
    }
    else if(!(*(size_t*)(ptr + *(size_t*)ptr) % 2)) {  //next block is also free
        remove_block(ptr);
        remove_block(ptr + *(size_t*)ptr);
        new_size = *(size_t*)ptr + *(size_t*)(ptr + *(size_t*)ptr);
        *(size_t*)(ptr + new_size - 4) = new_size;
        *(size_t*)ptr = new_size;
        add_free_block(ptr);
    }
}


// mm_free - Freeing a block does nothing.
//Mark block from pointer as free and call merge

void mm_free(void *ptr)
{
    if(ptr == NULL)
        return;

    char* p = ptr - 4;
    size_t size = *(size_t*)p & -2;
    if(!(*p % 2)) {
        printf("Something is wrong with your intentions");
        return;
    }
    *(size_t*)(p + size - 4) = size;
    *(size_t*)p = size;
    add_free_block(p);
    merge(p);
    
    
    
    // DON't MODIFY THIS STAGE AND LEAVE IT AS IT WAS
    if (gl_ranges)
        remove_range(gl_ranges, ptr);
}

/*
 * mm_realloc - empty implementation; YOU DO NOT NEED TO IMPLEMENT THIS
 */
void* mm_realloc(void *ptr, size_t t)
{
    return NULL;
}

/*
 * mm_exit - finalize the malloc package.
 Goes through whole heap and checks for not freed blocks
 */
void mm_exit(void)
{
    char* tmp;
    char* p = heap_start;
    while(*(size_t*)p != 1) {
        tmp = p + (*(size_t*)p & -2);
        if(*(size_t*)p % 2)
            mm_free(p + 4);
        p = tmp;
    }
}

