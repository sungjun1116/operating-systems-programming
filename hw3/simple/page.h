// free frame list

struct frame {
	int pfn;				//page frame number						
	struct frame *p;		//previous frame name
	struct frame *n;		//next frame name
};

#define MAX_FRAMES 0x1000		
struct frame page_frame[MAX_FRAMES];    // 4K frames: 4K*4K=16M

struct frame * get_free_frame();		//비어있는 page frame 중 하나를 가져온다.


void page_init();     //page 초기화


#define NPROC		1 // assume total # proc.

#define ADDR_SIZE	(1<<16)		//16bit address size 1의 2^16
#define PG_SIZE		0x1000		//4kb
#define PG_TBL_SIZE	(ADDR_SIZE / PG_SIZE)		

int pg_table[NPROC][PG_TBL_SIZE];
