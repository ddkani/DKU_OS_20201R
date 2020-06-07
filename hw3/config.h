  
#ifndef _H_CONFIG
#define _H_CONFIG

#define DEBUG

// -----------------------------------------------------------------------------------------

#define FILENAME_PID0 "inputfile0.raw"
#define FILENAME_PID1 "inputfile1.raw"

// -----------------------------------------------------------------------------------------

// 전체 프로세스의 갯수입니다.
#define MAX_PROCESS 10

// 시스템의 최대 페이지 프레임 갯수입니다.
#define MAX_PAGE_FRAME 10240

// 시스템의 물리 메모리입니다. KB 단위입니다.
#define  PHYSICAL_MEMORY_KB 1024
// 임의 지정한 시스템의 메모리 주소 시작지점입니다.
// 이렇게 하면 사용하게 되는 접근 주소는 0x0A000000 ~ 0x0A064000
// #define  PHYSICAL_MEMORY_START_ADDR 0x0A000000

// 시스템의 디스크 크기입니다. KB 단위입니다.
#define DISK_KB 10240

// -----------------------------------------------------------------------------------------

// 페이지 상태
#define PAGE_STAT_UNUSED 0
#define PAGE_STAT_ALLOCATED 1
#define PAGE_STAT_SWAPPED 2

// -----------------------------------------------------------------------------------------

// 4096 바이트
// malloc으로 시뮬레이션하므로 실제 각 페이지 간 간격은 4096 바이트 이상
// 실습용 서버에서는 4112바이트
#define PAGE_SIZE 0x1000

// 12비트 사용
#define DIRECTORY_INDEX_BIT 0xFFF00000
#define DIRECTORY_INDEX_SHIFT 20
#define MAX_DIRECTORY_PAGES (int)pow(2, 12) 

// 8비트 사용
#define TABLE_INDEX_BIT 0x000FF000
#define TABLE_INDEX_SHIFT 12
#define MAX_TABLE_PAGES (int)pow(2, 8)

#define OFFSET_BIT 0x00000FFF

// 32비트를 주소값으로 이용합니다.
// 0xFFFFFFFF == 4294967295
// 4294967295 | 11111111111111111111111111111111

/****** 비트별 데이터 구성도 ******/
// 11111111 11111111 11111111 11111111 VA - 32bit
// 11111111 11110000 00000000 00000000 DIRECTORY_INDEX - 12bit
// 00000000 00001111 11110000 00000000 TABLE_INDEX - 8bit
// 00000000 00000000 00001111 11111111 OFFSET - 12bit
// 각 값을 unsigned int로 구성하고, VA 와 AND 연산하여 값을 얻어냅니다.


#endif