#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv){
	FILE * in;
	FILE * out;
	if(argc < 4){
		printf("input: input_name  output_name  byte_num/line\n");
		printf("example:\n./read_text input.text output.bin 32\n");
		exit(1);
	}
	char * input_name = argv[1];
	char * output_name = argv[2];
	int byte_size = atoi(argv[3]);
	int cl_num = byte_size * 2;
	char cl[cl_num];
	char c;
	unsigned char d[byte_size];
	int d_idx = 0;
	int dump = 0;
	
	printf("input_name:%s output_name:%s byte_size:%d\n", input_name, output_name, byte_size);
	
	if((in = fopen(input_name, "r")) == NULL)
		exit(1);
	if((out = fopen(output_name, "w")) == NULL)
		exit(1);
	while(fscanf(in, "%s", cl) == 1){
		d_idx = byte_size - 1;
		dump = 0;
		memset(d, 0, byte_size);
		//printf("%s\n", cl);
		
		for(int i = 0; i < cl_num; i++){
			c = cl[i];
			if( c == '@'){
				dump = 1;
				break;
			}
			if (c >= '0' && c <= '9') {
				c -= '0';
			} else if (c >= 'a' && c <= 'f') {
				c -= 'a';
				c += 10;
			} else {
				printf("error");
			}
			d[d_idx] <<= 4;
			d[d_idx] |= c;
			if(i % 2 == 1)
				d_idx -= 1;
			if(i == (cl_num - 1)){
				fwrite(d, byte_size, 1, out);
				memset(d, 0, byte_size);
				d_idx = byte_size - 1;
			}
		}
		if(dump == 1)
			continue;
	}
	if(fclose(in) != 0)
		exit(1);
	if(fclose(out) != 0)
		exit(1);
	
	printf("end!!!\n");
	return 0;
}