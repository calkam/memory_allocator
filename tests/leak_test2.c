#include "../mem_alloc.h"

int leaking = 1;

void leaking_fun(int n) {
  void *a,*b,*c;
  
  if(n<0) return;
  a = memory_alloc(5);
  b = memory_alloc(10);
  leaking_fun(n-1);
  memory_free(a);
  c = memory_alloc(5);
  leaking_fun(n-2);
  memory_free(c);
  if(!leaking || (n%2)==0) memory_free(b);
}

int main(int argc, char *argv[]) {
    memory_init();
    
  if(argc>1) 
    leaking = 0;
  else 
    leaking = 1;
  leaking_fun(6);
  return 0;
}
