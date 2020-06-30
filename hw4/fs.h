#ifndef _H_FS
#define _H_FS

/* File system header (FS.H) */

/**
	Some Define values
*/
#define SIMPLE_PARTITION	0x1111

#define INVALID_INODE			0
#define INODE_MODE_AC_ALL		0x777
#define INODE_MODE_AC_USER_R	0x001
#define INODE_MODE_AC_USER_W	0x002
#define INODE_MODE_AC_USER_X	0x004
#define INODE_MODE_AC_OTHER_R	0x010
#define INODE_MODE_AC_OTHER_W	0x020
#define INODE_MODE_AC_OTHER_X	0x040
#define INODE_MODE_AC_GRP_R		0x100
#define INODE_MODE_AC_GRP_W		0x200
#define INODE_MODE_AC_GRP_X		0x400

#define INODE_MODE_REG_FILE		0x10000
#define INODE_MODE_DIR_FILE		0x20000
#define INODE_MODE_DEV_FILE		0x40000

#define DENTRY_TYPE_REG_FILE	0x1
#define DENTRY_TYPE_DIR_FILE	0x2

#define BLOCK_SIZE				0x400

#define INODE_NUM	224 // inode 갯수 지정

// struct 구조체 - typedef 작성

/**
  Partition structure
	ASSUME: data block size: 1K

	partition: 4MB

	Superblock: 1KB
	Inode table (32 bytes inode array) * 224 entries = 7KB
	data blocks: 1KB blocks array (~4K)
*/

/**
  Super block structure

  64비트 시스템 기준입니다.
*/
typedef struct _super_block {
	unsigned int partition_type; // 4369
	unsigned int block_size; // 1024
	unsigned int inode_size; // 32
	unsigned int first_inode; // 2

	unsigned int num_inodes; // 224
	unsigned int num_inode_blocks; // 7
	unsigned int num_free_inodes; // 121

	unsigned int num_blocks; // 4088
	unsigned int num_free_blocks; // 4088
	unsigned int first_data_block; // 8
	char volume_name[24];
	unsigned char padding[960]; //1024-64
} SUPER_BLOCK;

/**
  32 byte I-node structure

64비트 시스템 기준입니다.
*/

/*
unsigned int 4bytes / int 8bytes
4 + 4 + 4 + 4 + 8 = 24
*/

typedef struct _inode {
	unsigned int mode; 		// reg. file, directory, dev., permissions
	unsigned int locked; 	// opened for write
	unsigned int date;
	unsigned int size;
	int indirect_block; 	// N.B. -1 for NULL
	// 첫번째 블록을 dentry로 간주?
	unsigned short blocks[0x6]; // 2bytes * 6 (블록의 고유번호입니다.)
} INODE;

// 실제 데이터를 저장합니다.
typedef struct _blocks {
	// 배열 포인터가 아닌 1024바이트의 길이를 blocks 구조체가 전부 포함합니다.
	unsigned char d[1024]; 
} BLOCKS;

/* physical partition structure */
typedef struct _partition {	
	SUPER_BLOCK s;
	INODE inode_table[224]; // 최대 할당 가능한 파일은 224개 입니다.
	// 여기까지 8192 bytes
	BLOCKS data_blocks[4088]; //4096-8
} PARTITION;

/**
  Directory entry structure - 272bytes
  4 + 4 + 4 + 4 (272 - 16 = 256(names))
*/

/*

*/
typedef struct _dentry {
	unsigned int inode;
	unsigned int dir_length; // 알고보니 이게 디스크에 저장된 dentry의 크기였다 @_@
	unsigned int name_len;
	unsigned int file_type;
	union { // name
		unsigned char name[256];
		unsigned char n_pad[16][16]; // 사용하지 않음.
	};
} DENTRY;


// linked-list로 구현할 수 있다.
typedef struct _fstree {
	char* name;
	unsigned char is_dir;
	unsigned int inode;

	int child_cnt;

	struct _fstree* parent;
	// 파일을 추가하게 될 경우, realloc을 사용하게 될 수 있음.
	struct _fstree** child; // 우선 파일의 추가를 생각하지 않음.
} FSTREE;

typedef struct _k_file {
	FSTREE* fstree;
	int offset; // 현재 파일의 오프셋입니다.
} KFILE;

#endif