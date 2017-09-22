#ifndef   	_MEM_ALLOC_H_
#define   	_MEM_ALLOC_H_

/* Allocator functions, to be implemented in mem_alloc.c */
void memory_init(void); 
char *memory_alloc(int size); 
void memory_free(char *p);

/* Display function, to be implemented in mem_alloc.c */
void memory_display_state(void);

/* to display info about memory leaks on program exit*/
void run_at_exit(void);

/* Logging functions */
void print_list(void);

/* to be called when a block has been successfully allocated */
/* addr: the address of the allocated region returned to the user */
/* size: the size of the block allocated to the user*/
void print_alloc_info(char *addr, int size); 

/* to be called on free*/
/* addr: the addr of the user buffer to be freed*/
void print_free_info(char *addr); 

/* to be called when an allocation fails*/
/* size: the size of the block requested by the user*/
void print_alloc_error(int size);

/* displays the size of the memory region*/
void print_info(void);

#include <stdlib.h>


#endif 	    /* !_MEM_ALLOC_H_ */
