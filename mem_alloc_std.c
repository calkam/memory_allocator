#include "mem_alloc.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "mem_alloc_types.h"

static int __mem_alloc_init_flag=0;

void *malloc(size_t size){
  if(!__mem_alloc_init_flag){
      __mem_alloc_init_flag = 1;
      memory_init();
      //print_info();
  }
  return (void*)memory_alloc((size_t)size);
}

void free(void *p){
  if (p == NULL) return;
  memory_free((char*)p);
  /*print_free_blocks();*/
}

void *calloc(size_t nmemb, size_t size)
{
    if(!__mem_alloc_init_flag){
        __mem_alloc_init_flag = 1;
        memory_init();
        //print_info();
    }
    
    return (void*)memory_alloc((size_t)size*nmemb);
}


void *realloc(void *ptr, size_t size){

    if(!__mem_alloc_init_flag){
        __mem_alloc_init_flag = 1;
        memory_init();
    }
    
    if(ptr == NULL)
        return memory_alloc(size);

//#warning This case needs to be checked again ("ps" or "ls -l" fail)
    /* safety check */
    int padding = (sizeof(mem_bfree_t)% MEM_ALIGNMENT == 0)? 0: MEM_ALIGNMENT - (sizeof(mem_bfree_t) % MEM_ALIGNMENT);
    int real_block_size = sizeof(mem_bfree_t) + padding;
    
    mem_bfree_t *bb = (mem_bfree_t*)((char*)ptr-real_block_size);
    
    fprintf(stderr, "Reallocating %d bytes to %d\n", bb->block_size , (int)size);
    if(size <= bb->block_size)
        return ptr;

    char *new = memory_alloc(size);
    memcpy(new, ptr, bb->block_size);
    memory_free(ptr);
    return (void*)(new);
}
