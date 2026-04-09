#include <tomcrypt.h>
#include <golden.h>

static unsigned seed;

unsigned xseed(unsigned rs)
{
	seed = rs;
	return seed;
}
unsigned xrand(void)
{
	unsigned tmp[8];
	gdn_sha1(&seed,sizeof(seed),tmp);
	seed=tmp[1];
	return tmp[0];
}

#define BLOCK_LEN	(20)

void *xfill(void *mem,unsigned len)
{
	unsigned char *p=mem;
	unsigned cur=0;
	for(cur=0;cur+BLOCK_LEN<len;cur+=BLOCK_LEN,p+=BLOCK_LEN){
		gdn_sha1(&seed,sizeof(seed),p);
		unsigned char *q=p+BLOCK_LEN-4;
		seed=(*q)|(*(q+1)<<8)|(*(q+2)<<16)|(*(q+3)<<24);//update seed
	}
	for(;cur<len;cur++){
		//padding unaligned bytes
		*p++=xrand();
	}
	return mem;
}

