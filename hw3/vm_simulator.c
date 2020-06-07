#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "kernel.h"
#include "process.h"

#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
#pragma GCC diagnostic ignored "-Wformat"

void write_file_to_memory(int pid, unsigned int va, size_t *size, char* filename)
{
    FILE *fp = fopen(filename, "r");
    
    char buf;

    printf("%s\n", filename);

    size_t sz;
    while(sz = fread(&buf, 1, 1, fp) > 0)
    {
        
        char* target = get_pa_from_pid_va(pid, va);
        *target = buf; // PA의 주소에 데이터를 씁니다.
        va++;   
        *size += sz;
    }

    fclose(fp);
}


int main(int argc, char **argv)
{
    // kernel - 시스템 페이지 테이블을 초기화합니다.
    init_system_page_frame();
    // process - 사용자 프로세스별 페이지 테이블을 초기화합니다.
    init_process();
    
    #ifdef DEBUG
        printf("[DEBUG] initialize system_page_frame / process OK!\n");
        printf("[DEBUG] maximum process : %d (0 ~ %d)", MAX_PROCESS, MAX_PROCESS - 1);
    #endif

    // 각각 pid0 / pid1 프로세스의 랜덤한 주소에 파일에서 데이터를 입력합니다.
    
    size_t sz_pid0 = 0;
    size_t sz_pid1 = 0;
    unsigned int va_pid0 = rand();
    unsigned int va_pid1 = rand();

    write_file_to_memory(0, va_pid0, &sz_pid0, FILENAME_PID0);
    write_file_to_memory(1, va_pid1, &sz_pid1, FILENAME_PID1);


    // 어디 주소에 데이터를 입력했는지 알려줌.
    printf("PID0 %p : written %s %lu bytes.\n", (void*)va_pid0, FILENAME_PID0, sz_pid0);
    printf("PID1 %p : written %s %lu bytes.\n", (void*)va_pid1, FILENAME_PID1, sz_pid1);


    printf("Entering interactive console. Ctrl+c to quit.\n");
    printf("input) pid addr_lower_hex size\n");
    printf("example) 3 0xcafebabe 16\n\n");

    int pid;
    unsigned int addr; 
    int size;
    while (1)
    {
        printf("> ");
        scanf("%d 0x%x %d", &pid, &addr, &size);
        
        int cur = 0;
        while (cur < size)
        {
            if (cur % 8 == 0) printf("\n(%p) ", addr + cur);
            char d = *((char*)get_pa_from_pid_va(pid, addr + cur));

            if (d == (char)10 || d == (char)0) d = (char)178;

            printf("0x%x [%c] ", d, d);
            cur++;
        }
        printf("\nRead end!\n\n");

    }
    return 0;
}