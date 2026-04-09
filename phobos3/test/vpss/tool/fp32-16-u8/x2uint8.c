#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>



typedef struct 
{
	unsigned short  manti : 10;
	unsigned short  exp : 5;
	unsigned short  sign : 1;
} fp16;

typedef struct 
{
  unsigned short  manti: 7;
  unsigned short  exp  : 8;
  unsigned short  sign : 1;
} bf16;

typedef union {
  float f;
  char c[4];
} float2char;

typedef union {
  bf16 f;
  char c[2];
} bfp16tochar;

typedef union {
  fp16 f;
  char c[2];
} fp16tochar;

int main(int argc, char * argv[])
{

	if(argc < 2) {
		printf("./x2uint8 src_file src_format\n");
		exit(0);
	}

	char * files = argv[1];
	char * s_format = argv[2];
	FILE *fd;
	FILE * wfd;
	int retval;


	fd = fopen(files,"rb");
	if (!fd) {

		printf("open file failed %s, %s\n",files,s_format);
		exit(-1);
	}

	fseek(fd, 0, SEEK_END);
	int fileSize = ftell(fd); 
	printf("fileSize %d\n",fileSize);
	fseek(fd, 0, 0);
	char out_files[256];
	sprintf(out_files, "out-%s", files);
	wfd = fopen(out_files,"wb");
	if (!wfd) {

		printf("open file write failed %s\n",out_files);
		exit(-1);
	}

	if(strcmp(s_format,"fp32") == 0){
		char buf1[4];

		for (int i = 0; i < fileSize ; i += 4){

			retval = fread(&buf1, 4, 1, fd);
			
			printf("\n%x,%x,%x,%x",buf1[0],buf1[1],buf1[2],buf1[3]);
			float2char y;
			y.c[0] = buf1[0];
			y.c[1] = buf1[1];
			y.c[2] = buf1[2];
			y.c[3] = buf1[3];
			printf("\n%x,%x,%x,%x",y.c[0],y.c[1],y.c[2],y.c[3]);
			printf("\n %f", y.f);			
	
			char w_char = y.f;
			printf("\n %x", w_char);
			fwrite(&w_char,1,1,wfd);

		}
		fclose(wfd);
		fclose(fd);

	
	}else if(strcmp(s_format,"fp16") == 0){
		char buf1[2];

		for (int i = 0; i < fileSize ; i += 2){
			retval = fread(&buf1, 2, 1, fd);
			
			printf("%x,%x",buf1[0],buf1[1]);
			fp16tochar y;
			y.c[0] = buf1[0];
			y.c[1] = buf1[1];

			printf("\n%x,%x",y.c[0],y.c[1]);
			printf("\n %f", y.f);			
	
			int y_exp = y.f.exp;//(y.f & 0x7F80);
			
			int y_fac = 0x400u + y.f.manti;//(y.f & 0x7F);
			//7: manti bits 
			int l_move = (10 - (y_exp-15));
			int res = (y_fac >> l_move);
			char w_char = res;
			printf("\n %x,%d", y_exp,l_move);
			printf("\n %x", y_fac);

			printf("\n %x", res);

			fwrite(&w_char,1,1,wfd);

		}
		fclose(wfd);
		fclose(fd);
			
	}else if(strcmp(s_format,"bfp16") == 0){
		char buf1[2];

		for (int i = 0; i < fileSize ; i += 2){
			retval = fread(&buf1, 2, 1, fd);
			
			printf("%x,%x",buf1[0],buf1[1]);
			bfp16tochar y;
			y.c[0] = buf1[0];
			y.c[1] = buf1[1];

			printf("\n%x,%x",y.c[0],y.c[1]);
			printf("\n %f", y.f);			
	
			int y_exp = y.f.exp;//(y.f & 0x7F80);

			int y_fac = 0x80u + y.f.manti;//(y.f & 0x7F);
			//7: manti bits 
			int l_move = (7 - (y_exp-127));
			int res = (y_fac >> l_move);
			char w_char = res;
			printf("\n %x,%d", y_exp,l_move);
			printf("\n %x", y_fac);

			printf("\n %x", res);

			fwrite(&w_char,1,1,wfd);

		}
		fclose(wfd);
		fclose(fd);

	}else if(strcmp(s_format,"int8") == 0){
		
		printf("please direct use input files!!");

	}
}
