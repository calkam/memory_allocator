#include "mem_alloc.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "mem_alloc_types.h"

#define couleur(param) printf("\033[%sm",param)

#define MAGIC 314159265358979

/* memory */
char memory[MEMORY_SIZE];

/* Pointer to the first free block in the memory */
mem_bfree_t *first_free;

/* boolean to inform on the corruption of the memory */
int corrupted = 0;

#define ULONG(x)((long unsigned int)(x))
#define max(x,y) (x>y?x:y)

#if defined(FIRST_FIT)

/*{
	Found the first free block with a block size < size
	precondition : size > 0
	initial state : an address and his previous address in the linked list, the size
		to allocate
	final state : the address of a block and his previous block with the block size < size
		NULL else
}*/
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

/*{
	Found the first free block with a minimum block size - size > 0
	precondition : size > 0
	initial state : an address and his previous address in the linked list, the size
		to allocate
	final state : the address of a block and his previous block with the block size < size
		NULL else
}*/
void fit(mem_bfree_t **previous_address, mem_bfree_t **current_address, int size){
	mem_bfree_t *AP = *previous_address;
	mem_bfree_t *AC = *current_address;
	mem_bfree_t *APmin = AP;
	mem_bfree_t *ACmin = AC;
	int found = 0;

	while(AC != NULL){
		if(AC->block_size >= size){
			found = 1;
			if(AC->block_size < ACmin->block_size){
				ACmin = AC;
				APmin = AP;
			}
		}
		AP = AC;
		AC = AC->next;
	}
	
	if(found){
		*previous_address = APmin;
		*current_address = ACmin;
	}else{
		*previous_address = NULL;
		*current_address = NULL;
	}
}

#elif defined(WORST_FIT)

/*{
	Found the first free block with a maximum block size - size > 0
	precondition : size > 0
	initial state : an address and his previous address in the linked list, the size
		to allocate
	final state : the address of a block and his previous block with the block size < size
		NULL else
}*/
void fit(mem_bfree_t **previous_address, mem_bfree_t **current_address, int size){
	mem_bfree_t *AP = *previous_address;
	mem_bfree_t *AC = *current_address;
	mem_bfree_t *APmax = AP;
	mem_bfree_t *ACmax = AC;
	int found = 0;

	while(AC != NULL){
		if(AC->block_size >= size){
			found = 1;
			if(AC->block_size > ACmax->block_size){
				ACmax = AC;
				APmax = AP;
			}
		}
		AP = AC;
		AC = AC->next;
	}
	
	if(found){
		*previous_address = APmax;
		*current_address = ACmax;
	}else{
		*previous_address = NULL;
		*current_address = NULL;
	}
}

#endif

void run_at_exit(void){

	//if the memory is corrupted we don't display the memory state
	if(corrupted){
		return;
	}

	if(first_free != NULL && first_free->block_size == MEMORY_SIZE){
		printf("All the memory have been freed\n");
	}else{
		printf("Allocated blocks haven't been freed\n");
		memory_display_state();
	}
}

void memory_init(void){
	atexit(run_at_exit);
	//Initialisation of the head of the linked list
	first_free = (mem_bfree_t *)memory;
	first_free->block_size = MEMORY_SIZE;
	first_free->next = NULL;
}

/* Add size bytes to the address addr */
mem_bfree_t *add_offset_address(mem_bfree_t *addr, int size){
	return (mem_bfree_t *)((char *)addr + size);
}

/* check if the pointer in the linked list are not corrupted */
int check_pointer(mem_bfree_t *AC){
	if(AC->next == NULL){
		return 1;
	}
	
	long unsigned int anext = ULONG(AC->next);
	long unsigned int ff = ULONG(first_free);

	return (anext > ULONG(AC)) && (anext - ff < MEMORY_SIZE) && (ff - anext > 0);
}

/* check if the alignment is correct */
int check_alignment(mem_bfree_t *AC){
	return ULONG(AC) % MEM_ALIGNMENT == 0;
}

/* check if the magic number at the address AC is not corrupted */
int check_magic(mem_balloc_t *AC){
	return AC->magic == MAGIC;
}

/* check if the memory is not corrupted */
void corruption_check(void){
	char *p = memory;
	int size_total=0;
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
					corrupted = 1;
					exit(0);				
				}
			}else{
				printf("invalid alignment : corrupted free block\n");
				corrupted = 1;
				exit(0);
			}
		}else{
			if(check_magic(((mem_balloc_t *) p))){
				size_total += ((mem_balloc_t *)p)->block_size;
			}else{
				printf("memory corrupted\n");
				corrupted = 1;
				exit(0);
			}
		}
		p = memory + size_total;
	}
	
	if(size_total != MEMORY_SIZE){
		printf("sizes of blocks don't add up to memory_size\n");
		corrupted = 1;
		exit(0);
	}
	
	return;
}

/* DEBUG : print the linked list of free block */
void print_list(){
	mem_bfree_t *AC = first_free;
	
	while(AC != NULL){
		printf("{");
		printf("%lu, ", ULONG(AC) - ULONG(memory));
		printf("%d}\n", AC->block_size);
		AC = AC->next;
	}
}

/* Modify the size of the allocate block to align the allocate block */
int memory_alignment(int size){
	int r = size % MEM_ALIGNMENT;
	
	if(r != 0){
		size += MEM_ALIGNMENT - r;
	}
	
	return size;
}

char *memory_alloc(int size){
	
	if(size < 0){
		printf("size < 0 is impossible\n");
		exit(0);
	}
	
	/* we check the corruption before and after the allocation to be sure
	that the allocation don't fail the allocator */
	corruption_check();

	char *new_offset;
	int original_size;
	mem_balloc_t *block_allocate;
	
	mem_bfree_t *AC = first_free;
	mem_bfree_t *AP = first_free;
	
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
		print_alloc_info(new_offset, original_size - sizeof(mem_balloc_t));
	}else{
		print_alloc_error(original_size - sizeof(mem_balloc_t));
		exit(0);
	}
	
	corruption_check();
	
	return new_offset;
}

/* procedure to merge the consecutive free block */
/* it assume that the system is correct, so there are not
most of 3 consecutive blocks */
void fusion_free(){

	mem_bfree_t *AP = first_free;	
	mem_bfree_t *AC = first_free;
	
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

/* safety check if the block is not already free */
int is_allocated(char *p){
	mem_bfree_t *AC = first_free;
	
	while(AC != NULL && ULONG(p) > (ULONG(AC)+AC->block_size)){
		AC = AC->next;
	}
	
	return (AC == NULL || ULONG(p) <= ULONG(AC));
}

/* safety check if the address is not the beginning of an allocate zone */
int is_allocated_zone(char *p){
	
    mem_balloc_t *block_allocate = (mem_balloc_t*)p;
    
    return block_allocate->magic == MAGIC;
}

void memory_free(char *p){

	corruption_check();

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
}

void memory_display_state(void){
	int *block_size;
	char *p = memory;
	int size_total=0;
	mem_bfree_t *AC = first_free;

	while(ULONG(p) < ULONG(memory) + MEMORY_SIZE){
		if(ULONG(p) == ULONG(AC)){
			couleur("31");
			printf("<metadata__free>");
			couleur("0");
			for(int i=0; i<AC->block_size-sizeof(mem_bfree_t); i++){
				printf(".");
			}
			if(AC != NULL){
				AC = AC->next;
			}
			size_total += ((mem_bfree_t *)p)->block_size;
		}else{
			couleur("34");
			printf("<metadata_alloc>");
			couleur("0");
			block_size = (int *)p;
			for(int i=0; i<(*block_size)-sizeof(mem_bfree_t); i++){
				printf("X");
			}
			size_total += ((mem_balloc_t *)p)->block_size;
		}
		p = memory + size_total;
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

  memory_init();
  print_info();
  
  /*//address is not the beginning of an allocate zone
  char *a = memory_alloc(10);
  memory_free(a+5);*/
  
  /*//pointer is out of memory
  first_free->next = (mem_bfree_t*)100; 
  memory_alloc(10);*/
  
  /*//magic corrupted
  char *a = memory_alloc(10);
  mem_balloc_t *alloc = (mem_balloc_t*)a;
  alloc->magic = 1;
  memory_alloc(10);*/
  
  return EXIT_SUCCESS;
}
#endif 
