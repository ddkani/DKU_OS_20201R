/* kernel.c

운영체제에서 제공하는 파일 접근 함수를 시뮬레이션합니다.
시뮬레이션된 각 함수는 k_ 접두사를 붙여 구현합니다.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "fs.h"  // 파일시스템 관련 structure 정의
#include "config.h" // 환경설정 정의
#include "functions.h"
#include "kernel.h" // 자신의 헤더파일

FILE* f_img; // 이미지 파일 입출력
SUPER_BLOCK* super_block;
INODE inodes[INODE_NUM];

FSTREE* root;

// init_system 에서 초기화함.
long OFFSET_INODE_START;
long OFFSET_BLOCKS_START;

// ****** 테스트 기능 ********
void print_root_fstree()
{
    print_fstree(root);
}


void print_fstree(FSTREE* tree)
{
    printf ("list directory of %s\n", tree->name);

    for (int i = 0; i < tree ->child_cnt; i++)
    {
        FSTREE* cur = tree->child[i];
        printf("%s %d\n",  cur->name, get_inode_id(cur->inode)->size);
        
        if (cur->is_dir) print_fstree(cur);
    }       
}


void print_inode(INODE* inode)
{
    #ifdef DEBUG
    printf("inode ptr : %p\n", inode);
    #endif
    
    unsigned int first_blocks = inode->blocks[0];

    printf("KD] mode = %u, locked = %u, date = %u\n",
        inode->mode, inode->locked, inode->date);
    printf("KD] size = %u, indirect_block = %d, blocks_first = %u\n",
        inode->size, inode->indirect_block, first_blocks);


//BLOCKS* blocks = get_block_id(first_blocks);
//printf("%s\n", blocks->d);

    DENTRY* dentry = (DENTRY*)get_block_id(first_blocks);
    print_dentry(dentry);

    printf("\n");
}


void print_dentry(DENTRY* dentry)
{
    #ifdef DEBUG
    printf("dentry ptr : %p\n", dentry);
    #endif

    printf("KD] inode = %u, dir_length = %u, name_len = %u, file_type = %u\n",
        dentry->inode, dentry->dir_length, dentry->name_len, dentry->file_type);

    printf("KD] filename = %s\n", dentry->name);
}

// *********** 내부 기능 구현 ***************

FSTREE* get_fstree_name(FSTREE* tree, char** name, int idx, int finish)
{
    char* cur = name[idx];
    #ifdef DEBUG
    printf("D] find fstree %s\n", cur);
    #endif

    for (int i = 0; i < tree->child_cnt; i++)
    {
        FSTREE* child = tree->child[i];
        if (strcmp(cur, child->name) == 0)
        {
            if (idx == finish) return child;
            else return get_fstree_name(child, name, idx + 1, finish);
        }
    }
}

/* 지정한 번호의 BLOCKS를 가져옵니다.
해당 데이터는 디스크에서 불러와 동적 할당하였으므로, 사용이 끝나면 해제해야 합니다.
*/
BLOCKS* get_block_id(int block_id)
{
    BLOCKS* blocks = (BLOCKS*)malloc(sizeof(BLOCKS));

    long offset = OFFSET_BLOCKS_START + (block_id * sizeof(BLOCKS));
    fseek(f_img, offset, SEEK_SET);

    #ifdef KDEBUG
    printf("KD] 블록 접근 block_id %d offset : %ld ftell : %ld\n", block_id, offset, ftell(f_img));
    #endif

    if (ftell(f_img) != offset)
    {
        printf("ERR] 잘못된 파일 읽기 시도입니다.\n");
        assert (0);
    }

    size_t cnt = fread((void*)blocks, 1, sizeof(BLOCKS), f_img);

    if (cnt != sizeof(BLOCKS))
    {
        printf("ERR] %d번 BLOCKS를 올바르게 불러오지 못했습니다.\n", block_id);
        printf("ERR] 읽어들인 데이터 : %lu\n", cnt);
        assert (0);
    }

    return blocks;
}

/* 지정한 번호의 INODE를 가져옵니다. */
INODE* get_inode_id(int node_id)
{
    return &inodes[node_id];
}
/* 지정한 파일 이름의 INODE를 가져옵니다. */
INODE* get_inode_name(char* filename)
{
    // TODO: 문자열 파싱해서 디렉터리 순으로 내려가기
    return NULL;
}

/* inode 저장
지정한 inode를 디스크에 기록합니다.
*/
void save_inode(int node_id)
{
    // TODO: 지정한 node_id 의 물리 오프셋을 구해서 해당 크기만큼을 디스크에 씁니다.
}

int is_inode_directory(INODE* inode)
{
    return (inode->mode & 0x20000);
} 

/* 디렉터리 노드를 생성하고 FSTREE를 그립니다.
해당 노드가 / 노드이면 dir_tree를 NULL로 설정합니다.
*/
void init_dentry(INODE* dir_node, FSTREE* dir_tree)
{
    // dentry에 자신의 정보가 맨 처음 있고, 그 다음 파일의 데이터들이 끝까지 존재한다.
    // dir_node는 자신의 부모 노드임.
    // 모든 경우가 동일함.
    // 만약 dir_tree가 NULL이면 새로운 ROOT를 생성합니다.

    FSTREE* current_dir;
    // 루트 노드일 경우 - 외부에서 생성해서 넘겨줌
    // 밑의 디렉터리일 경우 - 역시 외부에서 생성해서 넘겨줌.
    // => 자식 노드에 대한 데이터만 작성하면 된다.

    char* ptr = (char*)get_block_id(dir_node->blocks[0]);
    
    while (1)
    {
        DENTRY* dentry = (DENTRY*)ptr;
    
        if (dentry->dir_length == 0) break;

        FSTREE* fstree = (FSTREE*)malloc(sizeof(FSTREE));
        fstree->name = dentry->name; // 포인터 복사.
        fstree->inode = dentry->inode; // 값 복사.
        // 여기서 디렉터리이면 그대로 재귀함수로 디렉터리 구조를 계속 그립니다.
        fstree->is_dir = is_inode_directory(get_inode_id(dentry->inode));
        if (fstree->is_dir)
            init_dentry(get_inode_id(fstree->inode), fstree);

        fstree->parent = dir_tree;
        dir_tree->child[dir_tree->child_cnt] = fstree;
        dir_tree->child_cnt += 1;

        #ifdef DEBUG
        printf("D] create fstree %s/%s\n", dir_tree->name, fstree->name);
        #endif

        ptr += dentry->dir_length;        
    }
}

/* 시스템 시작
이미지 파일을 읽어들이고, 가상 마운트 작업을 시작합니다.
*/
void init_system(const char *imgname)
{
    OFFSET_INODE_START = sizeof(SUPER_BLOCK);
    OFFSET_BLOCKS_START  =  OFFSET_INODE_START + (sizeof(INODE) * INODE_NUM);

    f_img = fopen(imgname, "r+");

    size_t cnt;
    
    // #1. super_block 로드
    super_block = (SUPER_BLOCK*)malloc(sizeof(SUPER_BLOCK));
    cnt = fread((void*)super_block, 1, sizeof(SUPER_BLOCK), f_img);
    
    if (cnt != 1024)
    {
        printf("ERR] SUPER_BLOCK을 올바르게 불러오지 못했습니다.\n");
        printf("ERR] 읽어들인 데이터 : %lu\n", cnt);
        assert (0);
    }

    cnt = fread((void*)inodes, 1, sizeof(INODE) * INODE_NUM, f_img);

    if (cnt != sizeof(INODE) * INODE_NUM)
    {
        printf("ERR] INODE 테이블을 올바르게 불러오지 못했습니다.\n");
        printf("ERR] 읽어들인 데이터 : %lu\n", cnt);
        assert (0);
    }

    #ifdef KDEBUG
    printf("KD] mount %s to / successful!\n", super_block->volume_name);

    printf("KD] partition_type = %d, block_size = %d, inode_size = %d\n",
        super_block->partition_type, super_block->block_size, super_block->inode_size);
    printf("KD] first_inode = %d, num_inodes = %d, num_inode_blocks = %d\n",
        super_block->first_inode, super_block->num_inodes, super_block->num_inode_blocks);
    printf("KD] num_free_inodes = %d, num_blocks = %d, num_free_blocks = %d, first_data_block = %d\n",
        super_block->num_free_inodes, super_block->num_blocks, super_block->num_free_blocks, super_block->first_data_block);
    #endif

    // 초기 시작된 inode부터 데이터를 읽어들입니다.
    // 맨 처음 루트 데이터는 임의적으로 root를 생성한다.
    INODE* entry_node = get_inode_id(super_block->first_inode);
    root = (FSTREE*)malloc(sizeof(FSTREE));
    root->name = "/";
    root->is_dir = 1;
    root->child = (FSTREE**)malloc(sizeof(FSTREE) * 100);
    init_dentry(entry_node, root);
}

/* 시스템 종료
아직 디스크에 저장되지 않은 내용이 있다면 쓰고, 동적 할당한 메모리를 해제합니다.
*/
void shutdown_system()
{
    // FSTREE 메모리 해제
    // INODE, SUPERBLOCK 메모리 해제
}

// *********** 시뮬레이션 API 구현 ***********

KFILE* k_fopen(const char *filename, const char *mode)
{
    // #1. FSTREE에서 해당 파일을 찾습니다.
    // 문자열을 나눕니다. 맨 첫 문자는 루트파일 밑, 중간은 디렉터리, 마지막은 파일 이름입니다.
    int cnt;
    char** paths = split_str(filename, "/", &cnt);
    assert (cnt != 0);
    
    /*
    ex) /file => file
    ex) /dir/file => dir file    
    */
    
    FSTREE* fstree = get_fstree_name(root, paths, 0, cnt - 1);

    #ifdef DEBUG
    printf("D] fstree filename = %s\n", fstree->name);
    #endif

    // #2. file descriptor에 저장하고, 반환합니다.
    KFILE* f = (KFILE*)malloc(sizeof(KFILE));
    f->fstree = fstree;
    f->offset = 0;

    for (int i = 0; i < cnt; i++) free(paths[cnt]);
    free(paths);

    return f;
}


size_t k_fread(void *ptr, size_t bytes, KFILE *stream)
{
    INODE* node = get_inode_id(stream->fstree->inode);
    
    size_t filesize = node->size;
    size_t readsize = 0;

    // 파일 크기가 읽을 크기보다 클 경우 조정합니다.
    if (bytes > filesize) bytes = filesize;

    char* buf = (char*)ptr;
    int block_idx = stream->offset / 1024;
    int offset = stream->offset % 1024;

    // 오프셋부터 시작해서 파일을 읽어들입니다.
    // 전체 크기를 각각 1024 의 나머지로 나누어 파일을 읽습니다.
    
    // ? - 시작할 때 오프셋?
    // m

    while (1)
    {
        BLOCKS* blocks = get_block_id(node->blocks[block_idx]);
        
        if (bytes + offset <= 1024)
        {
            memcpy(buf, blocks + offset, bytes);
            readsize += bytes;
            free(blocks);
            break;
        }
        else
        {
            // 오프셋 값을 더한 상태에서 1024 - offset
            memcpy(buf, blocks + offset, 1024 - offset);
            buf += 1024 - offset;
            readsize += 1024 - offset;
            free(blocks);
        }

        offset = 0; // 다음번부터 사용되지 않음.
        block_idx++;
        if (block_idx > super_block->num_blocks - 1) 
        {
            printf("KD] 파일의 최대 블럭 갯수를 초과했습니다.");
            assert (0);
        }
    }
    
    stream->offset += readsize;
    return readsize;
}


int k_fclose(KFILE *stream)
{
    // TODO: PEB (Process Environment Block 에 디스크립트 내용 해제)
    free(stream); 
    return 0;
}


/* 추가 구현 사항

멀티 스레드르 사용한다면, k_fopen 에서 PID를 추가로 받아야 한다.
size_t는 long unsigned int 타입이며, size는 long int 타입이다.
*/