#ifndef _H_FUNCTIONS
#define _H_FUNCTIONS



#define SPLIT_LENGTH 128
#define MAX_STR_LENGTH 1024

char** split_str(char* src, char* delimeter, int* count);
int is_executable(char* path);

#endif
