#include <stdio.h>
#include <stdlib.h>

int main(int argc, char**argv) {
  if(argc != 2) {
     fprintf(stderr,"Usage: %s filename\n", argv[0]);
     return 42;
   }

  FILE* file = fopen(argv[1], "r"); // may return NULL 
  char* line = NULL;
  size_t capacity =0;
  ssize_t bytesread;
  int linenumber = 1;
  while(1) {
    bytesread = getline( &line, &capacity, file);
    if(bytesread == -1) break;
    printf("%3d: %s", linenumber++, line);
  }
  free(line);
  fclose(file);
  return 0;
}
