#include "mem_alloc.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "mem_alloc_types.h"

#define MAGIC 314159265358979
//#define MEMORY_SIZE 512
//#define MAIN

/* memory */
char memory[MEMORY_SIZE];

/* Pointer to the first free block in the memory */
mem_bfree_t *first_free;

#define ULONG(x)((long unsigned int)(x))
#define max(x,y) (x>y?x:y)

#if defined(FIRST_FIT)

void fit(mem_bfree_t **previous_address, mem_bfree_t **current_address, int size){
	mem_bfree_t *AP = *previous_address;
	mem_bfree_t *AC = *current_address;
	
	while(AC != NULL && AC->block_size < size){
		AP = AC;
		AC = AC->next;
	}   

	*previous_address = AP;
	*current_address = AC;
}

#elif defined(BEST_FIT)

void fit(mem_bfree_t **previous_address, mem_bfree_t **current_address, int size){
	mem_bfree_t *AP = *previous_address;
	mem_bfree_t *AC = *current_address;
	mem_bfree_t *APmin = AP;
	mem_bfree_t *ACmin = AC;

	while(AC != NULL){
		if(AC->block_size > size && (AC->block_size < ACmin->block_size)){
			ACmin = AC;
			APmin = AP;
		}
		AP = AC;
		AC = AC->next;
	}
	
	*previous_address = APmin;
	*current_address = ACmin;
}

#elif defined(WORST_FIT)

void fit(mem_bfree_t **previous_address, mem_bfree_t **current_address, int size){
	mem_bfree_t *AP = *previous_address;
	mem_bfree_t *AC = *current_address;
	mem_bfree_t *APmax = AP;
	mem_bfree_t *ACmax = AC;

	while(AC != NULL){
		if(AC->block_size > size && (AC->block_size > ACmax->block_size)){
			ACmax = AC;
			APmax = AP;
		}
		AP = AC;
		AC = AC->next;
	}
	
	*previous_address = APmax;
	*current_address = ACmax;
}

#endif

void run_at_exit(void){
    /* function called when the programs exits */
    /* To be used to display memory leaks informations */
	
	if(first_free != NULL && first_free->block_size == MEMORY_SIZE){
		printf("All the memory have been freed\n");
	}else{
		printf("Allocated blocks haven't been freed\n");
		memory_display_state();
	}
}

void memory_init(void){
	/* register the function that will be called when the programs exits*/
	atexit(run_at_exit);
	first_free = (mem_bfree_t *)memory;
	first_free->block_size = MEMORY_SIZE;
	first_free->next = NULL;
}

mem_bfree_t *add_offset_address(mem_bfree_t *addr, int size){
	return (mem_bfree_t *)((char *)addr + size);
}

int check_pointer(mem_bfree_t *AC){
	if(AC->next == NULL){
		return 1;
	}
	
	long unsigned int anext = ULONG(AC->next);
	long unsigned int ff = ULONG(first_free);

	return (anext > ULONG(AC)) && (anext - ff < MEMORY_SIZE) && (ff - anext > 0);
}

int check_alignment(mem_bfree_t *AC){
	return ULONG(AC) % MEM_ALIGNMENT == 0;
}

int check_magic(mem_balloc_t *AC){
	return AC->magic == MAGIC;
}

void corruption_check(void){
	char *p = memory;
	int size_total;
	mem_bfree_t *AC = first_free;

	while(ULONG(p) < ULONG(memory) + MEMORY_SIZE){
		if(ULONG(p) == ULONG(AC)){
			if(check_alignment(AC)){
				if(check_pointer(AC)){			
					if(AC != NULL){
						AC = AC->next;
					}
					size_total += ((mem_bfree_t *)p)->block_size;
				}else{
					printf("invalid pointer : corrupted free block\n");
					exit(0);				
				}
			}else{
				printf("invalid alignment : corrupted free block\n");
				exit(0);
			}
		}else{
			if(check_magic(((mem_balloc_t *) p))){
				size_total += ((mem_balloc_t *)p)->block_size;
			}else{
				printf("memory corrupted\n");
				exit(0);
			}
		}
		p = memory + size_total;
	}
	
	if(size_total != MEMORY_SIZE){
		printf("sizes of blocks don't add up to memory_size\n");
		exit(0);
	}
	
	return;
}

void print_list(){
	mem_bfree_t *AC = first_free;
	
	while(AC != NULL){
		printf("{");
		printf("%lu, ", ULONG(AC) - ULONG(memory));
		printf("%d}\n", AC->block_size);
		AC = AC->next;
	}
}

int memory_alignment(int size){
	int r = size % MEM_ALIGNMENT;
	
	if(r != 0){
		size += MEM_ALIGNMENT - r;
	}
	
	return size;
}

char *memory_alloc(int size){

	char *new_offset;
	int original_size;
	
	mem_bfree_t *AC = first_free;
	mem_bfree_t *AP = first_free;
	
	mem_balloc_t *block_allocate;
	
	size = size + sizeof(mem_balloc_t);
	
	original_size = size;
	
	size = memory_alignment(size);
	
	fit(&AP, &AC, size);
	
	new_offset = (char *)add_offset_address(AC, sizeof(mem_balloc_t));
	
	if(AC != NULL){
		block_allocate = (mem_balloc_t*)AC;
		if(AC->block_size - size < sizeof(mem_bfree_t) || AC->block_size - size == 0){
			if(AC==first_free){
				first_free = AC->next;
			}else{
				AP->next = AC->next;
			}
			size = AC->block_size; 
		}else if(AC==first_free){
			first_free = add_offset_address(AC, size);
			first_free->next = AC->next;
			first_free->block_size = AC->block_size - size;
		}else{
			AC = add_offset_address(AC, size);
			AC->next = AP->next->next;
			AC->block_size = AP->next->block_size - size;
			AP->next = AC;
		}
		block_allocate->block_size = size;
		block_allocate->magic = MAGIC;
		int old_size = original_size - sizeof(mem_balloc_t);
		print_alloc_info(new_offset, old_size);
	}else{
		print_alloc_error(size);
		exit(0);
	}
	
	print_list();
	corruption_check();
	
	return new_offset;
}

void fusion_free(){

	mem_bfree_t *AP = first_free;	
	mem_bfree_t *AC = first_free;
	
	//print_list();
	
	while(AC != NULL && (ULONG(add_offset_address(AP, AP->block_size)) != ULONG(AC))){
		AP = AC;
		AC = AC->next;
	}
	
	if(AC != NULL){
		AP->block_size += AC->block_size;
		AP->next = AC->next;
	
		AC = AC->next;
	
		if(ULONG(add_offset_address(AP, AP->block_size)) == ULONG(AC)){
			AP->block_size += AC->block_size;
			AP->next = AC->next;
		}
	}
	
}

int is_allocated(char *p){
	mem_bfree_t *AC = first_free;
	
	while(AC != NULL && ULONG(p) > (ULONG(AC)+AC->block_size)){
		AC = AC->next;
	}
	
	return (AC == NULL || ULONG(p) < ULONG(AC));
}

int is_allocated_zone(char *p){
	
    mem_balloc_t *block_allocate = (mem_balloc_t*)p;
    
    return block_allocate->magic == MAGIC;
}

void memory_free(char *p){

	mem_balloc_t *block_allocate = (mem_balloc_t*)(p - sizeof(mem_balloc_t));

	if(!is_allocated(p)){
		printf("This address is already freed\n");
		return;
	}else if(!is_allocated_zone((char *)block_allocate)){
		printf("This address is not the beginning of an allocate zone\n");
		return;
	}

    mem_bfree_t *AC = first_free;
	mem_bfree_t *AP = first_free;
    mem_bfree_t *new_fblock;
    
    int size = block_allocate->block_size;
    
    while(AC != NULL && ULONG(p) > ULONG(AC)){
    	AP = AC;
    	AC = AC->next;
    }
    
    new_fblock = (mem_bfree_t *)(p - sizeof(mem_balloc_t));
    new_fblock->block_size = size;
    new_fblock->next = AC;
    
    if(AC == first_free){
    	first_free = new_fblock;
    }else{
    	AP->next = new_fblock;
    }
	
	print_free_info((char *)new_fblock + sizeof(mem_balloc_t));
	
	fusion_free();
	
	corruption_check();
	print_list();
}

void memory_display_state(void){
	mem_bfree_t *AC = first_free;

	if(AC != NULL){
		int block_size_allocate = ULONG(AC) - ULONG(memory);
		for(int i=0; i<block_size_allocate; i++){
			printf("X");
		}
	}

	while(AC != NULL){
		for(int i=0; i<AC->block_size; i++){
			printf(".");
		}
		if(AC->next != NULL){
			int block_size_allocate = ULONG(AC->next) - (ULONG(AC) + AC->block_size);
			for(int i=0; i<block_size_allocate; i++){
				printf("X");
			}
		}
		AC = AC->next;
	}
	printf("\n");
}


void print_alloc_info(char *addr, int size){
    fprintf(stderr, "ALLOC at : %lu (%d byte(s))\n", ULONG(addr - memory), size);
}


void print_free_info(char *addr){
    fprintf(stderr, "FREE  at : %lu \n", ULONG(addr - memory));
}

void print_alloc_error(int size) 
{
    fprintf(stderr, "ALLOC error : can't allocate %d bytes\n", size);
}

void print_info(void) {
  fprintf(stderr, "Memory : [%lu %lu] (%lu bytes)\n", (long unsigned int) memory, (long unsigned int) (memory+MEMORY_SIZE), (long unsigned int) (MEMORY_SIZE));
}

#ifdef MAIN
int main(int argc, char **argv){

  /* The main can be changed, it is *not* involved in tests */
  memory_init();
  print_info();
  int i;
  for( i = 0; i < 10; i++){
    char *b = memory_alloc(rand()%8);
    memory_free(b);
  }

  char * a = memory_alloc(15); 
  memory_free(a);


  a = memory_alloc(10);
  memory_free(a);

  fprintf(stderr,"%lu\n",(long unsigned int) (memory_alloc(9)));
  return EXIT_SUCCESS;
}
#endif 
