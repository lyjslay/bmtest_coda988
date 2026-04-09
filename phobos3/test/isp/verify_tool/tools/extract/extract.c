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
   int height , width, extracting_mode;
   FILE *out_file, *in_file;

   //-------------------------------------------------------------------------------------------------
   // Arguments check
   //-------------------------------------------------------------------------------------------------
   if( argc != 4 ) {
       printf("usage in_file out_file extracting_mode\n");
       exit(1);
   } else {
       extracting_mode = atoi(argv[3]); 
   }

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

  //check max_rgb depth
  int depth = (int)ceil(log2((double)max_rgb));
  int pix_size = (depth + 7) /8;
  int row_size = pix_size * width;
  unsigned char * even_row = malloc(row_size);
  unsigned char * odd_row = malloc(row_size);
  unsigned char * out_row = malloc(row_size);
  if( !even_row || !odd_row || !out_row){
     printf(" memory malloc failed\n ");
     exit(1);    
  }
  
  for ( int row_idx = 0; row_idx < height ; row_idx += 2 ){
       fread(even_row, 1, row_size, in_file);
       fread(odd_row, 1, row_size, in_file);
       if ( extracting_mode == EVEN_SEL ){
           memcpy( out_row, even_row, row_size);
           fwrite(out_row, 1, row_size, out_file);
       }
       else if( extracting_mode == ODD_SEL){
           memcpy( out_row, odd_row, row_size);
           fwrite(out_row, 1, row_size, out_file);
       }
       else if (extracting_mode == BYPASS){
           fwrite(even_row, 1, row_size, out_file);
           fwrite(odd_row, 1, row_size, out_file);
       } 
       else {
                 printf("Not supported now\n");
                 exit(1);
       }
  }
  
  fclose(in_file);
  fclose(out_file);
  return 0;

}
