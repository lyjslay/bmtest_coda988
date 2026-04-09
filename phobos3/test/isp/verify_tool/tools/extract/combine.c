#include "stdlib.h"
#include "stdio.h"
#include "math.h"
#include "string.h"

#define EVEN_SEL 0
#define ODD_SEL  1
#define AVG      2
#define BYPASS   3   
 
int main(int argc, char *argv[])
{
  int height , width, extracting_mode;
  FILE *out_file, *in_file0, *in_file1;

  //open PPM file for reading
  in_file0 = fopen(argv[1], "rb");
  if ( !in_file0 ) {
    printf("Unable to open input file0 '%s'\n", argv[1]);
    exit(1);
  }

  //open PPM file for reading
  in_file1 = fopen(argv[2], "rb");
  if ( !in_file1 ) {
    printf("Unable to open input file1 '%s'\n", argv[1]);
    exit(1);
  }


  out_file = fopen(argv[3], "wb");
  if ( !out_file ) {
    printf("Unable to open output file '%s'\n", argv[2]);
    exit(1);
  }
  
  while( !feof(in_file0) && !feof(in_file1) ){
      unsigned char ch;
      fread(&ch, 1, 1, in_file0);
      if( !feof(in_file0) ) 
         fwrite(&ch, 1, 1, out_file);
      fread(&ch, 1, 1, in_file1);
      if( !feof(in_file1) )
         fwrite(&ch, 1, 1, out_file);
  }

  
  fclose(in_file0);
  fclose(in_file1);
  fclose(out_file);
  return 0;

}
