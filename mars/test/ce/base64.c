#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

char *base64std =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789"
	"+/=";

char *base64url =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789"
	"-_=";

static int base64_box_t(unsigned char box_t[256], char * box)
{
	int i;
	int box_len=strlen(box);
	memset(box_t, 0x00, 256);
	for (i = 0; i < 64; ++i) {
		box_t[(unsigned int)box[i]] = i;
	}
	box_t[64] = 0;
	return 0;
}

static int base64_dec_core(void *dst, void *src, unsigned char *box)
{
	unsigned char *d, *s;
	unsigned char t[4];
	d = dst;
	s = src;
	t[0] = box[s[0]];
	t[1] = box[s[1]];
	t[2] = box[s[2]];
	t[3] = box[s[3]];

	d[0] = (t[0] << 2) | (t[1] >> 4);
	d[1] = (t[1] << 4) | (t[2] >> 2);
	d[2] = (t[2] << 6) | (t[3]);

	return 0;
}


int base64_decode(void *dst, void *src,unsigned long size, char *box)
{
	unsigned char *d = dst;
	char *s = src;
	assert(size % 4 == 0);
	unsigned char box_t[256];
	base64_box_t(box_t, box);
	unsigned long block = size / 4;
	unsigned long i;
	for (i = 0; i < block; ++i) {
		base64_dec_core(d, s, box_t);
		d += 3;
		s += 4;
	}
	return 0;
}

static int base64_enc_core(void *dst, void *src, char *box)
{
	//convert 3x8bits to 4x6bits
	unsigned char *d, *s;
	d = dst;
	s = src;
	d[0] = box[(s[0] >> 2) & 0x3f];
	d[1] = box[((s[0] & 0x3) << 4) | (s[1] >> 4)];
	d[2] = box[((s[1] & 0xf) << 2) | (s[2] >> 6)];
	d[3] = box[(s[2] & 0x3f)];
	return 0;
}

static int base64_enc_left(void *dst, void *src,int size, char *box)
{
	assert(size == 1 || size == 2);
	//convert 3x8bits to 4x6bits
	unsigned char *d, s[3];
	memset(s, 0x00, sizeof(s));
	memcpy(s, src, size);
	d = dst;
	d[0] = box[(s[0] >> 2) & 0x3f];
	d[1] = box[((s[0] & 0x3) << 4) | (s[1] >> 4)];
	d[2] = box[((s[1] & 0xf) << 2) | (s[2] >> 6)];
	d[3] = box[64];
	if ( size == 1 )
		d[2] = d[3];
	return 0;
}

int base64_encode(void *dst, void *src, unsigned long size, char *box)
{
	unsigned long block = size / 3;
	unsigned long left = size % 3;
	unsigned char *d = dst;
	unsigned char *s = src;
	int i;
	for (i = 0; i < block; ++i) {
		base64_enc_core(d, s, box);
		d += 4;
		s += 3;
	}
	if (left)
		base64_enc_left(d, s, left, box);
	return 0;
}

void base64_show(void *str, unsigned long size)
{
	int col = 76;
	int i;
	for (i = 0; i < size; ++i) {
		if (i % col == 0 && i != 0)
			printf("\n");
		printf("%c", ((char *)str)[i]);
	}
	printf("\n");
}
