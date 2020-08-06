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
*/
struct super_block {			        //파일시스템의 메타 데이타가 들어있다
	unsigned int partition_type;		
	unsigned int block_size;			
	unsigned int inode_size;			
	unsigned int first_inode;			
									
	unsigned int num_inodes;			
	unsigned int num_inode_blocks;		
	unsigned int num_free_inodes;		
	 
	unsigned int num_blocks;			
	unsigned int num_free_blocks;		
	unsigned int first_data_block;		
	char volume_name[24];				//여기까지가 64byte, 이 파일시스템의 이름
	unsigned char padding[960];         //960 + 64 -> 즉 1024byte짜리 자료구조가 만들어지는 것.
								
};								
	
/**
  32 byte I-node structure
*/
struct inode {
	unsigned int mode; 				// reg. file, directory, dev., permissions
	unsigned int locked; 			// opened for write
	unsigned int date;	
	unsigned int size;		
									
	int indirect_block; 			// N.B. -1 for NULL , 이거무시하고 구현하기.
	unsigned short blocks[0x6];		//2byte짜리 자료구조로 데이터블록에서 몇번째 데이터블록인지를 나타냄
};

struct blocks {
	unsigned char d[1024];
};

/* physical partition structure */   	//4MB
struct partition {
	struct super_block s;				//처음 1kb즉 첫 번째 블록은 super_block이다.
	struct inode inode_table[224];		//32byte * 224 = 7KB. 7개의 block이 inode를 저장
	struct blocks data_blocks[4088];	//나머지는 data_block으로 사용됨
};

/**
  Directory entry structure
*/
struct dentry {
	unsigned int inode;			
	unsigned int dir_length;	
	unsigned int name_len;		
	unsigned int file_type;		

	union { 		
		unsigned char name[256];	
		unsigned char n_pad[16][16];	
	};									
};

struct opener {								//원하는 filename을 open하고 출력하기 위한 구조체
	unsigned char contents[32];			
};
