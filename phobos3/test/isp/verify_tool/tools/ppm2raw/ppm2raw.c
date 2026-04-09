#include "stdlib.h"
#include "stdio.h"
#include "math.h"
#include "string.h"

#define EVEN_SEL 0
#define ODD_SEL  1
#define AVG      2
#define BYPASS   3   
 
//This function is to read a ppm file and downsample it in vertical direction,then stores to binary
//in file is in ppm format
//out file is bin format
int main(int argc, char *argv[])
{
   int height , width, bitwidth;
   FILE *out_file, *in_file;

   //-------------------------------------------------------------------------------------------------
   // Arguments check
   //-------------------------------------------------------------------------------------------------
   if( argc != 6 ) {
       printf("usage in_file out_file width height bitwidth\n");
       exit(1);
   }
   bitwidth = atoi(argv[5]);
  //open PPM file for reading
  in_file = fopen(argv[1], "rb");
  if (!in_file) {
    printf("Unable to open input file '%s'\n", argv[1]);
    exit(1);
  }

  out_file = fopen(argv[2], "wb");
  if (!out_file) {
    printf("Unable to open output file '%s'\n", argv[2]);
    exit(1);
  }

  
  //read image format
  char buff[16];
  if (!fgets(buff, sizeof(buff), in_file)) {
    printf("%s read image format failed\n", argv[1]);
    exit(1);
  }
  
  //check the image format
  if (buff[0] != 'P' || buff[1] != '5') {
    printf("Invalid image format (must be 'P5') (error loading '%s')\n", in_file);
    exit(1);
  }

  //check for comments
  char c;
  c = getc(in_file);
  while (c == '#') {
    while (getc(in_file) != '\n');
    c = getc(in_file);
  }

  ungetc(c, in_file);
  //read image size information
  if (fscanf(in_file, "%d %d", &width, &height) != 2) {
    printf("Invalid image size (error loading '%s')\n", in_file);
    exit(1);
  }

  //read max_rgb
  unsigned int max_rgb;
  if (fscanf(in_file, "%d", &max_rgb) != 1) {
    printf("Invalid max_rgb (error loading '%s')\n", in_file);
    exit(1);
  }
  while (fgetc(in_file) != '\n');
  
  unsigned int byte_count;
  unsigned char * work_buf; 
	work_buf = malloc(byte_count);
	if( bitwidth %8 == 0 ){
	  byte_count = width * height*bitwidth/8;
	  work_buf = malloc(byte_count);
	  if( byte_count != fread(work_buf, 1, byte_count, in_file) ){
		 printf("read data failed \n");
		 exit(1);
	 }
	 if( byte_count != fwrite(work_buf, 1, byte_count, out_file) ){
		 printf("write data failed \n");
		 exit(1);
	 }
	}else {//12bit
     byte_count = width * height * 3 / 2;
     work_buf = malloc(byte_count);
     for( int pix_idx = 0 ; pix_idx < width * height / 2 ; pix_idx++){
          int byte00, byte01, byte10, byte11;
          unsigned char b0, b1, b2;
          unsigned short word0, word1;
          byte00 = fgetc(in_file);
          byte01 = fgetc(in_file);
          byte10 = fgetc(in_file);
          byte11 = fgetc(in_file);
          word0 = ((byte00 << 8) | byte01) & 0xffff;
          word1 = ((byte10 << 8) | byte11) & 0xffff;

          b0 = (word0>>4) & 0xFF;
          b1 = (word1>>4) & 0xFF;
          b2 = (((word1 & 0xf) << 4) | (word0 & 0xf))& 0xff;

          fwrite(&b0, 1, 1, out_file);
          fwrite(&b1, 1, 1, out_file);
          fwrite(&b2, 1, 1, out_file);
          // int b0, b1, b2;
          // b0 = fgetc(in_file);
          // b1 = fgetc(in_file); 
          // b2 = fgetc(in_file);
          // unsigned short word0 = ( (b0<<4) | ((b2>>4) &0xf) ) & 0xffff;
          // unsigned short word1 = ( (b1<<4) | (b2 &0xf) ) & 0xffff;
          // unsigned char byte00, byte01, byte10, byte11;
          // byte00 = (word0>>8);
          // byte01 = word0&0xff;
          // byte10 = (word1>>8);
          // byte11 = word1&0xff;
          // fwrite(&byte00, 1, 1, out_file);
          // fwrite(&byte01, 1, 1, out_file);
          // fwrite(&byte10, 1, 1, out_file);
          // fwrite(&byte11, 1, 1, out_file);
     } 
   }

   free(work_buf);
   fclose(in_file);
   fclose(out_file);
}
