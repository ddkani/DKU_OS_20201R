#ifndef _H_KERNEL
#define _H_KERNEL

#include <stdio.h>
#include "fs.h"

void print_inode(INODE* inode);
void print_dentry(DENTRY* dentry);

FSTREE* get_fstree_name(FSTREE* tree, char** name, int idx, int finish);
BLOCKS* get_block_id(int block_id);
INODE* get_inode_id(int node_id);
INODE* get_inode_name(char* filename);
void save_inode(int node_id);


void init_system(const char *imgname);
void shutdown_system();

// 가상 API 구현

KFILE* k_fopen(const char *filename, const char *mode);
size_t k_fread(void *ptr, size_t bytes, KFILE *stream);
int k_fclose(KFILE *stream);

#endif