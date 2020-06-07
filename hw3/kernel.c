#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "config.h"
#include "kernel.h"


// 페이지 프레임 아이디 별 PA값을 저장합니다.
// 시뮬레이션이므로, 실제 PA 값이 아닌 malloc으로 할당한 메모리 값을 가집니다.
Systemframe *system_page_frame;
int allocated_page_count = 0;
const int max_allocated_page = PHYSICAL_MEMORY_KB * 1024 / PAGE_SIZE;

int allocated_disk_count = 0;
const int max_allocated_disk = DISK_KB * 1024 / PAGE_SIZE;

/*
페이지 프레임 정보를 저장하는 공간을 초기화합니다.
*/
void init_system_page_frame()
{  
    system_page_frame = (Systemframe*)malloc(sizeof(Systemframe) * MAX_PAGE_FRAME);
    
    for (int i = 0; i < MAX_PAGE_FRAME; i++)
    {
        system_page_frame[i].id = i;
        system_page_frame[i].status = PAGE_STAT_UNUSED;
    }

    #ifdef DEBUG
        printf("[DEBUG] init_system_page_frame()\n");
    #endif
}

/*
응용 프로그램에서 사용할 시스템 페이지 프레임을 생성합니다.
malloc() 으로 실제 저장 공간을 생성하고, 해당 값을 PA로 저장하게 됩니다.

반환 값 : 시스템 페이지 프레임 아이디 값
*/
int allocate_page()
{
    int page_id = -1;

    // 빈 페이지를 찾습니다.
    for (int i = 0; i < MAX_PAGE_FRAME; i++)
    {
        if (system_page_frame[i].status != PAGE_STAT_UNUSED)
            continue;

        system_page_frame[i].status = PAGE_STAT_ALLOCATED;
        void* ptr = malloc(PAGE_SIZE);
        memset(ptr, 0, PAGE_SIZE);
        system_page_frame[i].address = ptr;
        #ifdef DEBUG
            printf("[DEBUG] allocate_page() %d is empty space. allocated %p.\n",
                i, system_page_frame[i].address
            );
        #endif

        // 오류 - 이 부분이 호출이 안됨.
        // 공간이 부족한 경우
        if (allocated_page_count >= max_allocated_page)
        {
            #ifdef DEBUG
                printf("[DEBUG] allocate_page() free space not exist.\n");
            #endif

            // 임의 페이지를 스왑합니다.
            swap_and_load_frame(&system_page_frame[i]);
            // 스왑하면 밑의 allocated_page_count는 무효화된다.
            // allocated_page_count--;
        }

        allocated_page_count++;
        page_id = i;
        break;
    }

    assert (page_id != -1);
    return page_id;
}

/*
지정한 페이지 프레임의 메모리를 해제합니다.
*/
void free_page(int page_frame_id)
{
    int page_id = -1;

    for (int i = 0; i < MAX_PAGE_FRAME; i++)
    {
            if (system_page_frame[i].id != page_frame_id)
                continue;

            page_id = i;
            #ifdef DEBUG
                printf("[DEBUG] free_page() %d free %p.\n",
                    i, system_page_frame[i].address
                );
            #endif

            // 지정한 프레임을 찾아서 메모리를 해제하고 반환합니다.
            system_page_frame[i].status = PAGE_STAT_UNUSED;
            free(system_page_frame[i].address);
            allocated_page_count--;
            break;
    }

    assert (page_id != -1);   
}

/*
페이지 주소로 메모리 값을 불러옵니다.

반환 값 : 페이지 데이터가 저장된 메모리 주소 값
*/
void* get_pa_from_page_frame(int page_frame_id)
{
    int page_id = -1;
    
    for (int i = 0; i < MAX_PAGE_FRAME; i++)
    {
            Systemframe *frame = system_page_frame + i;
            if (frame->id != page_frame_id)
                continue;

            page_id = i;
            break;
    }
    
    #ifdef DEBUG
        printf("[DEBUG] get_pa_from_page_frame() %d.\n", page_id);
    #endif

    assert (page_id != -1);   
    assert ((system_page_frame +page_id)->status != PAGE_STAT_UNUSED);
    
    // 디스크 데이터를 원복하고 가져와야 합니다.
    switch ((system_page_frame +page_id)->status)
    {
        case (PAGE_STAT_ALLOCATED):
            // 메모리에 저장되어 있으므로, 바로 실행할 수 있습니다.
            break;

        case (PAGE_STAT_SWAPPED):
            printf("[KERNEL] get_pa_from_page_frame() %d page fault!.\n", page_id);
            swap_and_load_frame(system_page_frame + page_id);
            break;
    }

    return (system_page_frame + page_id)->address;
}

/*
현재 스왑되어 있는 페이지의 메모리 로드를 시도합니다.
더이상 빈 공간이 없다면, 기존에 존재하는 페이지를 임의로 스왑하고 원하는 페이지를 메모리에 로드합니다.
*/
void swap_and_load_frame(Systemframe *frame)
{
    // 메모리에 여유 공간이 존재합니다.
    if (allocated_page_count < max_allocated_page)
    {
        #ifdef DEBUG
        printf("[DEBUG] swap_and_load_frame() free space exist\n");
        #endif
        load_from_disk_frame(frame);
    }
    // 메모리에 여유 공간이 없습니다.
    else
    {
        // 임의 공간을 스왑하고 자신을 불러옵니다.
        int cur_page_id = frame->id;

        for (int i = 0; i < MAX_PAGE_FRAME; i++)
        {
            Systemframe *frame = system_page_frame + i;
            int page_id = frame->id;
            int stat = frame->status;

            // 현재 자신이 아니며, 메모리에 로드되어있는 페이지를 찾습니다.
            if (page_id == cur_page_id || stat != PAGE_STAT_ALLOCATED)
                continue;

            #ifdef DEBUG
            printf("[DEBUG] swap_and_load_frame() select to swap is %d\n", page_id);
            #endif
            save_to_disk_frame(system_page_frame + i); 
            break;
        }
        load_from_disk_frame(frame);
    }
    
}


/*
지정한 페이지를 디스크로 스왑합니다.
*/
void save_to_disk_frame(Systemframe *frame)
{
    if (frame->status != PAGE_STAT_ALLOCATED)
        return;

    if (allocated_disk_count >= max_allocated_disk)
    {
        printf("------------------------------------------------------------------------\n");
        printf("System failure: swapping failed! - no more disk space\n");
        printf("------------------------------------------------------------------------\n");
        assert (0);
    }

    frame->status = PAGE_STAT_SWAPPED;

    allocated_page_count--;
    allocated_disk_count++;

    #ifdef DEBUG
        printf("[DEBUG] save_to_disk_frame() %d saved to disk\n", frame->id);
    #endif
}


/*
지정한 페이지를 디스크에서 메모리로 불러옵니다.
페이지가 디스크에 저장되어 있지 않다면, 작업하지 않습니다.
*/
void load_from_disk_frame(Systemframe *frame)
{
    if (frame->status != PAGE_STAT_SWAPPED)
        return;

    if (allocated_page_count >= max_allocated_page)
    {
        printf("------------------------------------------------------------------------\n");
        printf("System failure: load failed! - no more memory space\n");
        printf("------------------------------------------------------------------------\n");
        assert (0);
    }

    frame->status = PAGE_STAT_ALLOCATED;

    allocated_page_count++;
    allocated_disk_count--;

    #ifdef DEBUG
        printf("[DEBUG] load_from_disk_frame() %d loaded to memory\n", frame->id);
    #endif
}