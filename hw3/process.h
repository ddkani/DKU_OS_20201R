#ifndef _H_PROCESS
#define _H_PROCESS

typedef struct _tablepage {
    int id;
    int initialized;
    int page_frame_id;
} Tablepage; 

// 각 디렉터리 페이지는 테이블 페이지의 배열을 가지고 있습니다.
typedef struct _directorypage {
    int id;
    Tablepage *tables;
} Directorypage;

// 각 프로세스는 디렉터리 페이지의 배열을 가지고 있습니다.
typedef struct _process {
    int pid;
    Directorypage *directories;
} Process;

void init_table_page(Directorypage *directory);
void init_directory_page(Process *process);
void init_process();

void va_to_arguments(unsigned int va, int *directory_index, int *table_index, int *offset);
void* get_pa_from_pid_va(int pid, unsigned int va);

#endif