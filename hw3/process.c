#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h> // config.h - pow() declaration

#include "config.h"
#include "process.h"
#include "kernel.h"

// 프로세스의 정보를 담고 있습니다.
// 각 프로세스는 개별적인 페이지 테이블을 가지고 있습니다. 
Process* processes;

/*
테이블 페이지 데이터를 초기화합니다.
*/
void init_table_page(Directorypage* directory)
{
    Tablepage* ptr = (Tablepage*)malloc(sizeof(Tablepage) * MAX_TABLE_PAGES);
    assert (ptr != NULL);
    directory -> tables = ptr;
    
    for (int i = 0; i < MAX_TABLE_PAGES; i++)
    {
        Tablepage *t = directory->tables + i;
        t -> id = i;
        t -> page_frame_id = 0;
        t -> initialized = 0;
    }    
}

/*
디렉터리 페이지 데이터를 초기화합니다.
*/
void init_directory_page(Process *process)
{
    Directorypage* ptr = (Directorypage*)malloc(sizeof(Directorypage) * MAX_DIRECTORY_PAGES);
    assert (ptr != NULL);
    process->directories = ptr;

    for (int i = 0; i < MAX_DIRECTORY_PAGES; i++)
    {
        Directorypage *d = process->directories + i;
        d->id = i;
        init_table_page(d);   
    }
}

/*
프로세스 데이터를 초기화합니다.
*/
void init_process()
{
    processes = (Process*)malloc(sizeof(Process) * MAX_PROCESS);

    #ifdef DEBUG
    printf("[DEBUG] init_process()\n");
    printf("[DEBUG] init_process() MAX_PROCESS %d / MAX_DIRECTORY %d / MAX_TABLE %d\n", 
        MAX_PROCESS, MAX_DIRECTORY_PAGES, MAX_TABLE_PAGES);
    #endif

    for (int i = 0; i < MAX_PROCESS; i++)
    {
        Process *p = processes + i;
        p->pid = i;
        init_directory_page(p);
    }
}

// ---------------------------------------------------------------------------------------------------------------


/*
VA를 입력받아 directory_index, table_index, offset을 지정한 주소에 반환합니다.
*/
void va_to_arguments(unsigned int va, int *directory_index, int *table_index, int *offset)
{
    *directory_index = (va & DIRECTORY_INDEX_BIT) >> DIRECTORY_INDEX_SHIFT;
    *table_index = (va & TABLE_INDEX_BIT) >> TABLE_INDEX_SHIFT;
    *offset = va & OFFSET_BIT;
    
    #ifdef DEBUG
    printf("[DEBUG] VA %x translated DI %d | TI %d | OFF %d\n", 
        va, *directory_index, *table_index, *offset);
    #endif
}

/*
지정한 프로세스와 VA로 실제 PA(시뮬레이션이므로 malloc된 데이터가 저장된 주소) 를 반환합니다.

반환 값 : 해당 VA에 해당하는 메모리 주소 값
*/
void* get_pa_from_pid_va(int pid, unsigned int va)
{
     int directory_index;
     int table_index;
     int offset;

    va_to_arguments(va, &directory_index, &table_index, &offset);

    // void* 값을 인덱스 형태로 가져오면 잘못된 접근입니다!!
    Process *p = processes + pid;
    Directorypage *d = p->directories + directory_index;
    Tablepage* t = d->tables + table_index;

    #ifdef DEBUG
            printf("[DEBUG] get_pa_from_va() pid : %d / %d %d\n",
                pid, d->id, t->id
            );
    #endif

    // 아직 해당 구역의 페이지가 사용되지 않았습니다.
    if (t->initialized == 0)
    {
        #ifdef DEBUG
            printf("[DEBUG] get_pa_from_va() pid : %d / %d %d not allocated. Acquire...\n",
                pid, directory_index, table_index
            );
        #endif

        t->page_frame_id = allocate_page();
        t->initialized = 1;

        #ifdef DEBUG
            printf("[DEBUG] get_pa_from_va() pid : %d / %d %d page_frame_id %d\n",
                pid, directory_index, table_index, t->page_frame_id
            );
        #endif
    }

    // 해당 페이지의 시작 주소를 받습니다.
    char* pa_for_table = (char*)get_pa_from_page_frame(t->page_frame_id);
    #ifdef DEBUG
            printf("[DEBUG] get_pa_from_va() pid : %d / %d %d PA %p\n",
                pid, directory_index, table_index, pa_for_table
            );
        #endif
    return (void*)(pa_for_table + offset);
}