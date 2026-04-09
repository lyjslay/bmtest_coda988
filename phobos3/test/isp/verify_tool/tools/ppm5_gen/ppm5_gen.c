#include "stdlib.h"
#include "stdio.h"
#include "math.h"
#include "string.h"

int main(int argc , char * argv[]) 
{
   FILE *in_file, *out_file;
   int width, height, bitwidth;

   //-------------------------------------------------------------------------------------------------
   // Arguments check
   //-------------------------------------------------------------------------------------------------
   if( argc != 6 ) {
       printf("usage in_file out_file width height bitwidth\n");
       exit(1);
   }

   //open PPM file for reading
   in_file = fopen(argv[1], "rb");
   if ( !in_file ) {
    printf("Unable to open input file '%s'\n", argv[1]);
    exit(1);
   }

   out_file = fopen(argv[2], "wb");
   if ( !out_file ) {
     printf("Unable to open output file '%s'\n", argv[2]);
     exit(1);
   }

   width = atoi(argv[3]);
   height = atoi(argv[4]);
   bitwidth = atoi(argv[5]);

   if ( bitwidth %8 != 0 && bitwidth != 12 ){
      printf("The data size is not aligned\n");
      exit(1);
   }
   
   if( bitwidth == 12 && width * height % 2 != 0 ){
      printf("The pix number should be in even when bitwidth is 12\n");
      exit(1);
   }

   //write the header file
   //image format
   fprintf(out_file, "P5\n");

   //comments
   fprintf(out_file, "# Created by CVITEK DPU Model\n");

   //image size
   fprintf(out_file, "%d %d\n",width,height);

   // rgb component depth
   unsigned int max_value; //// supports up to 2**32 - 1 
   if ( bitwidth >= 32) {
     max_value = 0xffffffff;
   }
   else {
     max_value = (1 << bitwidth) - 1;
   }
   fprintf(out_file, "%u\n", max_value);

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
   }
   else {//12bit
     byte_count = width * height*2;
     work_buf = malloc(byte_count);
     for( int pix_idx = 0 ; pix_idx < width * height / 2 ; pix_idx++){
          int b0, b1, b2;
          b0 = fgetc(in_file);
          b1 = fgetc(in_file); 
          b2 = fgetc(in_file);
          unsigned short word0 = ( (b0<<4) | ((b2 & 0xF) &0xf) ) & 0xffff;
          unsigned short word1 = ( (b1<<4) | (b2 >> 4) ) & 0xffff;
          unsigned char byte00, byte01, byte10, byte11;
          byte00 = (word0>>8);
          byte01 = word0&0xff;
          byte10 = (word1>>8);
          byte11 = word1&0xff;
          fwrite(&byte00, 1, 1, out_file);
          fwrite(&byte01, 1, 1, out_file);
          fwrite(&byte10, 1, 1, out_file);
          fwrite(&byte11, 1, 1, out_file);
     } 
   }

   free(work_buf);

   return 0;
}

