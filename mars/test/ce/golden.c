#include <tomcrypt.h>

#define GDN_IV_MAX	(32)

#define gdn_err(...)     printf(__VA_ARGS__)
#define gdn_info(...)    printf(__VA_ARGS__)
#define gdn_dbg(...)     printf(__VA_ARGS__)

int gdn_init(void)
{
	register_all_ciphers();
	register_all_hashes();
	return 0;
}

int gdn_ctr(
	    char *algo,
	    void *input,
	    void *output,
	    unsigned long size,
	    void *key,
	    int key_size,
	    int enc,
	    void *iv,
	    void *outiv)
{
	symmetric_CTR ctr;
	/* start up CTR mode */
	int err;

	err = ctr_start(
			find_cipher(algo), /* index of desired cipher */
			iv,
			key, /* the secret key */
			key_size, /* length of secret key (16 bytes) */
			0, /* 0 default # of rounds */
			CTR_COUNTER_BIG_ENDIAN,  //CTR_COUNTER_LITTLE_ENDIAN
			&ctr);/* where to store the CTR state */

	if (err != CRYPT_OK) {
		gdn_err("ctr_start error: %s\n", error_to_string(err));
		return -1;
	}
	if (enc) {
		/* somehow fill buffer than encrypt it */
		err = ctr_encrypt(
				  input, /* plaintext */
				  output, /* ciphertext */
				  size, /* length of plaintext pt */
				  &ctr);/* CTR state */
		if (err != CRYPT_OK) {
			gdn_err("ctr_encrypt error: %s\n",
				error_to_string(err));
			return -1;
		}
	} else {
		/* somehow fill buffer than encrypt it */
		err = ctr_decrypt(
				   input, /* plaintext */
				   output, /* ciphertext */
				   size, /* length of plaintext pt */
				   &ctr);/* CTR state */
		if (err != CRYPT_OK) {
			gdn_err("ctr_decrypt error: %s\n",
				error_to_string(err));
			return -1;
		}
	}
	unsigned long ivlen = GDN_IV_MAX;

	err = ctr_getiv(outiv, &ivlen, &ctr);
	if (err != CRYPT_OK) {
		gdn_err("cbc_getiv error: %s\n", error_to_string(err));
		return -1;
	}
	/* terminate the stream */
	err = ctr_done(&ctr);
	if (err != CRYPT_OK) {
		gdn_err("ctr_done error: %s\n", error_to_string(err));
		return -1;
	}
	return 0;
}

int gdn_cbc(
	    char *algo,
	    void *input,
	    void *output,
	    unsigned long size,
	    void *key,
	    int key_size,
	    int enc,
	    void *iv,
	    void *outiv)
{
	symmetric_CBC cbc;
	/* start up CBC mode */
	int err;

	err = cbc_start(
			find_cipher(algo), /* index of desired cipher */
			iv,
			key, /* the secret key */
			key_size, /* length of secret key (16 bytes) */
			0, /* 0 default # of rounds */
			&cbc); /* where to store the CBC state */

	if (err != CRYPT_OK) {
		gdn_err("cbc_start error: %s\n", error_to_string(err));
		return -1;
	}
	if (enc) {
		/* somehow fill buffer than encrypt it */
		err = cbc_encrypt(
				  input, /* plaintext */
				  output, /* ciphertext */
				  size, /* length of plaintext pt */
				  &cbc);/* CBC state */
		if (err != CRYPT_OK) {
			gdn_err("cbc_encrypt error: %s\n",
				error_to_string(err));
			return -1;
		}
	} else {
		/* somehow fill buffer than encrypt it */
		err = cbc_decrypt(
				  input, /* plaintext */
				  output, /* ciphertext */
				  size, /* length of plaintext pt */
				  &cbc);/* CBC state */
		if (err != CRYPT_OK) {
			gdn_err("cbc_decrypt error: %s\n",
				error_to_string(err));
			return -1;
		}
	}
	unsigned long ivlen = GDN_IV_MAX;

	err = cbc_getiv(outiv, &ivlen, &cbc);
	if (err != CRYPT_OK) {
		gdn_err("cbc_getiv error: %s\n", error_to_string(err));
		return -1;
	}
	/* terminate the stream */
	err = cbc_done(&cbc);
	if (err != CRYPT_OK) {
		gdn_err("cbc_done error: %s\n", error_to_string(err));
		return -1;
	}
	return 0;
}

int gdn_ecb(
	    char *algo,
	    void *input,
	    void *output,
	    unsigned long size,
	    void *key,
	    int key_size,
	    int enc)
{
	symmetric_ECB ecb;
	/* start up ECB mode */
	int err;

	err = ecb_start(
			find_cipher(algo), /* index of desired cipher */
			key, /* the secret key */
			key_size, /* length of secret key (16 bytes) */
			0, /* 0 default # of rounds */
			&ecb); /* where to store the ECB state */

	if (err != CRYPT_OK) {
		gdn_err("ecb_start error: %s\n", error_to_string(err));
		return -1;
	}
	if (enc) {
		/* somehow fill buffer than encrypt it */
		err = ecb_encrypt(
				   input, /* plaintext */
				   output, /* ciphertext */
				   size, /* length of plaintext pt */
				   &ecb); /* ECB state */
		if (err != CRYPT_OK) {
			gdn_err("ecb_encrypt error: %s\n",
				error_to_string(err));
			return -1;
		}
	} else {
		/* somehow fill buffer than encrypt it */
		err = ecb_decrypt(
				  input, /* plaintext */
				  output, /* ciphertext */
				  size, /* length of plaintext pt */
				  &ecb);/* ECB state */
		if (err != CRYPT_OK) {
			gdn_err("ecb_decrypt error: %s\n",
				error_to_string(err));
			return -1;
		}
	}
	/* terminate the stream */
	err = ecb_done(&ecb);
	if (err != CRYPT_OK) {
		gdn_err("ecb_done error: %s\n", error_to_string(err));
		return -1;
	}
	return 0;
}

int gdn_sha1(void *data, unsigned long len, void *dgst)
{
	hash_state md;

	sha1_init(&md);
	sha1_process(&md, data, len);
	sha1_done(&md, dgst);
	return 0;
}

int gdn_sha256(void *data, unsigned long len, void *dgst)
{
	hash_state md;

	sha256_init(&md);
	sha256_process(&md, data, len);
	sha256_done(&md, dgst);
	return 0;
}

