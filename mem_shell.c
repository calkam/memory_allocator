#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "mem_alloc.h"

#define SIZE_BUFFER 128

#define ULONG(x)((long unsigned int)(x))

int main(int argc, char *argv[]) {
	char buffer[SIZE_BUFFER];
	char commande;
	//int offset;
	int index;
	int taille;

	char* block_pointer[1024];
	int count=1;

	memset(block_pointer,0,1024*sizeof(char*));

	memory_init();

	while (1) {
		commande = getchar();
		switch (commande) {
			case 'a':
		  		scanf ("%d",&taille);
			  	block_pointer[count]=memory_alloc(taille);
				count++;
		  	break;
			case 'f': 
	            scanf("%d",&index);
				memory_free(block_pointer[index]);
		  	break;
		  	case 'd':
	            scanf("%d",&index);
                char *addr = block_pointer[index];
                printf("%lu\n", ULONG(addr));
	            printf("%d\n", *addr);
            break;
			case 'p':
				memory_display_state();
			break;
			case 'q':
				exit(0);
			default:
				fprintf(stderr,"Command not found !\n");
		}
		/* empty the rest of the line from the input buffer*/
		fgets(buffer,SIZE_BUFFER,stdin);
	}
	return 0;
}
