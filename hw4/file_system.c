#include <stdio.h>
#include <string.h>
#include "fs.h"                             //File system header 선언

FILE *fp;
int ret;
struct dentry *d;
struct opener *k;
struct super_block *s;
struct inode *i;
struct inode *i2;
unsigned char buffer[0x400000];	   			//4mb크기의 buffer를 선언

int main() {
	
	memset(buffer, 0, sizeof(buffer));		
	fp = fopen("disk.img", "rb");			//"disk.img"라는 파일을 read모드로 open
	while (1) {								
		ret = fread(&buffer, sizeof(buffer), 1, fp);
		if (ret == 0)
			break;
	}
	fclose(fp);
	
	FLAG:;                                  //line84의 goto문의 label, filenmae을 다시 입력받기 위함
	
	/*buffer의 처음 인덱스로 포인터를 초기화*/
	
	s = (struct super_block *)&buffer[0];
	i = (struct inode *)&buffer[0];
	i2 = (struct inode *)&buffer[0];
	d = (struct dentry *)&buffer[0];		
	k = (struct opener *)&buffer[0];
	
	/*inode에 해당하는 block으로 포인터 i를 초기화*/
	long long temp1 = (long long)i;			
	temp1 = temp1 + BLOCK_SIZE + ((s->first_inode)*0x20); //super block의 first_inode(2)를 알아내어 이동, 0x440
	i = (struct inode *)temp1;
	
	/*inode의 blocks의 채워진 블록수를 확인*/
	int inode_blocks_number = 0;		
	for (int j = 1; j <= 6; j++) {
		if (i->blocks[j] == 0) {
			inode_blocks_number = j;		//4개의 블록에 대이터가 들어있다면 j=4
			break;
		}
	}
	
	/*data block에 해당하는 부분으로 포인터 d를 초기화*/
	long long res = i->blocks[0];
	long long temp2 = (long long)d;
	temp2 = temp2 + 0x2000 + (0x400*res);//super_block의 메타데이터에 따라 data block은 8번부터 시작.8KB=0x2000
	d = (struct dentry *)temp2;
	
	long long num = 0;						//입력한 filename의 inode number를 기억하기 위한 변수  
	long long cnt = 0;						//입력한 filename이 존재하는지 확인하기 위한 변수
	char in[8];	
	printf("open하고싶은 filename을 입력하세요 : ");
	scanf("%s",in);
	
	while (1) {
		if ((long long)d-(long long)&buffer[0] > 0x2000 + (0x400 * inode_blocks_number))//끝까지 읽는다면 break
			break;
		if (d->name[0] == 0)						//읽는 중간에 null이 나온다면 break
			break;
		if (strcmp(in, d->name) == 0) 				//일치하는 filenmae의 inode를 기억하라	
		{
			num = (long long)d->inode;
			cnt = cnt + num;
		}
			
		printf("file name : %s index: %d\n", d->name, d->inode);		
																		
		long long addr = (long long)d;
		addr = addr + (d->dir_length);					//dir_length가 0x20=>decimal로 32byte
		d = (struct dentry *)addr;						//d가 2줄씩 건너뛰며 증가함
	
	}
	
	/*입력받은 filname이 존재하지 않는다면 다시 입력받기 위함*/
	if (cnt == 0){
		printf("입력하신 filename은 존재하지 않으므로 다시 입력해주세요.\n");
		goto FLAG;
	}
	
	/*입력받은 filename에 해당하는 inode block으로 포인터 i2를 초기화*/
	long long temp3 = (long long)i2;		
	temp3 = temp3 + BLOCK_SIZE + (num*0x20);
	i2 = (struct inode *)temp3;
	
	/*해당하는 file의 내용을 담는 구조체(struct opener)로 포인터 k를 초기화*/
	res = i2->blocks[0];
	long long temp4 = (long long)k;
	temp4 = temp4 + 0x2000 + (0x400*res);
	k = (struct opener *)temp4;

	printf("%s", k->contents);
	return 0;
}