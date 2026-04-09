#ifndef __GOLDEN_H__
#define __GOLDEN_H__


int gdn_init(void);
int gdn_ecb(
		char *algo,
		void *input,
		void *output,
		unsigned long size,
		void *key,
		int key_size,
		int enc);
int gdn_cbc(
		char *algo,
		void *input,
		void *output,
		unsigned long size,
		void *key,
		int key_size,
		int enc,
		void *iv,
		void *outiv);

int gdn_ctr(
	    char *algo,
	    void *input,
	    void *output,
	    unsigned long size,
	    void *key,
	    int key_size,
	    int enc,
	    void *iv,
	    void *outiv);

int gdn_sha1(void *data, unsigned long len, void *dgst);
int gdn_sha256(void *data, unsigned long len, void *dgst);


#endif
