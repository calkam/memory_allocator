#ifndef   	_MEM_ALLOC_TYPES_H_
#define   	_MEM_ALLOC_TYPES_H_

#define MEM_ALIGNMENT 1

#include <stdint.h>

/* Structure declaration for a free block */
typedef struct memory_block_free{
	int block_size;
	struct memory_block_free *next;
} mem_bfree_t;

typedef struct {
	int block_size;
	uint64_t magic;
} mem_balloc_t;

/* Structure declaration for an allocated block */
//typedef mem_bfree_t mem_balloc_t;

//typedef int mem_balloc_t;

/* Specific metadata for allocated blocks */
/* typedef struct memory_block_allocated{ */
/*     int size; */
/*     /\* ...*\/ */
/* } mem_balloc_t; */

#endif
