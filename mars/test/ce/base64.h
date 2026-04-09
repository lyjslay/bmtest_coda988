#ifndef __BASE64_H__
#define __BASE64_H__

extern char *base64std, *base64url;
int base64_encode(void *dst, void *src, unsigned long size, char *box);
int base64_decode(void *dst, void *src,unsigned long size, char *box);
void base64_show(void *str, unsigned long size);

#endif
