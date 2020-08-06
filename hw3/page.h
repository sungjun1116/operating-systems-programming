// free frame list

struct frame {
	int pfn;

	struct frame *p;
	struct frame *n;
};

#define MAX_FRAMES 0x1000
struct frame page_frame[MAX_FRAMES]; // 4K frames: 4K*4K=16M

struct frame *
	get_free_frame();

void page_init();

#define NPROC		1 // assume total # proc.

#define ADDR_SIZE	(1<<16)
#define PG_SIZE		0x1000
#define PG_TBL_SIZE	(ADDR_SIZE / PG_SIZE)

int pg_table[NPROC][PG_TBL_SIZE];

