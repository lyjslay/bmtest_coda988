#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "dpu_common.h"

typedef union {
	unsigned short PixVal;
	unsigned char Byte[2];
} PixLayout;

//--------------------------------------------------------------------------------------
// NewFRAME: create a new empty frame
//--------------------------------------------------------------------------------------
FRAME *CM_NewFRAME(FRAME_TYPE type, COLOR_FORMAT format, int width, int height, int channel, int depth)
{
    FRAME *img;
    //alloc memory form image
    img = (FRAME *)malloc(sizeof(FRAME));
    if (!img) {
         fprintf(stderr, "Unable to allocate memory\n");
         exit(1);
    }
    //memory allocation for pixel data
    //printf("Malloc width: %d height:%d channel:%d\n", width, height, channel);
    img->type   = type;
    img->format = format;
    img->width  = width;
    img->height = height;
    img->depth  = depth;
    img->channel = channel;

    img->data   = (int *)malloc(width * height* channel * sizeof(int));
    if (!img->data) {
         fprintf(stderr, "Unable to allocate memory\n");
         exit(1);
    }
    return img;
}
//--------------------------------------------------------------------------------------
// CopyFRAME: copy a frame
//--------------------------------------------------------------------------------------
FRAME *CM_CopyFRAME(FRAME *img)
{
     FRAME *out;
     int x, y, ch;

     out = CM_NewFRAME(img->type, img->format, img->width, img->height, img->channel, img->depth);
     for(y=0;y<out->height; y++)
          for(x=0; x< out->width; x++)
               for(ch=0; ch<out->channel; ch++)
     {
          PIXEL(out, x, y, ch) = PIXEL(img, x, y, ch);
     }
     return out;
}

FRAME *CM_CopySingleCh(FRAME *img, int ch)
{
     FRAME *out;
     int x, y;

     out = CM_NewFRAME(img->type, GRAY, img->width, img->height, 1, img->depth);
     for(y=0;y<out->height; y++)
          for(x=0; x< out->width; x++)
     {
          PIXEL(out, x, y, 0) = PIXEL(img, x, y, ch);
     }
     return out;
}

FRAME *CM_Merge3Ch(FRAME *src0, FRAME *src1, FRAME *src2, COLOR_FORMAT format)
{
     FRAME *out;
     int x, y;

     out = CM_NewFRAME(src0->type, format, src0->width, src0->height, 3, src0->depth);
     for(y=0;y<out->height; y++)
          for(x=0; x< out->width; x++)
     {
          PIXEL(out, x, y, 0) = PIXEL(src0, x, y, 0);
          PIXEL(out, x, y, 1) = PIXEL(src1, x, y, 0);
          PIXEL(out, x, y, 2) = PIXEL(src2, x, y, 0);
     }
     return out;
}

//--------------------------------------------------------------------------------------
// DelFRAME: delete a frame
//--------------------------------------------------------------------------------------
int CM_DelFRAME(FRAME *img)
{
     if (img != NULL && img->data != NULL) {
          free(img->data);
          free(img);
     } else {
          if (img == NULL)
               printf("NULL ptr img:%x\n", img);
          else
               printf("NULL ptr img->data:%x\n", img->data);

          printf("Press any key to continue...\n");
          getchar();
     }
    return 0;
}

//--------------------------------------------------------------------------------------
// ReadPPM: read a RGB PPM into a frame
//--------------------------------------------------------------------------------------
FRAME *CM_ReadPPM(const char *filename, FRAME_TYPE type)
{
     char  buff[16], c;
     FRAME *img;
     FILE  *fp;
     int width, height, max_rgb;

     // open PPM file for reading
     fp = fopen(filename, "rb");
     if (!fp) {
          fprintf(stderr, "Unable to open file '%s'\n", filename);
          exit(1);
     }

     // read image format
     if (!fgets(buff, sizeof(buff), fp)) {
          perror(filename);
          exit(1);
     }

     // check the image format
     if (buff[0] != 'P' || buff[1] != '6') {
          fprintf(stderr, "Invalid image format (must be 'P6') (error loading '%s')\n", filename);
          exit(1);
     }

     // check for comments
     c = getc(fp);
     while (c == '#') {
     while (getc(fp) != '\n') ;
          c = getc(fp);
     }

     ungetc(c, fp);
     // read image size information
     if (fscanf(fp, "%d %d", &width, &height) != 2) {
          fprintf(stderr, "Invalid image size (error loading '%s')\n", filename);
          exit(1);
     }

     // read max_rgb
     if (fscanf(fp, "%d", &max_rgb) != 1) {
          fprintf(stderr, "Invalid max_rgb (error loading '%s')\n", filename);
          exit(1);
     }
     while (fgetc(fp) != '\n') ;

     // check max_rgb depth
     int depth = (int) ceil(log2((double) max_rgb));
     printf("Color depth (%d) max_rgb:(%d)\n", depth, max_rgb);

     // new frame
     int channel = 3;
     img = CM_NewFRAME(type,RGB, width, height, channel, depth);
     if (!img) {
          fprintf(stderr, "Unable to allocate memory\n");
          exit(1);
     }

     int x, y;
     int byte0 = 0;
     int byte1 = 0;
     int byte2 = 0;
     int byte3 = 0;
     for(y=0; y<height; y++) {
          for(x=0; x<width; x++) {
               for(c=0; c<3; c++) {
                    if(depth > 24) {byte3 = fgetc(fp);}
                    if(depth > 16) {byte2 = fgetc(fp);}
                    if(depth > 8) {byte1 = fgetc(fp);}
                    if(depth > 0) {byte0 = fgetc(fp);}
                    PIXEL(img,x,y,c) = (int)((byte3<<24)+(byte2<<16)+(byte1<<8)+(byte0));
                    if(feof(fp)) break;
               }
          }
     }
     fclose(fp);

     return img;
}

//--------------------------------------------------------------------------------------
// ReadPPM: read a GRAY PPM into a frame
//--------------------------------------------------------------------------------------
FRAME *CM_ReadPPM5(const char *filename, FRAME_TYPE type)
{
     char  buff[16], c;
     FRAME *img;
     FILE  *fp;
     int width, height, max_rgb;

     //open PPM file for reading
     fp = fopen(filename, "rb");
     if (!fp) {
          fprintf(stderr, "Unable to open file '%s'\n", filename);
          exit(1);
     }

     //read image format
     if (!fgets(buff, sizeof(buff), fp)) {
          perror(filename);
          exit(1);
     }

     //check the image format
     if (buff[0] != 'P' || buff[1] != '5') {
          fprintf(stderr, "Invalid image format (must be 'P5') (error loading '%s')\n", filename);
          exit(1);
     }

     //check for comments
     c = getc(fp);
     while (c == '#') {
     while (getc(fp) != '\n') ;
          c = getc(fp);
     }

     ungetc(c, fp);
     //read image size information
     if (fscanf(fp, "%d %d", &width, &height) != 2) {
          fprintf(stderr, "Invalid image size (error loading '%s')\n", filename);
          exit(1);
     }

     //read max_rgb
     if (fscanf(fp, "%d", &max_rgb) != 1) {
          fprintf(stderr, "Invalid max_rgb (error loading '%s')\n", filename);
          exit(1);
     }
     while (fgetc(fp) != '\n') ;

     //check max_rgb depth
     int depth = (int) ceil(log2((double) max_rgb));
     printf("Color depth (%d) max_rgb:(%d)\n", depth, max_rgb);

     //new frame
     int channel = 1;
     img = (FRAME *) CM_NewFRAME(type,GRAY, width,height,channel,depth);
     if (!img) {
          fprintf(stderr, "Unable to allocate memory\n");
          exit(1);
     }

     //read pixel data from file
     int x, y;
     int byte0, byte1, byte2, byte3;
     for(y=0; y<height; y++)
          for(x=0; x<width; x++)
               for(c=0; c<1; c++)
     {
          byte0 = byte1 = byte2 = byte3 = 0;
          if(depth>24)   byte3 = fgetc(fp);
          if(depth>16)   byte2 = fgetc(fp);
          if(depth>8)    byte1 = fgetc(fp);
          if(depth>0)    byte0 = fgetc(fp);
          PIXEL(img,x,y,c) = (int)((byte3<<24)+(byte2<<16)+(byte1<<8)+(byte0));
          if(feof(fp))      break;
     }
     //printf("SUCCESS\n");
     fclose(fp);

     return img;
}
//--------------------------------------------------------------------------------------
// WritePPM: write a PPM into a frame
//--------------------------------------------------------------------------------------
int CM_WritePPM(const char *filename, FRAME *img)
{
     FILE *fp;
     int i;
     //open file for output
     fp = fopen(filename, "wb");
     if (!fp) {
          fprintf(stderr, "Unable to open file '%s'\n", filename);
          exit(1);
     }

     //write the header file
     //image format
     fprintf(fp, "P6\n");

     //comments
     fprintf(fp, "# Created by CVITEK DPU Model\n");

     //image size
     fprintf(fp, "%d %d\n",img->width,img->height);

     // rgb component depth
     fprintf(fp, "%d\n", (1<<img->depth)-1);

     // pixel data
     int x, y;
     for(y=0; y < img->height; y++)
          for(x=0; x < img->width; x++)
     {
          int iR, iG, iB;
          if(img->channel == 3)
          {
               iR = PIXEL(img, x, y, 0);
               iG = PIXEL(img, x, y, 1);
               iB = PIXEL(img, x, y, 2);
          } else
          {
               iR = PIXEL(img, x, y, 0);
               iG = PIXEL(img, x, y, 0);
               iB = PIXEL(img, x, y, 0);
          }
          unsigned char c;
          if(img->depth > 24)
          {
               c = (iR & 0xFF000000)>>24;     fwrite(&c, 1, 1, fp);
               c = (iR & 0x00FF0000)>>16;     fwrite(&c, 1, 1, fp);
               c = (iR & 0x0000FF00)>>8 ;     fwrite(&c, 1, 1, fp);
               c = (iR & 0x000000FF)>>0 ;     fwrite(&c, 1, 1, fp);
               c = (iG & 0xFF000000)>>24;     fwrite(&c, 1, 1, fp);
               c = (iG & 0x00FF0000)>>16;     fwrite(&c, 1, 1, fp);
               c = (iG & 0x0000FF00)>>8 ;     fwrite(&c, 1, 1, fp);
               c = (iG & 0x000000FF)>>0 ;     fwrite(&c, 1, 1, fp);
               c = (iB & 0xFF000000)>>24;     fwrite(&c, 1, 1, fp);
               c = (iB & 0x00FF0000)>>16;     fwrite(&c, 1, 1, fp);
               c = (iB & 0x0000FF00)>>8 ;     fwrite(&c, 1, 1, fp);
               c = (iB & 0x000000FF)>>0 ;     fwrite(&c, 1, 1, fp);
          }
          else if(img->depth > 16)
          {
               c = (iR & 0x00FF0000)>>16;     fwrite(&c, 1, 1, fp);
               c = (iR & 0x0000FF00)>>8 ;     fwrite(&c, 1, 1, fp);
               c = (iR & 0x000000FF)>>0 ;     fwrite(&c, 1, 1, fp);
               c = (iG & 0x00FF0000)>>16;     fwrite(&c, 1, 1, fp);
               c = (iG & 0x0000FF00)>>8 ;     fwrite(&c, 1, 1, fp);
               c = (iG & 0x000000FF)>>0 ;     fwrite(&c, 1, 1, fp);
               c = (iB & 0x00FF0000)>>16;     fwrite(&c, 1, 1, fp);
               c = (iB & 0x0000FF00)>>8 ;     fwrite(&c, 1, 1, fp);
               c = (iB & 0x000000FF)>>0 ;     fwrite(&c, 1, 1, fp);
          }
          else if( img->depth > 8 )
          {
               c = (iR & 0x0000FF00)>>8 ;     fwrite(&c, 1, 1, fp);
               c = (iR & 0x000000FF)>>0 ;     fwrite(&c, 1, 1, fp);
               c = (iG & 0x0000FF00)>>8 ;     fwrite(&c, 1, 1, fp);
               c = (iG & 0x000000FF)>>0 ;     fwrite(&c, 1, 1, fp);
               c = (iB & 0x0000FF00)>>8 ;     fwrite(&c, 1, 1, fp);
               c = (iB & 0x000000FF)>>0 ;     fwrite(&c, 1, 1, fp);
          }
          else
          {
               c = (iR & 0x000000FF)>>0 ;     fwrite(&c, 1, 1, fp);
               c = (iG & 0x000000FF)>>0 ;     fwrite(&c, 1, 1, fp);
               c = (iB & 0x000000FF)>>0 ;     fwrite(&c, 1, 1, fp);
          }
     }
     fclose(fp);

     return 0;
}

//--------------------------------------------------------------------------------------
// WritePPM: write a PPM into a frame
//--------------------------------------------------------------------------------------
int CM_WritePPM5(const char *filename, FRAME *img)
{
     FILE *fp;
     //open file for output
     fp = fopen(filename, "wb");
     if (!fp) {
          fprintf(stderr, "Unable to open file '%s'\n", filename);
          exit(1);
     }

     //write the header file
     //image format
     fprintf(fp, "P5\n");

     //comments
     fprintf(fp, "# Created by CVITEK DPU Model\n");

     //image size
     fprintf(fp, "%d %d\n",img->width,img->height);

     // rgb component depth
     fprintf(fp, "%d\n", (1<<img->depth)-1);

     // pixel data
     int x, y;
     for(y=0; y < img->height; y++)
          for(x=0; x < img->width; x++)
     {
          int iG;
          iG = PIXEL(img, x, y, 0);

          unsigned char c;
          if(img->depth > 16)
          {
               c = (iG & 0xFF000000)>>24;     fwrite(&c, 1, 1, fp);
               c = (iG & 0x00FF0000)>>16;     fwrite(&c, 1, 1, fp);
               c = (iG & 0x0000FF00)>>8 ;     fwrite(&c, 1, 1, fp);
               c = (iG & 0x000000FF)>>0 ;     fwrite(&c, 1, 1, fp);
          }
          else if( img->depth > 8 )
          {
               c = (iG & 0x0000FF00)>>8 ;     fwrite(&c, 1, 1, fp);
               c = (iG & 0x000000FF)>>0 ;     fwrite(&c, 1, 1, fp);
          }
          else
          {
               c = (iG & 0x000000FF)>>0 ;     fwrite(&c, 1, 1, fp);
          }
     }
     fclose(fp);

     return 0;
}

//--------------------------------------------------------------------------------------
// RGB2YCbCr: Convert RGB to YCbCr
//--------------------------------------------------------------------------------------
FRAME *CM_RGB2YCbCr(FRAME *img)
{
     int x, y;
     int r0, g0, b0;
     int y0, cb0, cr0;
     FRAME *new_img;
     int min0, max0, half0;

     if(img->format != RGB)   {    printf("RGB2YCbCr format error\n");     exit(-1); }

     new_img = CM_NewFRAME(img->type,YCBCR, img->width,img->height,img->channel,img->depth);
     min0 = 0;
     max0 = (1<<img->depth)-1;
     half0 = (1<< (img->depth -1));
     for(y=0; y< img->height; y++)
          for(x=0; x< img->width; x++)
     {
          r0 = PIXEL(img, x, y, R);
          g0 = PIXEL(img, x, y, G);
          b0 = PIXEL(img, x, y, B);
          y0  = UROUND_BITWISE((77 * r0 + 150 * g0 + 29 * b0), 8);
          cb0 = SROUND_BITWISE((-44 * r0 - 87 * g0 + 131 * b0 + (half0<<8)), 8, img->depth);
          cr0 = SROUND_BITWISE((131 * r0 - 110 * g0 - 21 * b0 + (half0<<8)), 8, img->depth);
          PIXEL(new_img, x, y, Y ) = y0;
          PIXEL(new_img, x, y, CB) = cb0;
          PIXEL(new_img, x, y, CR) = cr0;
     }
     //CM_DelFRAME(img);

     return new_img;
}

//--------------------------------------------------------------------------------------
// YCbCr444to422: Convert YCbCr 444 to YCbCr 422
//--------------------------------------------------------------------------------------
int CM_YCbCr444to422(FRAME *YCbCr444, FRAME *Y422, FRAME *Cb422, FRAME *Cr422)
{
     int x, y;
     int min0, max0, half0;
     int cb0, cr0;
     int depth;

     depth = YCbCr444->depth;
     min0 = 0;
     half0 = (1<<(YCbCr444->depth-1));
     max0  = (1<<(YCbCr444->depth))-1;
     for(y=0; y<YCbCr444->height; y++)
          for(x=0; x< YCbCr444->width; x++)
     {
          PIXEL(Y422, x, y, 0)  = PIXEL(YCbCr444,x,y,0);
          if(x%2==0)    {
               //cb0 = SROUND_BITWISE((PIXEL(YCbCr444,x,y,1) + PIXEL(YCbCr444,x+1,y,1)), 1, depth);
               //cr0 = SROUND_BITWISE((PIXEL(YCbCr444,x,y,2) + PIXEL(YCbCr444,x+1,y,2)), 1, depth);
               cb0 = PIXEL(YCbCr444,x,y,1);
               cr0 = PIXEL(YCbCr444,x,y,2);
               PIXEL(Cb422, x/2, y, 0) = cb0;
               PIXEL(Cr422, x/2, y, 0) = cr0;
          }
     }
     return 0;
}

//--------------------------------------------------------------------------------------
// YCbCr444to420: Convert YCbCr 444 to YCbCr 420
//--------------------------------------------------------------------------------------
int CM_YCbCr444to420(FRAME *YCbCr444, FRAME *Y420, FRAME *Cb420, FRAME *Cr420)
{
     int x, y;
     int min0, max0, half0;
     int cb0, cr0;
     int depth;

     depth = YCbCr444->depth;
     min0 = 0;
     half0 = (1<<(YCbCr444->depth-1));
     max0  = (1<<(YCbCr444->depth))-1;
     for(y=0; y<YCbCr444->height; y++)
          for(x=0; x< YCbCr444->width; x++)
     {
          PIXEL(Y420, x, y, 0)  = PIXEL(YCbCr444,x,y,0);
          if(x%2==0 && y%2==0)    {
               //cb0 = SROUND_BITWISE((PIXEL(YCbCr444,x,y,1) + PIXEL(YCbCr444,x+1,y,1) + PIXEL(YCbCr444,x,y+1,1) + PIXEL(YCbCr444,x+1,y+1,1)), 2, depth);
               //cr0 = SROUND_BITWISE((PIXEL(YCbCr444,x,y,2) + PIXEL(YCbCr444,x+1,y,2) + PIXEL(YCbCr444,x,y+1,2) + PIXEL(YCbCr444,x+1,y+1,2)), 2, depth);
               cb0 = PIXEL(YCbCr444,x,y,1);
               cr0 = PIXEL(YCbCr444,x,y,2);
               PIXEL(Cb420, x/2, y/2, 0) = cb0;
               PIXEL(Cr420, x/2, y/2, 0) = cr0;
          }
     }
     return 0;
}

//--------------------------------------------------------------------------------------
// YCbCr422to444: Convert YCbCr 420 to YCbCr 444
//--------------------------------------------------------------------------------------
int CM_YCbCr422to444(FRAME *Y422, FRAME *Cb422, FRAME *Cr422, FRAME *YCbCr444)
{
     int x, y;
     int cb0, cr0;
     int width, height, depth, min0, half0, max0;

     width = Y422->width;
     height = Y422->height;
     depth = Y422->depth;
     min0 = 0;
     half0 = (1<<(depth-1));
     max0  = (1<<(depth))-1;

     for(y=0; y< height; y++)
          for(x=0; x< width; x++)
     {
          PIXEL(YCbCr444, x, y, 0)  = PIXEL(Y422,x,y,0);
          if(x%2==0)    {
               cb0 = PIXEL(Cb422,x/2,y,0);
               cr0 = PIXEL(Cr422,x/2,y,0);
          } else    {
               cb0 = SROUND_BITWISE((PIXEL(Cb422,x/2,y,0) + PIXEL(Cb422,x/2+1,y,0)), 1, depth);
               cr0 = SROUND_BITWISE((PIXEL(Cr422,x/2,y,0) + PIXEL(Cr422,x/2+1,y,0)), 1, depth);
          }
          PIXEL(YCbCr444, x, y, 1) = cb0;
          PIXEL(YCbCr444, x, y, 2) = cr0;
     }

     return 0;
}

//--------------------------------------------------------------------------------------
// YCbCr420to444: Convert YCbCr 420 to YCbCr 444
//--------------------------------------------------------------------------------------
int CM_YCbCr420to444(FRAME *Y420, FRAME *Cb420, FRAME *Cr420, FRAME *YCbCr444)
{
     int x, y;
     int cb0, cr0;
     int width, height, depth, min0, half0, max0;

     width = Y420->width;
     height = Y420->height;
     depth = Y420->depth;
     min0 = 0;
     half0 = (1<<(depth-1));
     max0  = (1<<(depth))-1;

     for(y=0; y< height; y++)
          for(x=0; x< width; x++)
     {
          PIXEL(YCbCr444, x, y, 0)  = PIXEL(Y420,x,y,0);
          if(x%2==0 && y%2==0)    {
               cb0 = PIXEL(Cb420,x/2,y/2,0);
               cr0 = PIXEL(Cr420,x/2,y/2,0);
          } else if(x%2==0 && y%2==1)   {
               cb0 = SROUND_BITWISE((PIXEL(Cb420,x/2,y/2,0) + PIXEL(Cb420,x/2,y/2+1,0)), 1, depth);
               cr0 = SROUND_BITWISE((PIXEL(Cr420,x/2,y/2,0) + PIXEL(Cr420,x/2,y/2+1,0)), 1, depth);
          } else if(x%2==1 && y%2==0)   {
               cb0 = SROUND_BITWISE((PIXEL(Cb420,x/2,y/2,0) + PIXEL(Cb420,x/2+1,y/2,0)), 1, depth);
               cr0 = SROUND_BITWISE((PIXEL(Cr420,x/2,y/2,0) + PIXEL(Cr420,x/2+1,y/2,0)), 1, depth);
          } else if(x%2==1 && y%2==1)   {
               cb0 = SROUND_BITWISE((PIXEL(Cb420,x/2,y/2,0) + PIXEL(Cb420,x/2+1,y/2,0) + PIXEL(Cb420,x/2,y/2+1,0) + PIXEL(Cb420,x/2+1,y/2+1,0)), 2, depth);
               cr0 = SROUND_BITWISE((PIXEL(Cr420,x/2,y/2,0) + PIXEL(Cr420,x/2+1,y/2,0) + PIXEL(Cr420,x/2,y/2+1,0) + PIXEL(Cr420,x/2+1,y/2+1,0)), 2, depth);
          }
          PIXEL(YCbCr444, x, y, 1) = cb0;
          PIXEL(YCbCr444, x, y, 2) = cr0;
     }

     return 0;
}

//--------------------------------------------------------------------------------------
// YCbCr2RGB: Convert YCbCr to RGB
//--------------------------------------------------------------------------------------
FRAME *CM_YCbCr2RGB(FRAME *img)
{
     int x, y;
     float r0, g0, b0;
     float y0, cb0, cr0;
     FRAME *new_img;

     if(img->format != YCBCR)   {    printf("YCbCr2RGB format error\n");     exit(-1); }

     new_img = CM_NewFRAME(img->type,RGB, img->width,img->height,img->channel,img->depth);
     for(y=0; y< img->height; y++)
          for(x=0; x< img->width; x++)
     {
          y0  = (float) PIXEL(img, x, y, Y );
          cb0 = (float) PIXEL(img, x, y, CB);
          cr0 = (float) PIXEL(img, x, y, CR);
          r0  = y0 + 1.371 * (cr0 - 128.0);
          g0  = y0 - 0.6982 * (cr0 - 128.0) - 0.3365 * (cb0 - 128.0);
          b0  = y0 + 1.732 * (cb0 - 128.0);
          PIXEL(new_img, x, y, R) = (int) MINMAX(UROUND(r0), 0, 255);
          PIXEL(new_img, x, y, G) = (int) MINMAX(UROUND(g0), 0, 255);
          PIXEL(new_img, x, y, B) = (int) MINMAX(UROUND(b0), 0, 255);
     }
     //CM_DelFRAME(img);

     return new_img;
}

//--------------------------------------------------------------------------------------
// GetField: Get TOP or BOTTOM field from a frame
//--------------------------------------------------------------------------------------
FRAME *CM_GetField(FRAME *img, FRAME_TYPE field)
{
     int x, y, c, new_h, offset;
     FRAME *new_img;

     new_h  = img->height/2;
     offset = (field == TOP_FIELD)? 0:1;
     new_img = CM_NewFRAME(field, img->format, img->width,new_h,img->channel,img->depth);

     for(y=0; y< new_h; y++)
          for(x=0; x<img->width; x++)
               for(c=0; c<img->channel; c++)
     {
          PIXEL(new_img, x, y, c) = PIXEL(img, x, 2*y+offset, c);
     }
     return new_img;
}

//--------------------------------------------------------------------------------------
// SetField:
//--------------------------------------------------------------------------------------
void CM_SetField(FRAME* img_di, FRAME *img, FRAME_TYPE field)
{
     int x, y, c, new_h, offset;
     new_h  = img->height;
     offset = (field == TOP_FIELD)? 0:1;

     for(y=0; y< new_h; y++)
          for(x=0; x<img->width; x++)
               for(c=0; c<img->channel; c++)
     {
          PIXEL(img_di, x, 2*y+offset, c) = PIXEL(img, x, y, c);
     }
     //return img_di;
}
//--------------------------------------------------------------------------------------
// LUT3: Gain Look Up according to [in_th_left, in_offset0], slope, and weight_r
//--------------------------------------------------------------------------------------
/**
 * @brief the linear interpolation (alpha blending) for 3 segment lines
 *
 * @param lut_in the input value
 * @param in_th_left the left threshold value
 * @param weight_l the output weighting for the left node
 * @param weight_r the output weighting for the right node
 * @param slope the slope between in_th_left and in_th_right
 * @param slope_shift slope shift bit, e.g. s8.10 => 10
 * @return int, the output weighting for the lut_in
 */
int CM_LUT3(int lut_in, int in_th_left, int weight_l, int weight_r, int slope, int slope_shift)
{
     int lut_out;

     if(lut_in <= in_th_left) {
          lut_out = weight_l;
     } else {
          int delta = UROUND_BITWISE((lut_in - in_th_left) * slope, slope_shift);
		lut_out = weight_l + delta;

          if(weight_l <= weight_r) {
               lut_out = MIN(lut_out, weight_r);
          } else {
               lut_out = MAX(lut_out, weight_r);
          }
	}
     return lut_out;
}

//--------------------------------------------------------------------------------------
// BlendIIR: blending A and B with Alpha and Shift. Favor A if Abs(A-B)==1;
//--------------------------------------------------------------------------------------
int CM_BlendIIR(int A, int B, int Alpha, int Shift)
{
     int delta = A-B;
     int add_on = (delta>0)? 1:0;     // delta >0

     return ((Alpha*A + ((1<<Shift) - Alpha)*B)>>Shift) + add_on;
}
//--------------------------------------------------------------------------------------
// BlendIIR_C: blending chroma A and chroma B with Alpha and Shift. Favor A if Abs(A-B)==1;
//--------------------------------------------------------------------------------------
int CM_BlendIIR_C(int A, int B, int Alpha, int Shift, int AB_Depth)
{
    int delta = A-B;
    int add_on = (delta>0)? 1:0;     // delta >0

    int iOut = (Alpha*A + ((1<<Shift) - Alpha)*B) + (add_on << Shift);  // TODO: double check add_on is needed

    int sign = (iOut >= (1 << (AB_Depth - 1 + Shift))) ? 1 : -1;
    int abs_val = ABS(iOut - (1 << (AB_Depth - 1 + Shift)));
    abs_val = (abs_val + (1 << (Shift - 1))) >> Shift;
    iOut = sign * abs_val + (1 << (AB_Depth - 1));

    return iOut;
}
//--------------------------------------------------------------------------------------
// BlendR: blending A and B with Alpha and Shift. Rounding by adding 1<<(Shift-1);
//--------------------------------------------------------------------------------------
int CM_BlendR(int A, int B, int Alpha, int Shift)
{
     return (Alpha*A + ((1<<Shift) - Alpha)*B + (1<<(Shift-1)))>>Shift;
}

//--------------------------------------------------------------------------------------
// ReadRAW: read a raw file by frame index and LE/SE information
// type:  RAW_RGB or RAW_RGBIR
// fidx:  frame index, start from 0
// mode:  0: sdr file
//        1: hdr file, and fetch LE
//        2: hdr file, and fetch SE
//--------------------------------------------------------------------------------------
FRAME *CM_ReadRAW(const char *filename, COLOR_FORMAT format, int fidx, int hdr_mode, int width, int height, int depth)
{
     FRAME *img;
     FILE  *fp;

     //open RAW file for reading
     fp = fopen(filename, "rb");
     if (!fp) {
          fprintf(stderr, "Unable to open file '%s'\n", filename);
          exit(1);
     }

     img = CM_NewFRAME(RAW, format, width, height, 1, depth);

     // seek the file offset
     int hdr = (hdr_mode!=0)? 1:0;
     int se  = (hdr_mode==2)? 1:0;
     long offset = ((hdr+1)*width*height*fidx + se*width)*sizeof(unsigned short);

     int x, y, byte0, byte1;
     for(y=0; y<height; y++)  {
          fseek(fp, offset, SEEK_SET);
          for(x=0; x<width; x++)   {
               byte0 = fgetc(fp);
               byte1 = fgetc(fp);
               PIXEL(img, x, y, 0) = (byte1 << 8) + byte0;
          }
          offset += (hdr+1)*width*sizeof(unsigned short);
     }

     fclose(fp);
     return img;
}

//--------------------------------------------------------------------------------------
// WriteRAW: write a RAW file in binary format
//--------------------------------------------------------------------------------------
int CM_WriteRAW(const char *filename, FRAME *raw)
{
     FILE *fp;
     int x, y;
     unsigned char byte0, byte1;

     if(raw->format != RAW)   {
          printf("File format != RAW\n");
          exit(1);
     }

     //open RAW file for writting
     fp = fopen(filename, "wb");
     if (!fp) {
          fprintf(stderr, "Unable to open file '%s'\n", filename);
          exit(1);
     }

     for(y=0; y<raw->height; y++)
          for(x=0; x<raw->width; x++)
     {
          byte0 = PIXEL(raw, x, y, 0) & 0xFF;
          byte1 = (PIXEL(raw, x, y, 0)>>8) & 0xFF;
          fputc(byte0, fp);
          fputc(byte1, fp);
     }

     fclose(fp);
     return 0;
}

//--------------------------------------------------------------------------------------
// RGB2PPM: Write a RGB frame to a PPM file
//--------------------------------------------------------------------------------------
int CM_RGB2PPM(const char *filename, FRAME *rgb, int target_depth)
{
     FILE *fp;
     int x, y;

     // check format
     if(rgb->format != RGB)   {
          printf("(RGB2PPM) File format != RGB\n");
          exit(1);
     }

     // open PPM file for writting
     fp = fopen(filename, "wb");
     if (!fp) {
          fprintf(stderr, "(RGB2PPM) Unable to open file '%s'\n", filename);
          exit(1);
     }

     /* ---------------------------------------------------------------------------------------------- */
     /* write the header file                                                                        */
     /* ---------------------------------------------------------------------------------------------- */
     // image format
     fprintf(fp, "P6\n");

     // comments
     fprintf(fp, "# Created by CVITEK DPU Model\n");

     // image size
     fprintf(fp, "%d %d\n",rgb->width,rgb->height);

     // rgb component depth
     int max_val = (1<<target_depth)-1;
     fprintf(fp, "%d\n", max_val);

     if(target_depth != 8) {printf("(RGB2PPM) Origianl Depth=%d\tTarget Depth=%d MAXRGB=%d\n",rgb->depth, target_depth, max_val);}

     int r0, g0, b0;
     for(y=0; y<rgb->height; y++) {
          for(x=0; x<rgb->width; x++) {
               r0 = MINMAX((PIXEL(rgb, x, y, R) >> (rgb->depth-target_depth)), 0, max_val);
               g0 = MINMAX((PIXEL(rgb, x, y, G) >> (rgb->depth-target_depth)), 0, max_val);
               b0 = MINMAX((PIXEL(rgb, x, y, B) >> (rgb->depth-target_depth)), 0, max_val);
               // if (y==0) printf("test write: %d\n", r0);
               if(target_depth > 24) {fputc((unsigned char) ((r0 & 0xFF000000)>>24), fp);}
               if(target_depth > 16) {fputc((unsigned char) ((r0 & 0xFF0000)>>16), fp);}
               if(target_depth > 8)  {fputc((unsigned char) ((r0 & 0xFF00)>>8), fp);}
               fputc((unsigned char) (r0 & 0xFF), fp);

               if(target_depth > 24) {fputc((unsigned char) ((g0 & 0xFF000000)>>24), fp);}
               if(target_depth > 16) {fputc((unsigned char) ((g0 & 0xFF0000)>>16), fp);}
               if(target_depth > 8)  {fputc((unsigned char) ((g0 & 0xFF00)>>8), fp);}
               fputc((unsigned char) (g0 & 0xFF), fp);

               if(target_depth > 24) {fputc((unsigned char) ((b0 & 0xFF000000)>>24), fp);}
               if(target_depth > 16) {fputc((unsigned char) ((b0 & 0xFF0000)>>16), fp);}
               if(target_depth > 8)  {fputc((unsigned char) ((b0 & 0xFF00)>>8), fp);}
               fputc((unsigned char) (b0 & 0xFF), fp);
          }
     }
     fclose(fp);

     return 0;
}

//--------------------------------------------------------------------------------------
// RAW2PPM6: Write a RAW frame to a PPM file with independent R/G/B channels
//--------------------------------------------------------------------------------------
int CM_RAW2PPM6(const char *filename, FRAME *raw, int target_depth)
{
     FILE *fp;
     int x, y, bayer_id;

     if(raw->type != RAW)   {
          printf("(RAW2PPM6) File type != RAW\n");
          exit(1);
     }

     if(raw->format != RAW_RGB)   {
          printf("(RAW2PPM6) File format != RAW_RGB\n");
          exit(1);
     }

     //open PPM file for writting
     fp = fopen(filename, "wb");
     if (!fp) {
          fprintf(stderr, "(RAW2PPM6) Unable to open file '%s'\n", filename);
          exit(1);
     }

     //write the header file
     //image format
     fprintf(fp, "P6\n");

     //comments
     fprintf(fp, "# Created by CVITEK DPU Model\n");

     //image size
     fprintf(fp, "%d %d\n",raw->width,raw->height);

     // rgb component depth
     int max_val = (1<<target_depth)-1;
     fprintf(fp, "%d\n", max_val);

     int x_offset0, y_offset0, x_offset, y_offset, offset;
     switch(raw->bayer_id)    {
          case BAYER_BG: x_offset0 = 0; y_offset0 = 0; break;
          case BAYER_GB: x_offset0 = 1; y_offset0 = 0; break;
          case BAYER_GR: x_offset0 = 0; y_offset0 = 1; break;
          case BAYER_RG: x_offset0 = 1; y_offset0 = 1; break;
          default:       x_offset0 = 0; break;
     }
     int r0, g0, b0;
     for(y=0; y<raw->height; y++)
          for(x=0; x<raw->width; x++)
     {
          x_offset = (x + x_offset0) % 2;
          y_offset = (y + y_offset0) % 2;
          offset = (y_offset << 1) + x_offset;
          r0 = g0 = b0 = 0;
          switch(offset)  {
               case 0:   b0 = MINMAX(PIXEL(raw, x, y, 0) >> (raw->depth - target_depth), 0, max_val);
                         break;
               case 1:   g0 = MINMAX(PIXEL(raw, x, y, 0) >> (raw->depth - target_depth), 0, max_val);
                         break;
               case 2:   g0 = MINMAX(PIXEL(raw, x, y, 0) >> (raw->depth - target_depth), 0, max_val);
                         break;
               case 3:   r0 = MINMAX(PIXEL(raw, x, y, 0) >> (raw->depth - target_depth), 0, max_val);
                         break;
               default:  break;
          }
          fputc((unsigned char) (r0 & 0xFF), fp);
          if(target_depth>8)  fputc((unsigned char) ((r0 & 0xFF00)>>8), fp);
          if(target_depth>16) {
               fputc((unsigned char) ((r0 & 0xFF0000)>>16), fp);
               fputc((unsigned char) ((r0 & 0xFF000000)>>24), fp);
          }
          fputc((unsigned char) (g0 & 0xFF), fp);
          if(target_depth>8)  fputc((unsigned char) ((g0 & 0xFF00)>>8), fp);
          if(target_depth>16) {
               fputc((unsigned char) ((g0 & 0xFF0000)>>16), fp);
               fputc((unsigned char) ((g0 & 0xFF000000)>>24), fp);
          }
          fputc((unsigned char) (b0 & 0xFF), fp);
          if(target_depth>8)  fputc((unsigned char) ((b0 & 0xFF00)>>8), fp);
          if(target_depth>16) {
               fputc((unsigned char) ((b0 & 0xFF0000)>>16), fp);
               fputc((unsigned char) ((b0 & 0xFF000000)>>24), fp);
          }
     }

     fclose(fp);
     return 0;
}

//--------------------------------------------------------------------------------------
// RGB2PPM5: Write a RAW frame to a Gray PPM file
//--------------------------------------------------------------------------------------
int CM_RAW2PPM5(const char *filename, FRAME *raw, int target_depth)
{
     FILE *fp;
     int x, y, bayer_id;

     if(raw->channel != 1)   {
          printf("(RAW2PPM5) Color channel > 1\n");
          exit(1);
     }

     //open PPM file for writting
     fp = fopen(filename, "wb");
     if (!fp) {
          fprintf(stderr, "(RAW2PPM5) Unable to open file '%s'\n", filename);
          exit(1);
     }

     //write the header file
     //image format
     fprintf(fp, "P5\n");

     //comments
     fprintf(fp, "# Created by CVITEK DPU Model\n");

     //image size
     fprintf(fp, "%d %d\n",raw->width,raw->height);

     // rgb component depth
     int max_val = (1<<target_depth)-1;
     fprintf(fp, "%d\n", max_val);

     int p0;
     for(y=0; y<raw->height; y++)
          for(x=0; x<raw->width; x++)
     {
          p0 = MINMAX(PIXEL(raw, x, y, 0) >> (raw->depth - target_depth), 0, max_val);
		  if (target_depth>24)	fputc((unsigned char) ((p0 & 0xFF000000)>>24), fp);
		  if (target_depth>16)	fputc((unsigned char) ((p0 & 0xFF0000)>>16), fp);
		  if (target_depth>8)	fputc((unsigned char) ((p0 & 0xFF00)>>8), fp);
		  fputc((unsigned char) (p0 & 0xFF), fp);
//          fputc((unsigned char) (p0 & 0xFF), fp);
//          if(target_depth>8)  fputc((unsigned char) ((p0 & 0xFF00)>>8), fp);
//          if(target_depth>16) {
//               fputc((unsigned char) ((p0 & 0xFF0000)>>16), fp);
//               fputc((unsigned char) ((p0 & 0xFF000000)>>24), fp);
//          }
     }

     fclose(fp);
     return 0;
}

#if 0
void Raw2PPMP5(AlgoSimConf& conf, char *fname, Mat &data) {
    Mat img_data = data.clone();

    short* src = img_data.ptr<short>(0);
    int img_height = img_data.rows;
    int img_width = img_data.cols;

    FILE* fp = NULL;

    if ((fp = fopen(fname, "wb")) == NULL) {
        perror("Fail to open raw file...");
        exit(1);
    }
    else {
        // header
        fprintf(fp, "P5\n");
        fprintf(fp, "#%s\n", conf.f_info->FileName.c_str());
        fprintf(fp, "%d %d\n", img_data.cols, img_data.rows);
        fprintf(fp, "%d\n", (1<<conf.f_info->i4Bits)-1);

		typedef union {
            unsigned short PixVal;
            unsigned char Byte[2];
		} PixLayout;


        for (int i = 0; i < img_height * img_width; i++) {
            PixLayout val = { *(src + i) };
            fwrite(&val.Byte[1], 1, 1, fp);
            fwrite(&val.Byte[0], 1, 1, fp);
        }
        fclose(fp);
    }
}
#endif

//--------------------------------------------------------------------------------------
// NewHIST: create a new histogram structure
//--------------------------------------------------------------------------------------
HIST *CM_NewHIST(int nbins, int depth, int channel)
{
     HIST *hist;

     //alloc memory form image
     hist = (HIST *)malloc(sizeof(HIST));
     if (!hist) {
          fprintf(stderr, "(NewHIST) Unable to allocate HIST memory\n");
          exit(1);
     }
     //memory allocation for histogram data
     hist->nbins = nbins;
     hist->depth = depth;
     hist->channel = channel;
     hist->data  = (int *)calloc(nbins * channel, sizeof(int));
     if (!hist->data) {
          fprintf(stderr, "(NewHIST) Unable to allocate data memory\n");
          exit(1);
     }
     return hist;
}

//--------------------------------------------------------------------------------------
// DelHIST: delete a histogram structure
//--------------------------------------------------------------------------------------
int CM_DelHIST(HIST *hist)
{
    free(hist->data);
    free(hist);
    return 0;
}

//--------------------------------------------------------------------------------------
// WriteHIST: write HIST file
//--------------------------------------------------------------------------------------
int CM_WriteHIST(const char *filename, HIST *hist)
{
     FILE *fp;

     // pixel data
     //unsigned char c;
     char *ch , *str_concated;
     unsigned int c ;
     int i_ch;
     int i;

     for(i_ch =0; i_ch < hist->channel ; i_ch ++){
         switch (i_ch){
         case HIST_RAW_R   : ch = "_R"  ; break;
         case HIST_RAW_G   : ch = "_G"  ; break;
         case HIST_RAW_B   : ch = "_B"  ; break;
         case HIST_RGB_V   : ch = "_V"  ; break;
         case HIST_RGB_Y   : ch = "_Y"  ; break;
         case HIST_RAW_IR  : ch = "_IR" ; break;
         default           : ch = "_"  ; break;
         }

         str_concated = CM_StringConcat( (char*)filename,ch);
         printf("[CM_WriteHIST] file_name :%s\n",str_concated);

         //open file for output
         fp = fopen(str_concated, "wb");
         if (!fp) {
              fprintf(stderr, "Unable to open file '%s'\n",str_concated );
              exit(1);
         }

        for(i=0; i < hist->nbins; i++)
        {
          c = (unsigned int) HIST_P(hist , i , i_ch);
          fprintf( fp, "%08x\n",c);
        }
        fclose(fp);
     }

     return 0;
}


//--------------------------------------------------------------------------------------
// Get color ID
//--------------------------------------------------------------------------------------
int CM_GET_COLOR_ID(int row, int col, int id) {


	BAYER_ID ColorPattern[16][16] = {
			{BAYER_ID_BG, BAYER_ID_GB, BAYER_ID_GR, BAYER_ID_RG}, // id = 0
			{BAYER_ID_GB, BAYER_ID_BG, BAYER_ID_RG, BAYER_ID_GR}, // id = 1
			{BAYER_ID_GR, BAYER_ID_RG, BAYER_ID_BG, BAYER_ID_GB}, // id = 2
			{BAYER_ID_RG, BAYER_ID_GR, BAYER_ID_GB, BAYER_ID_BG}, // id = 3

			{BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY},
			{BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY},
			{BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY},
			{BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY},

			{BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY,
			 BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY,
			 BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY,
			 BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY}, // id = 8

			{BAYER_ID_RG, BAYER_ID_GR, BAYER_ID_BG, BAYER_ID_GR,
			 BAYER_ID_GR, BAYER_ID_IR, BAYER_ID_GR, BAYER_ID_IR,
			 BAYER_ID_BG, BAYER_ID_GR, BAYER_ID_RG, BAYER_ID_GR,
			 BAYER_ID_GR, BAYER_ID_IR, BAYER_ID_GR, BAYER_ID_IR}, // id = 9

			{BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY,
			 BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY,
			 BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY,
			 BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY}, // id = 10

			{BAYER_ID_BG, BAYER_ID_GR, BAYER_ID_RG, BAYER_ID_GR,
			 BAYER_ID_GR, BAYER_ID_IR, BAYER_ID_GR, BAYER_ID_IR,
			 BAYER_ID_RG, BAYER_ID_GR, BAYER_ID_BG, BAYER_ID_GR,
			 BAYER_ID_GR, BAYER_ID_IR, BAYER_ID_GR, BAYER_ID_IR}, // id = 11

			{BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY,
			 BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY,
			 BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY,
			 BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY}, // id = 12

			{BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY,
			 BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY,
			 BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY,
			 BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY}, // id = 13

			{BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY,
			 BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY,
			 BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY,
			 BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY}, // id = 14

			{BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY,
			 BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY,
			 BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY,
			 BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY, BAYER_ID_DUMMY}, // id = 15
	};

	if (id < 4) {

		return
			((((row & 0x01) == 0) && ((col & 0x01) == 0)) ? (ColorPattern[id][0]) :
			((((row & 0x01) == 0) && ((col & 0x01) == 1)) ? (ColorPattern[id][1]) :
			((((row & 0x01) == 1) && ((col & 0x01) == 0)) ? (ColorPattern[id][2]) :
															((ColorPattern[id][3])))));
	}
	else {

		return
			((((row & 0x03) == 0) && ((col & 0x03) == 0)) ? (ColorPattern[id][0]) :
			((((row & 0x03) == 0) && ((col & 0x03) == 1)) ? (ColorPattern[id][1]) :
			((((row & 0x03) == 0) && ((col & 0x03) == 2)) ? (ColorPattern[id][2]) :
			((((row & 0x03) == 0) && ((col & 0x03) == 3)) ? (ColorPattern[id][3]) :

			((((row & 0x03) == 1) && ((col & 0x03) == 0)) ? (ColorPattern[id][4]) :
			((((row & 0x03) == 1) && ((col & 0x03) == 1)) ? (ColorPattern[id][5]) :
			((((row & 0x03) == 1) && ((col & 0x03) == 2)) ? (ColorPattern[id][6]) :
			((((row & 0x03) == 1) && ((col & 0x03) == 3)) ? (ColorPattern[id][7]) :

			((((row & 0x03) == 2) && ((col & 0x03) == 0)) ? (ColorPattern[id][8]) :
			((((row & 0x03) == 2) && ((col & 0x03) == 1)) ? (ColorPattern[id][9]) :
			((((row & 0x03) == 2) && ((col & 0x03) == 2)) ? (ColorPattern[id][10]) :
			((((row & 0x03) == 2) && ((col & 0x03) == 3)) ? (ColorPattern[id][11]) :

			((((row & 0x03) == 3) && ((col & 0x03) == 0)) ? (ColorPattern[id][12]) :
			((((row & 0x03) == 3) && ((col & 0x03) == 1)) ? (ColorPattern[id][13]) :
			((((row & 0x03) == 3) && ((col & 0x03) == 2)) ? (ColorPattern[id][14]) :
			                                                ((ColorPattern[id][15])))))))))))))))));
	}
}


//--------------------------------------------------------------------------------------
// String concat
//--------------------------------------------------------------------------------------
char *CM_StringConcat(char *str1, char *str2) {
   int length=strlen(str1)+strlen(str2)+1;
   char *result = (char*)malloc(sizeof(char) * length);
   strcpy(result, str1);
   strcat(result, str2);
   return result;
}

//
//
//
int CM_interp1(int in_val, int x0, int y0, int y1, int slope, int sft) {
    int lut_out, delta, rnd_val;
    int dir;

    dir = (y0 <= y1) ? 1:0;      // dir==1, LUT value from low to high

    if(in_val <= x0) lut_out = y0;
    else {
        rnd_val = (1<<(sft-1));
        delta = ((in_val-x0)*slope + rnd_val) >> sft;
        lut_out = y0 + delta;
        if(dir==1)
            lut_out = (lut_out > y1) ? y1 : lut_out;
        else
            lut_out = (lut_out < y1) ? y1 : lut_out;
    }
    return lut_out;
}

/**
 * @brief linear interpolation for multi-segment
 *
 * @param in_val input value
 * @param lut_in input threshold array
 * @param lut_out output weighting array
 * @param lut_slope slope array
 * @param lut_num the size of lut_in and lut_out
 * @param sft_bit slope shift bit, e.g. s8.10 => 10
 * @return int, the output weighting corresponding the in_val
 */
int CM_LUT(int in_val, int *lut_in, int *lut_out, int *lut_slope, int lut_num, int sft_bit)
{
    int out_val;
    int alpha;
    int i;

    i = 0;
    if (in_val <= lut_in[0])
    {
        out_val = lut_out[0];
    }
    else if (in_val > lut_in[lut_num - 1])
    {
        out_val = lut_out[lut_num - 1];
    }
    else
    {
          while(in_val > lut_in[i]) i++;
#if 1
          alpha = lut_out[i-1] + ((lut_slope[i-1] * (in_val - lut_in[i-1])) >> sft_bit);
          out_val = MEDIAN(lut_out[i-1], lut_out[i], alpha);
#else
               out_val = CM_interp1(in_val, lut_in[i-1], lut_out[i-1], lut_out[i], lut_slope[i-1], sft_bit);
#endif
     }

     return out_val;
}

int CM_SingleIntp(int x0, int x1, int y0, int y1, int x)
{
	if (x <= MIN(x0, x1))
		return MIN(x0,x1)==x0?y0:y1;
	if (x >= MAX(x0, x1))
		return MAX(x0,x1)==x0?y0:y1;

	int ratio = 0;
	if (x0 != x1)
		ratio = y0 + (int)(x - x0) * (y1 - y0) / (int)(x1 - x0);
	else
		ratio = y0;

	return ratio;
}

int CM_MultiIntp(int *x, int *y, int len, int p) {
	if (len <= 0)
		return 0;
	if (p <= x[0])
		return y[0];
	if (p >= x[len - 1])
		return y[len - 1];

	for (int i = 0; i < len-1; i++) {
		if (p >= x[i] && p < x[i + 1])
			return CM_SingleIntp(x[i], x[i+1], y[i], y[i+1], p);
	}

	return 0;
}


void CM_PPM6toPPM3(const char *filename, const char *filename_w, FRAME_TYPE type, char *dbg_path, const int frame_idx)
{
     char  buff[16], c;
     FRAME *img;
     FILE  *fp;
     int width, height, max_rgb;

     // open PPM file for reading
     fp = fopen(filename, "rb");
     if (!fp) {
          fprintf(stderr, "Unable to open file '%s'\n", filename);
          exit(1);
     }

     // read image format
     if (!fgets(buff, sizeof(buff), fp)) {
          perror(filename);
          exit(1);
     }

     // check the image format
     if (buff[0] != 'P' || buff[1] != '6') {
          fprintf(stderr, "Invalid image format (must be 'P6') (error loading '%s')\n", filename);
          exit(1);
     }

     // check for comments
     c = getc(fp);
     while (c == '#') {
     while (getc(fp) != '\n') ;
          c = getc(fp);
     }

     ungetc(c, fp);
     // read image size information
     if (fscanf(fp, "%d %d", &width, &height) != 2) {
          fprintf(stderr, "Invalid image size (error loading '%s')\n", filename);
          exit(1);
     }

     // read max_rgb
     if (fscanf(fp, "%d", &max_rgb) != 1) {
          fprintf(stderr, "Invalid max_rgb (error loading '%s')\n", filename);
          exit(1);
     }
     while (fgetc(fp) != '\n') ;

     // check max_rgb depth
     int depth = (int) ceil(log2((double) max_rgb));
     printf("Color depth (%d) max_rgb:(%d)\n", depth, max_rgb);

     // new frame
     int channel = 3;
     img = CM_NewFRAME(type,RGB, width, height, channel, depth);
     if (!img) {
          fprintf(stderr, "Unable to allocate memory\n");
          exit(1);
     }

     int x, y;
     int byte0 = 0;
     int byte1 = 0;
     int byte2 = 0;
     int byte3 = 0;
     for(y=0; y<height; y++) {
          for(x=0; x<width; x++) {
               for(c=0; c<3; c++) {
                    if(depth > 24) {byte3 = fgetc(fp);}
                    if(depth > 16) {byte2 = fgetc(fp);}
                    if(depth > 8) {byte1 = fgetc(fp);}
                    if(depth > 0) {byte0 = fgetc(fp);}
                    PIXEL(img,x,y,c) = (int)((byte3<<24)+(byte2<<16)+(byte1<<8)+(byte0));
                    if(feof(fp)) break;
               }
          }
     }
     fclose(fp);

     printf("read_frame_done\n");

// write p3
     FILE *fp_w;
     int i;

     char filename_wo[100];
     sprintf(filename_wo, "%s%s_%03d.ppm", dbg_path, filename_w, frame_idx);

     //open file for output
     fp_w = fopen(filename_wo, "wb");
     if (!fp_w) {
          fprintf(stderr, "Unable to open file '%s'\n", filename_wo);
          exit(1);
     }

     //write the header file
     //image format
     fprintf(fp_w, "P3\n");

     //image size
     fprintf(fp_w, "%d %d\n",img->width,img->height);

     // rgb component depth
     fprintf(fp_w, "%d\n", (1<<img->depth)-1);

     // pixel data
     for(y=0; y < img->height; y++)
          for(x=0; x < img->width; x++)
     {
          int iR, iG, iB;
          if(img->channel == 3)
          {
               iR = PIXEL(img, x, y, 0);
               iG = PIXEL(img, x, y, 1);
               iB = PIXEL(img, x, y, 2);
          } else
          {
               iR = PIXEL(img, x, y, 0);
               iG = PIXEL(img, x, y, 0);
               iB = PIXEL(img, x, y, 0);
          }
//	  fprintf(fp_w, "%d  %d  %d // %d %d\n", iR, iG, iB, x, y);
	  fprintf(fp_w, "%d  %d  %d\n", iR, iG, iB);

     }
     fclose(fp_w);

//     return 0;
}

/**
 * @brief Merge three one-channel images to one 3-chennel image
 * @param [in] LUT     Lookup table (odd number of bins)
 * @param [in] x0      Input value x0
 * @param [in] nbits_t #Bits of input value x0
 * @param [in] nbits_m #Bits of MSB of input value x0
 */
int CM_LutBilinearInterp(int* LUT, int x0, int nbits_t, int nbits_m)
{
	int nbits_n = nbits_t - nbits_m;
	int MASK_m = (1 << nbits_m) - 1;
	int MASK_n = (1 << nbits_n) - 1;

	int idx = ((x0 >> nbits_n) & MASK_m);
	int idx_next = MIN(idx + 1, MASK_m + 1);
	int ww = (x0 & MASK_n);

	int weight = (LUT[idx] * ((1 << nbits_n) - ww) + LUT[idx_next] * ww + (1 << (nbits_n - 1))) >> nbits_n;
	return weight;
}

int CM_WritePPM3(const char *filename, FRAME *img, char *dbg_path, const int frame_idx)
{
     FILE *fp_w;
     int i;

     char filename_wo[100];
     sprintf(filename_wo, "%s%s_%03d.ppm", dbg_path, filename, frame_idx);

     //open file for output
     fp_w = fopen(filename_wo, "wb");
     if (!fp_w) {
          fprintf(stderr, "Unable to open file '%s'\n", filename_wo);
          exit(1);
     }

     //write the header file
     //image format
     fprintf(fp_w, "P3\n");

     //image size
     fprintf(fp_w, "%d %d\n",img->width,img->height);

     // rgb component depth
     fprintf(fp_w, "%d\n", (1<<img->depth)-1);
     
     int x, y;

     // pixel data
     for(y=0; y < img->height; y++)
          for(x=0; x < img->width; x++)
     {
          int iR, iG, iB;
          if(img->channel == 3)
          {
               iR = PIXEL(img, x, y, 0);
               iG = PIXEL(img, x, y, 1);
               iB = PIXEL(img, x, y, 2);
          } else
          {
               iR = PIXEL(img, x, y, 0);
               iG = PIXEL(img, x, y, 0);
               iB = PIXEL(img, x, y, 0);
          }
	  fprintf(fp_w, "%x  %x  %x\n", iR, iG, iB);

     }
     fclose(fp_w);
     return 0;


}



void	Frame_Set_Constant(FRAME *frame_in, int constant)
{
	int width = frame_in->width;
	int height = frame_in->height;
	int channel = frame_in->channel;

	int x,y,z;
	for(z=0; z<channel; z++)
		for(y=0; y<height; y++)
			for(x=0; x<width; x++)
			{
			 	PIXEL(frame_in, x, y, z) = constant;
				//printf ("pxl=%d, x=%d, y=%d, ch=%d\n", PIXEL(frame_in, x, y, z),x,y,z);

			}
}
