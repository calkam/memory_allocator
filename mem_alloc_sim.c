#include "./mem_alloc.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>


#include "./mem_alloc_types.h"

#define MAX_BLOCK 1024

#define PADDING(_x)(((_x) % MEM_ALIGNMENT == 0)? 0: MEM_ALIGNMENT - ((_x) % MEM_ALIGNMENT))

#define REALSIZE(_x)((_x) + PADDING(_x))

/* memory */
char memory[MEMORY_SIZE];

/* allocated blocks*/
typedef struct block{
    int index;
    int size;
    int next;
}block_t;

int nb_blocks=0;
block_t stack[MAX_BLOCK];
int first_block=-1;

static void add_block(int index, int size)
{
    if(nb_blocks == MAX_BLOCK){
        fprintf(stderr, "ERROR: too many allocated blocks for simulation\n");
        exit(1);
    }
    
    if(first_block == -1){
        first_block=0;
        stack[0].index=index;
        stack[0].size=size;
        stack[0].next=-1;
        nb_blocks++;
        return;
    }

    int last_block=first_block;
    while(stack[last_block].next!=-1){
        last_block = stack[last_block].next;
    }

    int free_block=0;
    for(free_block=0; free_block<MAX_BLOCK; free_block++){
        if(stack[free_block].index==-1){
            break;
        }
    }

    stack[free_block].index=index;
    stack[free_block].size=size;
    stack[free_block].next=-1;

    stack[last_block].next=free_block;

    nb_blocks++;
}

static int remove_block(int index)
{
    if(first_block == -1){
        fprintf(stderr, "ERROR: no block to be freed\n");
        exit(1);
    }

    int the_block=first_block;
    int prev_block=-1;

    while(stack[the_block].next!=-1){
        if(stack[the_block].index == index){
            break;
        }
        prev_block=the_block;
        the_block=stack[the_block].next;
    }

    if(stack[the_block].index != index){
        fprintf(stderr, "ERROR: block not found\n");
        exit(1);
    }

    if(the_block == first_block){
        first_block = stack[the_block].next;
    }
    else{
        stack[prev_block].next=stack[the_block].next;
    }
    
    stack[the_block].index=-1;
    nb_blocks--;
    return stack[the_block].size;
}

#if defined(FIRST_FIT)
static int look_for_block(int size)
{
    int begin_address=0;
    int end_address=0;
    int valid_address=0;
    
    while(begin_address < MEMORY_SIZE){
        while(begin_address < MEMORY_SIZE && memory[begin_address]==1){
            begin_address++;
        }
        end_address=begin_address+1;
        while(end_address < MEMORY_SIZE && memory[end_address]==0 && end_address - begin_address < size){
            end_address++;
        }

        if(end_address - begin_address > REALSIZE(sizeof(mem_bfree_t))){
            valid_address=1;
        }
        else{
            int i=0;
            for(i=0; i< REALSIZE(sizeof(mem_bfree_t))-(end_address - begin_address); i++){
                if(memory[end_address+i]==1){
                    break;
                }
            }
            if(end_address - begin_address +i == REALSIZE(sizeof(mem_bfree_t))){
                valid_address=1;
            }   
            
        }
        
        if(end_address - begin_address == size && valid_address == 1){
            return begin_address;
        }
        begin_address=end_address;
    }

    return -1;
}

#elif defined(BEST_FIT)
static int look_for_block(int size)
{
    int begin_address=0;
    int end_address=0;
    int valid_address=0;

    int best_address=MEMORY_SIZE+1;
    int best_size=MEMORY_SIZE+1;
    
    while(begin_address <MEMORY_SIZE){
        while(begin_address < MEMORY_SIZE && memory[begin_address]==1 ){
            begin_address++;
        }
        end_address=begin_address+1;
        while(end_address < MEMORY_SIZE && memory[end_address]==0){
            end_address++;
        }
        
        if(end_address - begin_address > REALSIZE(sizeof(mem_bfree_t))){
            valid_address=1;
        }
        else{
            int i=0;
            for(i=0; i< REALSIZE(sizeof(mem_bfree_t))-(end_address - begin_address); i++){
                if(memory[end_address+i]==1){
                    break;
                }
            }
            if(end_address - begin_address +i == REALSIZE(sizeof(mem_bfree_t))){
                valid_address=1;
            }
        }
        
        if(valid_address==1 && end_address - begin_address >= size && end_address - begin_address < best_size){
            best_address = begin_address;
            best_size = end_address - begin_address;

            if(best_size == size){
                break;
            }
        }
        begin_address=end_address;
    }

    if(best_address == MEMORY_SIZE +1){
        return -1;
    }

    return best_address;    
}

#elif defined(WORST_FIT)
static int look_for_block(int size)
{
    int begin_address=0;
    int end_address=0;
    int valid_address=0;

    int best_address=MEMORY_SIZE+1;
    int best_size=0;
    
    while(begin_address <MEMORY_SIZE){
        while(begin_address < MEMORY_SIZE && memory[begin_address]==1 ){
            begin_address++;
        }
        end_address=begin_address+1;
        while(end_address < MEMORY_SIZE && memory[end_address]==0){
            end_address++;
        }

        if(end_address - begin_address > REALSIZE(sizeof(mem_bfree_t))){
            valid_address=1;
        }
        else{
            int i=0;
            for(i=0; i< REALSIZE(sizeof(mem_bfree_t))-(end_address - begin_address); i++){
                if(memory[end_address+i]==1){
                    break;
                }
            }
            if(end_address - begin_address +i == REALSIZE(sizeof(mem_bfree_t))){
                valid_address=1;
            }
        }

        
        if(valid_address==1 && end_address - begin_address >= size && end_address - begin_address > best_size){
            best_address = begin_address;
            best_size = end_address - begin_address;
        }
        begin_address=end_address;
    }

    if(best_address == MEMORY_SIZE +1){
        return -1;
    }

    return best_address;    
}

#endif


void memory_init(void)
{
    memset(memory, 0, MEMORY_SIZE);

    int i=0;
    for(i=0; i< MAX_BLOCK; i++){
        stack[i].index=-1;
    }
}
    
char *memory_alloc(int size)
{
    int begin_address=0;

    int real_size=REALSIZE(size) + REALSIZE(sizeof(mem_balloc_t));
    int i=0;    

    begin_address = look_for_block(real_size);

    if(begin_address == -1){
        fprintf(stderr, "ALLOC error : can't allocate %d bytes\n", size);
        exit(0);
    }
    

    fprintf(stderr, "ALLOC at : %lu (%d byte(s))\n", 
	    (long unsigned int)(begin_address + REALSIZE(sizeof(mem_balloc_t))), size);


    for(i=0; i<real_size; i++){
        memory[begin_address + i]=1;
    }

    add_block(begin_address,real_size);

    return ((char*)&memory[begin_address])+REALSIZE(sizeof(mem_balloc_t));
}

void memory_free(char *p)
{
    int index = p-memory-REALSIZE(sizeof(mem_balloc_t));
    int size = remove_block(index);

    int i=0;

    for(i=0; i< size; i++){
        memory[index+i]=0;
    }

    fprintf(stderr, "FREE  at : %lu \n", (unsigned long int) (p-memory));
}


void memory_display_state(void)
{
}

void print_info(void)
{
}

void print_alloc_info(char *addr, int size)
{
}

void print_free_info(char *addr)
{
}

void print_alloc_error(int size)
{
}
