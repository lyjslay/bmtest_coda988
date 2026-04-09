#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE (4 << 20)
#define VPPALIGN(x, mask)  (((x) + ((mask)-1)) & ~((mask)-1))
enum VPP_OUTPUT_FORMAT {
  OUT_YUV400P = 0,
  OUT_YUV420P = 1,
  OUT_YUV444P = 3,
  OUT_NV12 = 4,
  OUT_NV21 = 5,
  OUT_RGBYP = 16,
  OUT_RGBP = 17,
  OUT_RGB24 = 18,
  OUT_BGR24 = 19,
  OUT_HSV180 = 20,
  OUT_HSV256 = 21,
  OUT_FORMAT_MAX = 22
};


int diff_yuv444p(unsigned char *d1,unsigned char *d2)
{
  int out_axis_x = atoi(getenv("dst_crop_st_x"));
  int out_axis_y = atoi(getenv("dst_crop_st_y"));
  int out_crop_w = atoi(getenv("dst_crop_sz_w"));
  int out_crop_h = atoi(getenv("dst_crop_sz_h"));
  int dst_stride_ch0 = 0,dst_stride_ch1 = 0,dst_stride_ch2 = 0,dst_stride_ch3 = 0;
  int err = 0;
  int offset_c = 0, offset_f = 0;

  dst_stride_ch0 = out_axis_x + out_crop_w;
  dst_stride_ch1 = dst_stride_ch0;
  dst_stride_ch2 = dst_stride_ch0;
  dst_stride_ch3 = 0;


  for (int i = 0; i < out_crop_h; i++)
  {
    for (int j = 0; j < out_crop_w; j++)
    {
      int n3 = (out_crop_w * i + j) * 1;
      int m3 = (out_axis_y + i) * dst_stride_ch0 + (out_axis_x + j) * 1;

      if (d1[n3] != d2[m3])
      {
        printf("p1 diff: [%dx%d] %d:%d: %d:%d\n", i, j, n3, m3, d1[n3], d2[m3]);
        err++;
      }
    }
  }
  offset_c = out_crop_h * out_crop_w;
  offset_f = (out_axis_y + out_crop_h) * dst_stride_ch0;

  for (int i = 0; i < out_crop_h; i++)
  {
    for (int j = 0; j < out_crop_w; j++)
    {
      int n3 = (out_crop_w * i + j) * 1 + offset_c;
      int m3 = (out_axis_y + i) * dst_stride_ch0 + (out_axis_x + j) * 1 + offset_f;

      if (d1[n3] != d2[m3])
      {
        printf("p2 diff: [%dx%d] %d:%d: %d:%d\n", i, j, n3, m3, d1[n3], d2[m3]);
        err++;
      }
    }
  }
  offset_c = out_crop_h * out_crop_w * 2;
  offset_f = (out_axis_y + out_crop_h) * dst_stride_ch0 *2 ;
  for (int i = 0; i < out_crop_h; i++)
  {
    for (int j = 0; j < out_crop_w; j++)
    {
      int n3 = (out_crop_w * i + j) * 1 + offset_c;
      int m3 = (out_axis_y + i) * dst_stride_ch0 + (out_axis_x + j) * 1 + offset_f;

      if (d1[n3] != d2[m3])
      {
        printf("p3 diff: [%dx%d] %d:%d: %d:%d\n", i, j, n3, m3, d1[n3], d2[m3]);
        err++;
      }
    }
  }

  printf("format yuv444p, rgbp!\n");

  return err; 
}
int diff_yuv400p(unsigned char *d1,unsigned char *d2)
{
  int out_axis_x = atoi(getenv("dst_crop_st_x"));
  int out_axis_y = atoi(getenv("dst_crop_st_y"));
  int out_crop_w = atoi(getenv("dst_crop_sz_w"));
  int out_crop_h = atoi(getenv("dst_crop_sz_h"));
  int dst_stride_ch0 = 0,dst_stride_ch1 = 0,dst_stride_ch2 = 0,dst_stride_ch3 = 0;
  int err = 0;
  int offset_c = 0, offset_f = 0;

  dst_stride_ch0 = out_axis_x + out_crop_w;
  dst_stride_ch1 = dst_stride_ch0;
  dst_stride_ch2 = dst_stride_ch0;
  dst_stride_ch3 = 0;


  for (int i = 0; i < out_crop_h; i++)
  {
    for (int j = 0; j < out_crop_w; j++)
    {
      int n3 = (out_crop_w * i + j) * 1;
      int m3 = (out_axis_y + i) * dst_stride_ch0 + (out_axis_x + j) * 1;

      if (d1[n3] != d2[m3])
      {
        printf("p1 diff: [%dx%d] %d:%d: %d:%d\n", i, j, n3, m3, d1[n3], d2[m3]);
        err++;
      }
    }
  }

  printf("format 400!\n");
  return err; 
}
int diff_yuv420p(unsigned char *d1,unsigned char *d2)
{
  int out_axis_x = atoi(getenv("dst_crop_st_x"));
  int out_axis_y = atoi(getenv("dst_crop_st_y"));
  int out_crop_w = atoi(getenv("dst_crop_sz_w"));
  int out_crop_h = atoi(getenv("dst_crop_sz_h"));
  int dst_stride_ch0 = 0,dst_stride_ch1 = 0,dst_stride_ch2 = 0,dst_stride_ch3 = 0;
  int err = 0;
  int offset_c = 0, offset_f = 0;
  unsigned int dst_crop_sz_w_even = 0, dst_crop_sz_h_even = 0;

  dst_crop_sz_w_even = VPPALIGN(out_crop_w , 2);
  dst_crop_sz_h_even = VPPALIGN(out_crop_h , 2);

  dst_stride_ch0 = out_axis_x + out_crop_w;  
  dst_stride_ch1 = (out_axis_x + dst_crop_sz_w_even)/2;
  dst_stride_ch2 = dst_stride_ch1;
  dst_stride_ch3 = 0;

  for (int i = 0; i < out_crop_h; i++)
  {
    for (int j = 0; j < out_crop_w; j++)
    {
      int n3 = (out_crop_w * i + j) * 1;
      int m3 = (out_axis_y + i) * dst_stride_ch0 + (out_axis_x + j) * 1;

      if (d1[n3] != d2[m3])
      {
        printf("p1 diff: [%dx%d] %d:%d: %d:%d\n", i, j, n3, m3, d1[n3], d2[m3]);
        err++;
      }
    }
  }
  offset_c = out_crop_h * out_crop_w;
  offset_f = (out_axis_y + out_crop_h) * dst_stride_ch0;

  for (int i = 0; i < dst_crop_sz_h_even/2; i++)
  {
    for (int j = 0; j < dst_crop_sz_w_even/2; j++)
    {
      int n3 = (dst_crop_sz_w_even/2 * i + j) * 1 + offset_c;
      int m3 = (out_axis_y/2 + i) * dst_stride_ch1 + (out_axis_x/2 + j) * 1 + offset_f;

      if (d1[n3] != d2[m3])
      {
        printf("p2 diff: [%dx%d] %d:%d: %d:%d\n", i, j, n3, m3, d1[n3], d2[m3]);
        err++;
      }
    }
  }
  offset_c = offset_c + dst_crop_sz_h_even/2 * dst_crop_sz_w_even/2;
  offset_f = offset_f + (out_axis_y + dst_crop_sz_h_even) /2 * dst_stride_ch1;
  for (int i = 0; i < dst_crop_sz_h_even/2; i++)
  {
    for (int j = 0; j < dst_crop_sz_w_even/2; j++)
    {
      int n3 = (dst_crop_sz_w_even/2 * i + j) * 1 + offset_c;
      int m3 = (out_axis_y/2 + i) * dst_stride_ch2 + (out_axis_x/2 + j) * 1 + offset_f;

      if (d1[n3] != d2[m3])
      {
        printf("p3 diff: [%dx%d] %d:%d: %d:%d\n", i, j, n3, m3, d1[n3], d2[m3]);
        err++;
      }
    }
  }

  printf("format yuv420p!\n");

  return err; 
}
int diff_NV12(unsigned char *d1,unsigned char *d2)
{
  int out_axis_x = atoi(getenv("dst_crop_st_x"));
  int out_axis_y = atoi(getenv("dst_crop_st_y"));
  int out_crop_w = atoi(getenv("dst_crop_sz_w"));
  int out_crop_h = atoi(getenv("dst_crop_sz_h"));
  int dst_stride_ch0 = 0,dst_stride_ch1 = 0,dst_stride_ch2 = 0,dst_stride_ch3 = 0;
  int err = 0;
  int offset_c = 0, offset_f = 0;
  unsigned int dst_crop_sz_w_even = 0, dst_crop_sz_h_even = 0;

  dst_crop_sz_w_even = VPPALIGN(out_crop_w , 2);
  dst_crop_sz_h_even = VPPALIGN(out_crop_h , 2);

  dst_stride_ch0 = out_axis_x + out_crop_w;
  dst_stride_ch1 = out_axis_x + dst_crop_sz_w_even;
 


  for (int i = 0; i < out_crop_h; i++)
  {
    for (int j = 0; j < out_crop_w; j++)
    {
      int n3 = (out_crop_w * i + j) * 1;
      int m3 = (out_axis_y + i) * dst_stride_ch0 + (out_axis_x + j) * 1;

      if (d1[n3] != d2[m3])
      {
        printf("p1 diff: [%dx%d] %d:%d: %d:%d\n", i, j, n3, m3, d1[n3], d2[m3]);
        err++;
      }
    }
  }
  offset_c = out_crop_h * out_crop_w;
  offset_f = (out_axis_y + out_crop_h) * dst_stride_ch0;

  for (int i = 0; i < dst_crop_sz_h_even/2; i++)
  {
    for (int j = 0; j < dst_crop_sz_w_even; j++)
    {
      int n3 = (dst_crop_sz_w_even * i + j) * 1 + offset_c;
      int m3 = (out_axis_y/2 + i) * dst_stride_ch1 + (out_axis_x + j) * 1 + offset_f;

      if (d1[n3] != d2[m3])
      {
        printf("p2 diff: [%dx%d] %d:%d: %d:%d\n", i, j, n3, m3, d1[n3], d2[m3]);
        err++;
      }
    }
  }


  printf("format NV12!\n");

  return err; 
}
int diff_rgyp(unsigned char *d1,unsigned char *d2)
{
  int out_axis_x = atoi(getenv("dst_crop_st_x"));
  int out_axis_y = atoi(getenv("dst_crop_st_y"));
  int out_crop_w = atoi(getenv("dst_crop_sz_w"));
  int out_crop_h = atoi(getenv("dst_crop_sz_h"));
  int dst_stride_ch0 = 0,dst_stride_ch1 = 0,dst_stride_ch2 = 0,dst_stride_ch3 = 0;
  int err = 0;
  int offset_c = 0, offset_f = 0;

  dst_stride_ch0 = out_axis_x + out_crop_w;
  dst_stride_ch1 = dst_stride_ch0;
  dst_stride_ch2 = dst_stride_ch0;
  dst_stride_ch3 = dst_stride_ch0;


  for (int i = 0; i < out_crop_h; i++)
  {
    for (int j = 0; j < out_crop_w; j++)
    {
      int n3 = (out_crop_w * i + j) * 1;
      int m3 = (out_axis_y + i) * dst_stride_ch0 + (out_axis_x + j) * 1;

      if (d1[n3] != d2[m3])
      {
        printf("p1 diff: [%dx%d] %d:%d: %d:%d\n", i, j, n3, m3, d1[n3], d2[m3]);
        err++;
      }
    }
  }
  offset_c = out_crop_h * out_crop_w;
  offset_f = (out_axis_y + out_crop_h) * dst_stride_ch0;

  for (int i = 0; i < out_crop_h; i++)
  {
    for (int j = 0; j < out_crop_w; j++)
    {
      int n3 = (out_crop_w * i + j) * 1 + offset_c;
      int m3 = (out_axis_y + i) * dst_stride_ch0 + (out_axis_x + j) * 1 + offset_f;

      if (d1[n3] != d2[m3])
      {
        printf("p2 diff: [%dx%d] %d:%d: %d:%d\n", i, j, n3, m3, d1[n3], d2[m3]);
        err++;
      }
    }
  }
  offset_c = out_crop_h * out_crop_w * 2;
  offset_f = (out_axis_y + out_crop_h) * dst_stride_ch0 *2 ;
  for (int i = 0; i < out_crop_h; i++)
  {
    for (int j = 0; j < out_crop_w; j++)
    {
      int n3 = (out_crop_w * i + j) * 1 + offset_c;
      int m3 = (out_axis_y + i) * dst_stride_ch0 + (out_axis_x + j) * 1 + offset_f;

      if (d1[n3] != d2[m3])
      {
        printf("p3 diff: [%dx%d] %d:%d: %d:%d\n", i, j, n3, m3, d1[n3], d2[m3]);
        err++;
      }
    }
  }
  offset_c = out_crop_h * out_crop_w * 3;
  offset_f = (out_axis_y + out_crop_h) * dst_stride_ch0 *3 ;
  for (int i = 0; i < out_crop_h; i++)
  {
    for (int j = 0; j < out_crop_w; j++)
    {
      int n3 = (out_crop_w * i + j) * 1 + offset_c;
      int m3 = (out_axis_y + i) * dst_stride_ch0 + (out_axis_x + j) * 1 + offset_f;

      if (d1[n3] != d2[m3])
      {
        printf("p4 diff: [%dx%d] %d:%d: %d:%d\n", i, j, n3, m3, d1[n3], d2[m3]);
        err++;
      }
    }
  }
  printf("format rgyp!\n");

  return err; 
}

int diff_bgr24(unsigned char *d1,unsigned char *d2)
{
  int out_axis_x = atoi(getenv("dst_crop_st_x"));
  int out_axis_y = atoi(getenv("dst_crop_st_y"));
  int out_crop_w = atoi(getenv("dst_crop_sz_w"));
  int out_crop_h = atoi(getenv("dst_crop_sz_h"));
  int dst_stride_ch0 = 0,dst_stride_ch1 = 0,dst_stride_ch2 = 0,dst_stride_ch3 = 0;
  int err = 0;
  int offset_c = 0, offset_f = 0;

  dst_stride_ch0 = (out_axis_x + out_crop_w) *3;

  for (int i = 0; i < out_crop_h; i++)
  {
    for (int j = 0; j < out_crop_w; j++)
    {
      int n3 = (out_crop_w * i + j) * 3;
      int m3 = (out_axis_y + i) * dst_stride_ch0 + (out_axis_x + j) * 3;

      if ((d1[n3] != d2[m3]) || (d1[n3+1] != d2[m3+1]) || (d1[n3+2] != d2[m3+2]))
      {
        printf("p1 diff: [%dx%d] %d:%d: %d:%d\n", i, j, n3, m3, d1[n3], d2[m3]);
        err++;
      }
    }
  }

  printf("format bgr24, rgb24, hsv180, hsv256!\n");
  return err; 
}
int main(int argc, char *argv[])
{

  unsigned int format_in,out_format,src_width,src_height,src_crop_sz_w,src_crop_sz_h,dst_crop_sz_w,dst_crop_sz_h;

  format_in  = atoi(getenv("format_in"));
  out_format = atoi(getenv("format_out"));

  src_width = atoi(getenv("src_width"));
  src_height = atoi(getenv("src_height"));
  src_crop_sz_w = atoi(getenv("src_crop_sz_w"));
  src_crop_sz_h = atoi(getenv("src_crop_sz_h"));
  dst_crop_sz_w = atoi(getenv("dst_crop_sz_w"));
  dst_crop_sz_h = atoi(getenv("dst_crop_sz_h"));

  printf("format_in   %d, out_format   %d,src_width   %d,src_height   %d，src_crop_sz_w   %d，src_crop_sz_h   %d,dst_crop_sz_w   %d,dst_crop_sz_h   %d \n",format_in,out_format,src_width,src_height,src_crop_sz_w,src_crop_sz_h,dst_crop_sz_w,dst_crop_sz_h);


  FILE *f1 = fopen(argv[1], "r");
  FILE *f2 = fopen(argv[2], "r");
  if (f1 == NULL || f2 == NULL) {
    printf("open file error\n");
    return -1;
  }

  fseek(f1, 0, SEEK_END);
  fseek(f2, 0, SEEK_END);

  int len1 = ftell(f1);
  int len2 = ftell(f2);

  rewind(f1);
  rewind(f2);

  unsigned char *d1 = malloc(len1);
  unsigned char *d2 = malloc(len2);

  fread(d1, 1, len1, f1);
  fread(d2, 1, len2, f2);

  int err = 0;

  switch (out_format) {
  case OUT_YUV400P:
    err = diff_yuv400p(d1, d2);
    break;
  case OUT_YUV420P:
    err = diff_yuv420p(d1, d2);

    break;
  case OUT_YUV444P:
  case OUT_RGBP:
    err = diff_yuv444p(d1, d2);

    break;
  case OUT_NV12:
  case OUT_NV21:
    err = diff_NV12(d1, d2);
    break;
  case OUT_RGBYP:
    err = diff_rgyp(d1, d2);
    break;
  case OUT_RGB24:
  case OUT_BGR24:
  case OUT_HSV180:
  case OUT_HSV256:
    err = diff_bgr24(d1, d2);
    break;
  default: 
    printf("format err!\n");
    break;
  }


  printf("err count %d, out_format %d\n", err,out_format);

  free(d1);
  free(d2);
  fclose(f1);
  fclose(f2);
  if (err > 0)
  {
    return 1;
  }
  else
  {
    return 0;
  }

}
