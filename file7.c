#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[], char *envp[]){
  
  fprintf(stdout, "MYVAR: %s\n", getenv("MYVAR"));
  return 0;
}
