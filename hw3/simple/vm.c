// virtual memory simulation
// gets input from input_vm
// prints output va -> pa

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "page.h"

// assume Pmem 1MB 
int PMem[0x100000];                          //물리 메모리(0~255 의 page frame number)
static struct frame *free_list;

void init_pmem(){                            //물리 메모리 초기화
	int i;
	for (i = 0 ; i < 0x100000 ; i++) {
		PMem[i] = i;
	}
}

void init_pg_table(){                        //pg_table 초기화
	for(int i=0; i<PG_TBL_SIZE; i++){
		pg_table[NPROC][i]=-1;
	}
}

void access_pa(int pid, int va, int *pa){    //물리주소에 접근해서 va를 pa로 변환시켜 주는 함수
	if(pg_table[NPROC][va>>12]==-1){         //pg_table이 비어있다면
		free_list=get_free_frame();
		pg_table[NPROC][va>>12]=free_list ->p -> pfn;
		printf("fault! freepfn: 0x%x pgn: 0x%x ", pg_table[NPROC][va>>12], va>>12);
	}
	*pa=pg_table[NPROC][va>>12];             //page number를 추출
	*pa=*pa<<12;
	*pa=*pa+(va & 0x00000fff);               //괄호안에서 뒤의 세자리 offset을 추출
}

void main(int argc, char *argv[]){

	char *input_file_name = "input_vm"; 
	FILE *fp; 
	char *line = NULL;
	char *saveptr;

	char *pid_str;
	char *va_str;
	int pid; 
	int va; 
	int pa; 
	char *e;

	int pg;
	int pg_offset;
	size_t len;            

	if (argc == 2) {                          //파일명이 있다면
		input_file_name = argv[1];
	}
	fp = fopen(input_file_name, "r");         //파일 오픈

	page_init();                              //page_frame 초기화
	init_pmem();                              //물리 메모리 초기화
	init_pg_table();                          //pg_table 초기화

	// get a line from input
	while (getline(&line, &len, fp) != -1) {
		pid_str = strtok_r(line, " \n", &saveptr);
		va_str = strtok_r(NULL, " \n", &saveptr);

		pid = strtol(pid_str, &e, 16);
		va = strtol(va_str, &e, 16);
		pa = 0;                               // needs to be translated from va

		access_pa(pid, va, &pa);              // 핵심 매핑 함수(line27~line36) 
		printf("pid: %d, va: 0x%08x pa: [0x%08x] = 0x%08X\n", pid, va, pa, PMem[pa]);
	}

	fclose(fp);
}