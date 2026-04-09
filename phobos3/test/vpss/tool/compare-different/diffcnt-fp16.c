#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE (4 << 20)
#define VPPALIGN(x, mask)  (((x) + ((mask)-1)) & ~((mask)-1))
enum VPP_OUTPUT_FORMAT {
  OUT_YUV444P = 3,
  OUT_RGBP = 17
};


int diff_yuv444p_fp16(unsigned char *d1,unsigned char *d2)
{
  int out_axis_x = atoi(getenv("dst_crop_st_x"));
  int out_axis_y = atoi(getenv("dst_crop_st_y"));
  int out_crop_w = atoi(getenv("dst_crop_sz_w"));
  int out_crop_h = atoi(getenv("dst_crop_sz_h"));
  int dst_stride_ch0 = 0,dst_stride_ch1 = 0,dst_stride_ch2 = 0,dst_stride_ch3 = 0;
  int err = 0;
  int offset_c = 0, offset_f = 0;

  int cmodel_stride_ch0;
  cmodel_stride_ch0 = out_crop_w * 2;

  dst_stride_ch0 = (out_axis_x + out_crop_w) * 2;
  dst_stride_ch1 = dst_stride_ch0;
  dst_stride_ch2 = dst_stride_ch0;
  dst_stride_ch3 = 0;


  for (int i = 0; i < out_crop_h; i++)
  {
    for (int j = 0; j < cmodel_stride_ch0; j++)
    {
      int n3 = (cmodel_stride_ch0 * i + j) * 1;
      int m3 = (out_axis_y + i) * dst_stride_ch0 + (out_axis_x + j) * 1;

      if (d1[n3] != d2[m3])
      {
        printf("p1 diff: [%dx%d] %d:%d: %d:%d\n", i, j, n3, m3, d1[n3], d2[m3]);
        err++;
      }
    }
  }
  offset_c = out_crop_h * cmodel_stride_ch0;
  offset_f = (out_axis_y + out_crop_h) * dst_stride_ch0;

  for (int i = 0; i < out_crop_h; i++)
  {
    for (int j = 0; j < cmodel_stride_ch0; j++)
    {
      int n3 = (cmodel_stride_ch0 * i + j) * 1 + offset_c;
      int m3 = (out_axis_y + i) * dst_stride_ch0 + (out_axis_x + j) * 1 + offset_f;

      if (d1[n3] != d2[m3])
      {
        printf("p2 diff: [%dx%d] %d:%d: %d:%d\n", i, j, n3, m3, d1[n3], d2[m3]);
        err++;
      }
    }
  }
  offset_c = out_crop_h * cmodel_stride_ch0 * 2;
  offset_f = (out_axis_y + out_crop_h) * dst_stride_ch0 *2 ;
  for (int i = 0; i < out_crop_h; i++)
  {
    for (int j = 0; j < cmodel_stride_ch0; j++)
    {
      int n3 = (cmodel_stride_ch0 * i + j) * 1 + offset_c;
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

  case OUT_YUV444P:
  case OUT_RGBP:
    err = diff_yuv444p_fp16(d1, d2);
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
