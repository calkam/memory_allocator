#include "mem_alloc.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "mem_alloc_types.h"

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

mem_bfree_t *fit(mem_bfree_t *AP, mem_bfree_t *AC, int size){
	mem_bfree_t *APmax = AP;
	mem_bfree_t *ACmax = AC;

	ACmax->block_size = MEMORY_SIZE;

	while(AC != NULL){
		if(AC->block_size > size && (AC->block_size > ACmax->block_size)){
			ACmax = AC;
			APmax = AP;
		}
		AP = AC;
		AC = AC->next;
	}
	
	AC = ACmax;
	AP = APmax;
}

#endif

void run_at_exit(void){
    /* function called when the programs exits */
    /* To be used to display memory leaks informations */
    
    /* ... */
}

void memory_init(void){
	/* register the function that will be called when the programs exits*/
	atexit(run_at_exit);
	memset(memory, -1, MEMORY_SIZE);
	first_free = (mem_bfree_t *)memory;
	first_free->block_size = MEMORY_SIZE;
	first_free->next = NULL;
	//printf("memory alocated for blocksize : %lu\n", ULONG(&first_free->next)-ULONG(&first_free->block_size));
}

mem_bfree_t *add_offset_address(mem_bfree_t *addr, int size){
	return (mem_bfree_t *)((char *)addr + size);
}

void print_list(){
	mem_bfree_t *AC = first_free;
	
	while(AC != NULL){
		printf("{");
		printf("%lu, ", ULONG(AC));
		printf("%d}\n", AC->block_size);
		AC = AC->next;
	}
}

char *memory_alloc(int size){

	if(size < sizeof(mem_bfree_t)){
		printf("allocating %lu bytes instead of %d\n", sizeof(mem_bfree_t), size);
		size = sizeof(mem_bfree_t);
	}
	
	//TODO: try to put AP at NULL
	mem_bfree_t *AC = first_free;
	mem_bfree_t *AP = first_free;
	
	mem_alloc_t *block_size;
	
	size = size + sizeof(uint16_t);
	
	fit(&AP, &AC, size);
	
	if(AC != NULL){
		block_size = (mem_alloc_t*)AC;
		if(AC->block_size - size < sizeof(mem_bfree_t) || AC->block_size - size == 0){
			// TODO: Ne pas oublier de retirer ce commentaire
			// cette partie est niquée dans le cas du first fit
			if(AC==first_free){
				first_free = AC->next;
			}else{
				AP->next = AC->next;
			}
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
		*block_size = size;
		print_alloc_info((char *)AC, size);
	}else{
		print_alloc_error(size);
		exit(0);
		/* sinon return NULL et vérification de la valeur de retour dans le 
		main pour soulever l'erreur */
	}
	
	print_list();
	
	return (char *)add_offset_address(AC, sizeof(uint16_t));
}

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

void memory_free(char *p){

	/* Warning: do not forget to call print_free_info() */
    mem_bfree_t *AC = first_free;
	mem_bfree_t *AP = first_free;
    mem_bfree_t *new_fblock;
    
    uint16_t size = *(p - sizeof(uint16_t));
    
    while(AC != NULL && ULONG(p) > ULONG(AC)){
    	AP = AC;
    	AC = AC->next;
    }
    
    new_fblock = (mem_bfree_t *)(p - sizeof(uint16_t));
    new_fblock->block_size = size;
    new_fblock->next = AC;
    
    if(AC == first_free){
    	first_free = new_fblock;
    }else{
    	AP->next = new_fblock;
    }
	
	print_free_info((char *)new_fblock);
	
	fusion_free();
	
	print_list();
}

void memory_display_state(void){
	// test purpose	
	/*int indice;
	char *address = memory; //your address  

	for(indice=0;indice<512;indice++) 
	     printf("%02X",*address++);*/
	// end of test

	
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
			int block_size_allocate = AC->next - (AC + AC->block_size);
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
