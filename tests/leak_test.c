#include "../mem_alloc.h"

int leaking = 1;

void leaking_fun(int n) {
  void *a,*b,*c;
  if(n<0) return;
  a = malloc(5);
  b = malloc(10);
  leaking_fun(n-1);
  free(a);
  c = malloc(5);
  leaking_fun(n-2);
  free(c);
  if(!leaking || (n%2)==0) free(b);
}

int main(int argc, char *argv[]) {
  if(argc>1) 
    leaking = 0;
  else 
    leaking = 1;
  leaking_fun(6);
  return 0;
}
