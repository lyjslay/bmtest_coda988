#include "boot_test.h"
#include "uart.h"
#include "testcase_spacc.h"
#include "cli.h"
#include "mmio.h"
#include "bl2.h"
#include "bl2_aes128_enc.h"
#include "bl2_des_enc.h"
#include "bl2_sm4_enc.h"
#include "system_common.h"
#include "cache.h"

unsigned char key[16] = {0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61,
          0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61};
unsigned char iv[16] = {0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61,
          0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61};

int uart_loopback_test(uint8_t data)
{
  uint8_t recv;

  uart_putc(data);
  recv = (uint8_t)uart_getc();
  if (data != recv)
    return -1;

  return 0;
}
#if 0
static int irq_handler(int irqn, void *priv)
{
  uartlog("INT: I am here\n");

  return 0;
}
#endif
int plat_cryptodma_bypass(uintptr_t in, uintptr_t out, uint64_t len)
{
  uint32_t data;
  int i;

  __attribute__((aligned(64))) uint32_t dma_descriptor[32] = {0};

  NOTICE("plat_cryptodma, src=%p, des=%p, image_size=0x%x\n",
            (void *)in, (void *)out, (int)len);

  uint64_t src = (uintptr_t)in;
  uint64_t des = (uintptr_t)out;

  dma_descriptor[CRYPTODMA_CTRL] = DES_USE_BYPASS | 0xF;
  dma_descriptor[CRYPTODMA_CIPHER] = 0;

  dma_descriptor[CRYPTODMA_SRC_ADDR_L] = (uint32_t)((uint64_t)src & 0xFFFFFFFF);
  dma_descriptor[CRYPTODMA_SRC_ADDR_H] = (uint32_t)((uint64_t)src >> 32);

  dma_descriptor[CRYPTODMA_DST_ADDR_L] = (uint32_t)((uint64_t)des & 0xFFFFFFFF);
  dma_descriptor[CRYPTODMA_DST_ADDR_H] = (uint32_t)((uint64_t)des >> 32);

  dma_descriptor[CRYPTODMA_DATA_AMOUNT_L] = (uint32_t)((uint64_t)len & 0xFFFFFFFF);
  dma_descriptor[CRYPTODMA_DATA_AMOUNT_H] = (uint32_t)((uint64_t)len >> 32);

  // Set cryptodma dma control
  mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_INT_MASK, 0x3);
  mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DES_BASE_L, (uint32_t)((uint64_t)dma_descriptor & 0xFFFFFFFF));
  mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DES_BASE_H, (uint32_t)((uint64_t)dma_descriptor >> 32));

  data = mmio_read_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DMA_CTRL);

  flush_dcache_range((unsigned long)dma_descriptor, sizeof(dma_descriptor));
  flush_dcache_range((unsigned long)src, 64);

  for(i=0; i<22; i++)
  {
    NOTICE("dma_descriptor[%d] = %x\n", i, dma_descriptor[i]);
  }
  INFO("dma_descriptor address: %p\n", dma_descriptor);

  // Clear interrupt
  mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_WR_INT, 0x3);
  // Trigger cryptodma engine
  mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DMA_CTRL, DMA_WRITE_MAX_BURST << 24 | DMA_READ_MAX_BURST << 16 | DMA_DESCRIPTOR_MODE << 1 | DMA_ENABLE);

  //ts = timer_get_tick();
  do {
    data = mmio_read_32(SEC_CRYPTODMA_BASE + CRYPTODMA_WR_INT);
    NOTICE("INT status 0x%x\n", data);
    //if((timer_get_tick() - ts) >= 1000){
    //  NOTICE("Decryption timeout\n");
    //  break;
    //}
  } while(data == 0);

  invalidate_dcache_range(des, 64);
  invalidate_dcache_all();

  return 0;
}

int plat_cryptodma_aes_cbc(int enc, uintptr_t in, uintptr_t out, uint64_t len, unsigned char *key, unsigned char *iv)
{
  uint32_t data;
  //uint32_t ts;
  int i;

  __attribute__((aligned(64))) uint32_t dma_descriptor[32] = {0};
  //uint32_t *dma_descriptor = (uint32_t *)0x40100000;

  NOTICE("plat_cryptodma, src=%p, des=%p, image_size=0x%x\n",
            (void *)in, (void *)out, (int)len);

  // Fill descriptor data
  //memset(dma_descriptor, 0, 32*sizeof(uint32_t));

  uint64_t src = (uintptr_t)in;
  uint64_t des = (uintptr_t)out;

  dma_descriptor[CRYPTODMA_CTRL] = DES_USE_DESCRIPTOR_IV | DES_USE_DESCRIPTOR_KEY | DES_USE_AES | 0xF;
  dma_descriptor[CRYPTODMA_CIPHER] = AES_KEY_MODE << 3 | CBC_ENABLE << 1 | enc;

  dma_descriptor[CRYPTODMA_SRC_ADDR_L] = (uint32_t)((uint64_t)src & 0xFFFFFFFF);
  dma_descriptor[CRYPTODMA_SRC_ADDR_H] = (uint32_t)((uint64_t)src >> 32);

  dma_descriptor[CRYPTODMA_DST_ADDR_L] = (uint32_t)((uint64_t)des & 0xFFFFFFFF);
  dma_descriptor[CRYPTODMA_DST_ADDR_H] = (uint32_t)((uint64_t)des >> 32);

  dma_descriptor[CRYPTODMA_DATA_AMOUNT_L] = (uint32_t)((uint64_t)len & 0xFFFFFFFF);
  dma_descriptor[CRYPTODMA_DATA_AMOUNT_H] = (uint32_t)((uint64_t)len >> 32);

  memcpy(&dma_descriptor[CRYPTODMA_KEY], key, 16);
  memcpy(&dma_descriptor[CRYPTODMA_IV], iv, 16);

  for(i=0; i<16; i++) {
    NOTICE("key[%d] %x\n", i, key[i]);
    NOTICE("iv[%d] %x\n", i, iv[i]);
  }

  // Set cryptodma dma control
  mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_INT_MASK, 0x3);
  mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DES_BASE_L, (uint32_t)((uint64_t)dma_descriptor & 0xFFFFFFFF));
  mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DES_BASE_H, (uint32_t)((uint64_t)dma_descriptor >> 32));

  data = mmio_read_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DMA_CTRL);

  flush_dcache_range((unsigned long)dma_descriptor, sizeof(dma_descriptor));
  flush_dcache_range((unsigned long)src, 64);

  for(i=0; i<22; i++)
  {
    NOTICE("dma_descriptor[%d] = %x\n", i, dma_descriptor[i]);
  }
  INFO("dma_descriptor address: %p\n", dma_descriptor);

  // Clear interrupt
  mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_WR_INT, 0x3);
  // Trigger cryptodma engine
  mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DMA_CTRL, DMA_WRITE_MAX_BURST << 24 | DMA_READ_MAX_BURST << 16 | DMA_DESCRIPTOR_MODE << 1 | DMA_ENABLE);

  //ts = timer_get_tick();
  do {
    data = mmio_read_32(SEC_CRYPTODMA_BASE + CRYPTODMA_WR_INT);
    NOTICE("INT status 0x%x\n", data);
    //if((timer_get_tick() - ts) >= 1000){
    //  NOTICE("Decryption timeout\n");
    //  break;
    //}
  } while(data == 0);

  invalidate_dcache_range(des, 64);
  invalidate_dcache_all();

  return 0;
}

int plat_cryptodma_aes_ecb(int enc, uintptr_t in, uintptr_t out, uint64_t len, unsigned char *key)
{
  uint32_t data;
  int i;

  __attribute__((aligned(64))) uint32_t dma_descriptor[32] = {0};

  NOTICE("plat_cryptodma, src=%p, des=%p, image_size=0x%x\n",
            (void *)in, (void *)out, (int)len);

  uint64_t src = (uintptr_t)in;
  uint64_t des = (uintptr_t)out;

  dma_descriptor[CRYPTODMA_CTRL] = DES_USE_DESCRIPTOR_IV | DES_USE_DESCRIPTOR_KEY | DES_USE_AES | 0xF;
  dma_descriptor[CRYPTODMA_CIPHER] = AES_KEY_MODE << 3 | enc;

  dma_descriptor[CRYPTODMA_SRC_ADDR_L] = (uint32_t)((uint64_t)src & 0xFFFFFFFF);
  dma_descriptor[CRYPTODMA_SRC_ADDR_H] = (uint32_t)((uint64_t)src >> 32);

  dma_descriptor[CRYPTODMA_DST_ADDR_L] = (uint32_t)((uint64_t)des & 0xFFFFFFFF);
  dma_descriptor[CRYPTODMA_DST_ADDR_H] = (uint32_t)((uint64_t)des >> 32);

  dma_descriptor[CRYPTODMA_DATA_AMOUNT_L] = (uint32_t)((uint64_t)len & 0xFFFFFFFF);
  dma_descriptor[CRYPTODMA_DATA_AMOUNT_H] = (uint32_t)((uint64_t)len >> 32);

  memcpy(&dma_descriptor[CRYPTODMA_KEY], key, 16);

  for(i=0; i<16; i++) {
    NOTICE("key[%d] %x\n", i, key[i]);
  }

  // Set cryptodma dma control
  mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_INT_MASK, 0x3);
  mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DES_BASE_L, (uint32_t)((uint64_t)dma_descriptor & 0xFFFFFFFF));
  mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DES_BASE_H, (uint32_t)((uint64_t)dma_descriptor >> 32));

  data = mmio_read_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DMA_CTRL);

  flush_dcache_range((unsigned long)dma_descriptor, sizeof(dma_descriptor));
  flush_dcache_range((unsigned long)src, 64);

  for(i=0; i<22; i++)
  {
    NOTICE("dma_descriptor[%d] = %x\n", i, dma_descriptor[i]);
  }
  INFO("dma_descriptor address: %p\n", dma_descriptor);

  // Clear interrupt
  mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_WR_INT, 0x3);
  // Trigger cryptodma engine
  mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DMA_CTRL, DMA_WRITE_MAX_BURST << 24 | DMA_READ_MAX_BURST << 16 | DMA_DESCRIPTOR_MODE << 1 | DMA_ENABLE);

  //ts = timer_get_tick();
  do {
    data = mmio_read_32(SEC_CRYPTODMA_BASE + CRYPTODMA_WR_INT);
    NOTICE("INT status 0x%x\n", data);
    //if((timer_get_tick() - ts) >= 1000){
    //  NOTICE("Decryption timeout\n");
    //  break;
    //}
  } while(data == 0);

  invalidate_dcache_range(des, 64);
  invalidate_dcache_all();

  return 0;
}

int plat_cryptodma_aes_ctr(int enc, uintptr_t in, uintptr_t out, uint64_t len, unsigned char *key, unsigned char *iv)
{
  uint32_t data;
  int i;

  __attribute__((aligned(64))) uint32_t dma_descriptor[32] = {0};

  NOTICE("plat_cryptodma, src=%p, des=%p, image_size=0x%x\n",
            (void *)in, (void *)out, (int)len);

  uint64_t src = (uintptr_t)in;
  uint64_t des = (uintptr_t)out;

  dma_descriptor[CRYPTODMA_CTRL] = DES_USE_DESCRIPTOR_IV | DES_USE_DESCRIPTOR_KEY | DES_USE_AES | 0xF;
  dma_descriptor[CRYPTODMA_CIPHER] = AES_KEY_MODE << 3 | 0x1 << 2 | enc;

  dma_descriptor[CRYPTODMA_SRC_ADDR_L] = (uint32_t)((uint64_t)src & 0xFFFFFFFF);
  dma_descriptor[CRYPTODMA_SRC_ADDR_H] = (uint32_t)((uint64_t)src >> 32);

  dma_descriptor[CRYPTODMA_DST_ADDR_L] = (uint32_t)((uint64_t)des & 0xFFFFFFFF);
  dma_descriptor[CRYPTODMA_DST_ADDR_H] = (uint32_t)((uint64_t)des >> 32);

  dma_descriptor[CRYPTODMA_DATA_AMOUNT_L] = (uint32_t)((uint64_t)len & 0xFFFFFFFF);
  dma_descriptor[CRYPTODMA_DATA_AMOUNT_H] = (uint32_t)((uint64_t)len >> 32);

  memcpy(&dma_descriptor[CRYPTODMA_KEY], key, 16);
  memcpy(&dma_descriptor[CRYPTODMA_IV], iv, 16);

  for(i=0; i<16; i++) {
    NOTICE("key[%d] %x\n", i, key[i]);
    NOTICE("iv[%d] %x\n", i, iv[i]);
  }

  // Set cryptodma dma control
  mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_INT_MASK, 0x3);
  mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DES_BASE_L, (uint32_t)((uint64_t)dma_descriptor & 0xFFFFFFFF));
  mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DES_BASE_H, (uint32_t)((uint64_t)dma_descriptor >> 32));

  data = mmio_read_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DMA_CTRL);

  flush_dcache_range((unsigned long)dma_descriptor, sizeof(dma_descriptor));
  flush_dcache_range((unsigned long)src, 64);

  for(i=0; i<22; i++)
  {
    NOTICE("dma_descriptor[%d] = %x\n", i, dma_descriptor[i]);
  }
  INFO("dma_descriptor address: %p\n", dma_descriptor);

  // Clear interrupt
  mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_WR_INT, 0x3);
  // Trigger cryptodma engine
  mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DMA_CTRL, DMA_WRITE_MAX_BURST << 24 | DMA_READ_MAX_BURST << 16 | DMA_DESCRIPTOR_MODE << 1 | DMA_ENABLE);

  //ts = timer_get_tick();
  do {
    data = mmio_read_32(SEC_CRYPTODMA_BASE + CRYPTODMA_WR_INT);
    NOTICE("INT status 0x%x\n", data);
    //if((timer_get_tick() - ts) >= 1000){
    //  NOTICE("Decryption timeout\n");
    //  break;
    //}
  } while(data == 0);

  invalidate_dcache_range(des, 64);
  invalidate_dcache_all();

  return 0;
}

// AES-128-ECB data encryption
int testcase_spacc_aes_128_ecb_enc(void)
{
  int ret = 0;
  int i;
  
  __attribute__((aligned(64))) uint8_t src[64] = {0};
  __attribute__((aligned(64))) uint8_t result[64] = {0};

  uartlog("%s\n", __func__);

  char *p = (char *)src;
  p[0] = 'h';
  p[1] = 'e';
  p[2] = 'l';
  p[3] = 'l';
  p[4] = 'o';

  //Padding data
  for (i = 5; i < 16; i++) {
      p[i] = 0xb;
  }

  plat_cryptodma_aes_ecb(1, (uintptr_t)src, (uintptr_t)result, 16, key);

  for(i=0; i< 16; i++) {
    uartlog("%x\n", ((char *)result)[i]);
  }

  return ret;
}

// AES-128-ECB data decryption
int testcase_spacc_aes_128_ecb_dec(void)
{
  int ret = 0;
  int i;

  uartlog("%s\n", __func__);

  __attribute__((aligned(64))) uint8_t src[64] = {0};
  __attribute__((aligned(64))) uint8_t result[64] = {0};

  char *p = (char *)src;
  p[0] = 0xEC;
  p[1] = 0x61;
  p[2] = 0xD1;
  p[3] = 0x7C;
  p[4] = 0x58;
  p[5] = 0x0a;
  p[6] = 0x55;
  p[7] = 0xD6;
  p[8] = 0x80;
  p[9] = 0x8B;
  p[10] = 0x0B;
  p[11] = 0x62;
  p[12] = 0xDF;
  p[13] = 0xAB;
  p[14] = 0x6A;
  p[15] = 0x0D;

  plat_cryptodma_aes_ecb(0, (uintptr_t)src, (uintptr_t)result, 16, key);

  for(i=0; i< 16; i++) {
    uartlog("%x\n", ((char *)result)[i]);
  }

  return ret;
}

// AES-128-CBC data encryption
int testcase_spacc_aes_128_cbc_enc(void)
{
  int ret = 0;
  int i;

  __attribute__((aligned(64))) uint8_t src[64] = {0};
  //uintptr_t src = (uintptr_t)0xBC00000;
  //memset((void *)src, 0x0, 64);

  char *p = (char *)src;
  p[0] = 'h';
  p[1] = 'e';
  p[2] = 'l';
  p[3] = 'l';
  p[4] = 'o';

  //Padding data
  for (i = 5; i < 16; i++) {
      p[i] = 0xb;
  }

  __attribute__((aligned(64))) uint8_t result[64] = {0};
  //uintptr_t result = (uintptr_t)0x5200000;
  //unsigned char result[16] = {0};
  //memset((void *)result, 0x0, 64);

  uartlog("%s\n", __func__);

  plat_cryptodma_aes_cbc(1, (uintptr_t)src, (uintptr_t)result, 16, key, iv);

  for(i=0; i< 16; i++) {
    uartlog("%x\n", ((char *)result)[i]);
    /*if(((char *)result)[i] != bl2_aes128_enc[i]) {
      uartlog("mis-match %d\n", i);
      ret = 1;
      break;
    } */
  }

  return ret;
}

// AES-128-CBC data decryption
int testcase_spacc_aes_128_cbc_dec(void)
{
  int ret = 0;
  int i;

  uartlog("%s\n", __func__);

  __attribute__((aligned(64))) uint8_t src[64] = {0};
  __attribute__((aligned(64))) uint8_t result[64] = {0};

  char *p = (char *)src;
  p[0] = 0x5D;
  p[1] = 0x1C;
  p[2] = 0x7C;
  p[3] = 0x1E;
  p[4] = 0x64;
  p[5] = 0x7E;
  p[6] = 0x28;
  p[7] = 0xB4;
  p[8] = 0x07;
  p[9] = 0xF5;
  p[10] = 0x53;
  p[11] = 0xDF;
  p[12] = 0x51;
  p[13] = 0x46;
  p[14] = 0x8C;
  p[15] = 0xEA;

  plat_cryptodma_aes_cbc(0, (uintptr_t)src, (uintptr_t)result, 16, key, iv);

  for(i=0; i< 16; i++) {
    uartlog("%x\n", ((char *)result)[i]);
    /*if(((char *)result)[i] != bl2_aes128_enc[i]) {
      uartlog("mis-match %d\n", i);
      ret = 1;
      break;
    } */
  }

  return ret;
}

// AES-128-CTR data encryption
int testcase_spacc_aes_128_ctr_enc(void)
{
  int ret = 0;
  int i;
  
  __attribute__((aligned(64))) uint8_t src[64] = {0};
  __attribute__((aligned(64))) uint8_t result[64] = {0};

  uartlog("%s\n", __func__);

  char *p = (char *)src;
  p[0] = 'h';
  p[1] = 'e';
  p[2] = 'l';
  p[3] = 'l';
  p[4] = 'o';

  //Padding data
  for (i = 5; i < 16; i++) {
      p[i] = 0xb;
  }

  plat_cryptodma_aes_ctr(1, (uintptr_t)src, (uintptr_t)result, 16, key, iv);

  for(i=0; i< 16; i++) {
    uartlog("%x\n", ((char *)result)[i]);
  }

  return ret;
}

// AES-128-ctr data decryption
int testcase_spacc_aes_128_ctr_dec(void)
{
  int ret = 0;
  int i;

  uartlog("%s\n", __func__);

  __attribute__((aligned(64))) uint8_t src[64] = {0};
  __attribute__((aligned(64))) uint8_t result[64] = {0};

  char *p = (char *)src;
  p[0] = 0x39;
  p[1] = 0xED;
  p[2] = 0xAA;
  p[3] = 0x2B;
  p[4] = 0x24;
  p[5] = 0x29;
  p[6] = 0x87;
  p[7] = 0xB6;
  p[8] = 0xD9;
  p[9] = 0x49;
  p[10] = 0xE2;
  p[11] = 0x19;
  p[12] = 0x55;
  p[13] = 0xB5;
  p[14] = 0x16;
  p[15] = 0x58;

  plat_cryptodma_aes_ctr(0, (uintptr_t)src, (uintptr_t)result, 16, key, iv);

  for(i=0; i< 16; i++) {
    uartlog("%x\n", ((char *)result)[i]);
  }

  return ret;
}

// bypass mode
int testcase_spacc_bypass_mode(void)
{
  int ret = 0;
  int i;
  
  __attribute__((aligned(64))) uint8_t src[64] = {0};
  __attribute__((aligned(64))) uint8_t result[64] = {0};

  uartlog("%s\n", __func__);

  char *p = (char *)src;
  p[0] = 'h';
  p[1] = 'e';
  p[2] = 'l';
  p[3] = 'l';
  p[4] = 'o';

  //Padding data
  for (i = 5; i < 16; i++) {
      p[i] = 0xb;
  }

  plat_cryptodma_bypass((uintptr_t)src, (uintptr_t)result, 16);

  for(i=0; i< 16; i++) {
    uartlog("%x\n", ((char *)result)[i]);
  }

  return ret;
}

#define SHA256          (0x1 << 1)
#define SHA_LOAD_PARAM  (0x1)

static uint8_t is_little_endian()
{
    static uint32_t _endian_x_ = 1;
    return ((const uint8_t *)(& _endian_x_))[0];
}

static uint8_t  is_big_endian()
{
    return !is_little_endian();
}
//reverse byte order
static  uint32_t reverse_32bit(uint32_t data)
{
    data = (data >> 16) | (data << 16);
    return ((data & 0xff00ff00UL) >> 8) | ((data & 0x00ff00ffUL) << 8);
}

//host byte order to big endian
static uint32_t os_htobe32(uint32_t data)
{
    if (is_big_endian()) {
        return data;
    }
    return reverse_32bit(data);
}

static inline uint64_t reverse_64bit(uint64_t data)
{
    data = (data >> 32) | (data << 32);
    data = ((data & 0xff00ff00ff00ff00ULL) >> 8) | ((data & 0x00ff00ff00ff00ffULL) << 8);

    return ((data & 0xffff0000ffff0000ULL) >> 16) | ((data & 0x0000ffff0000ffffULL) << 16);
}

//host to big endian
static uint64_t os_htobe64(uint64_t data)
{
    if (is_big_endian()) {
        return data;
    }

    return reverse_64bit(data);
}

int testcase_spacc_sha256(void)
{
  __aligned(64) uint32_t dma_descriptor[32] = { 0 };
  __aligned(64) uint8_t data[64] = { 0 };

  uint32_t state[8];

  state[0] = os_htobe32(0x6A09E667);
  state[1] = os_htobe32(0xBB67AE85);
  state[2] = os_htobe32(0x3C6EF372);
  state[3] = os_htobe32(0xA54FF53A);
  state[4] = os_htobe32(0x510E527F);
  state[5] = os_htobe32(0x9B05688C);
  state[6] = os_htobe32(0x1F83D9AB);
  state[7] = os_htobe32(0x5BE0CD19);

  uint64_t src = (uintptr_t)data;
  uint32_t status;
  int i;

  memcpy(data, "hello", 5);
  data[5] = 0x80;

  uint64_t bits_len_be = os_htobe64(40);
  memcpy(&data[56], &bits_len_be, 8);

  // Prepare descriptor
  dma_descriptor[CRYPTODMA_CTRL] = DES_USE_DESCRIPTOR_KEY | DES_USE_DESCRIPTOR_IV | DES_USE_SHA | 0xF;
  dma_descriptor[CRYPTODMA_CIPHER] = SHA256 | SHA_LOAD_PARAM;

  dma_descriptor[CRYPTODMA_SRC_ADDR_L] = (uint32_t)(src & 0xFFFFFFFF);
  dma_descriptor[CRYPTODMA_SRC_ADDR_H] = (uint32_t)(src >> 32);

  dma_descriptor[CRYPTODMA_DST_ADDR_L] = 0;
  dma_descriptor[CRYPTODMA_DST_ADDR_H] = 0;

  dma_descriptor[CRYPTODMA_DATA_AMOUNT_L] = 64;
  dma_descriptor[CRYPTODMA_DATA_AMOUNT_H] = 0;
    
  // Set cryptodma control
  mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_INT_MASK, 0x3);
  mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DES_BASE_L,
                  (uint32_t)((uintptr_t)dma_descriptor & 0xFFFFFFFF));
  mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DES_BASE_H,
                  (uint32_t)((uintptr_t)dma_descriptor >> 32));

  // Clear SHA output first
  for (i = 0; i < 8; i++)
    mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_SHA_PARA + i * 4, 0x12345678);

  for (i = 0; i < 8; i++)
    dma_descriptor[CRYPTODMA_KEY + i] = state[i];

  status = mmio_read_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DMA_CTRL);

  flush_dcache_range((unsigned long)dma_descriptor, sizeof(dma_descriptor));
  flush_dcache_range((unsigned long)data, 64);

  // Clear interrupt
  mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_WR_INT, 0x3);
  // Trigger cryptodma engine
  mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DMA_CTRL,
                  DMA_WRITE_MAX_BURST << 24 | DMA_READ_MAX_BURST << 16 | DMA_DESCRIPTOR_MODE << 1 | DMA_ENABLE);

  //uint32_t time_start = csi_tick_get_ms();
  do {
    status = mmio_read_32(SEC_CRYPTODMA_BASE + CRYPTODMA_WR_INT);
    /*if ((csi_tick_get_ms() - time_start) >= 5) {
      printf("sha timeout\n");
      return CSI_ERROR;
    }*/
  } while (status == 0);
  
  for (i = 0; i < 8; i++) {
    uintptr_t a = SEC_CRYPTODMA_BASE + CRYPTODMA_SHA_PARA + i * 4;
    state[i] = mmio_read_32(a);
    printf("i : %x, 0x%x \n", i, state[i]);
  }

  return 0;
}

int testcase_spacc_sha1(void)
{
  __aligned(64) uint32_t dma_descriptor[32] = { 0 };
  __aligned(64) uint8_t data[64] = { 0 };

  uint32_t state[5];

  state[0] = os_htobe32(0x67452301);
  state[1] = os_htobe32(0xEFCDAB89);
  state[2] = os_htobe32(0x98BADCFE);
  state[3] = os_htobe32(0x10325476);
  state[4] = os_htobe32(0xC3D2E1F0);

  uint64_t src = (uintptr_t)data;
  uint32_t status;
  int i;

  memcpy(data, "hello", 5);
  data[5] = 0x80;

  uint64_t bits_len_be = os_htobe64(40);
  memcpy(&data[56], &bits_len_be, 8);

  // Prepare descriptor
  dma_descriptor[CRYPTODMA_CTRL] = DES_USE_DESCRIPTOR_KEY | DES_USE_DESCRIPTOR_IV | DES_USE_SHA | 0xF;
  dma_descriptor[CRYPTODMA_CIPHER] = SHA_LOAD_PARAM;

  dma_descriptor[CRYPTODMA_SRC_ADDR_L] = (uint32_t)(src & 0xFFFFFFFF);
  dma_descriptor[CRYPTODMA_SRC_ADDR_H] = (uint32_t)(src >> 32);

  dma_descriptor[CRYPTODMA_DST_ADDR_L] = 0;
  dma_descriptor[CRYPTODMA_DST_ADDR_H] = 0;

  dma_descriptor[CRYPTODMA_DATA_AMOUNT_L] = 64;
  dma_descriptor[CRYPTODMA_DATA_AMOUNT_H] = 0;
    
  // Set cryptodma control
  mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_INT_MASK, 0x3);
  mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DES_BASE_L,
                  (uint32_t)((uintptr_t)dma_descriptor & 0xFFFFFFFF));
  mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DES_BASE_H,
                  (uint32_t)((uintptr_t)dma_descriptor >> 32));

  // Clear SHA output first
  for (i = 0; i < 5; i++)
    mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_SHA_PARA + i * 4, 0x12345678);

  for (i = 0; i < 5; i++)
    dma_descriptor[CRYPTODMA_KEY + i] = state[i];

  status = mmio_read_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DMA_CTRL);

  flush_dcache_range((unsigned long)dma_descriptor, sizeof(dma_descriptor));
  flush_dcache_range((unsigned long)data, 64);

  // Clear interrupt
  mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_WR_INT, 0x3);
  // Trigger cryptodma engine
  mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DMA_CTRL,
                  DMA_WRITE_MAX_BURST << 24 | DMA_READ_MAX_BURST << 16 | DMA_DESCRIPTOR_MODE << 1 | DMA_ENABLE);

  //uint32_t time_start = csi_tick_get_ms();
  do {
    status = mmio_read_32(SEC_CRYPTODMA_BASE + CRYPTODMA_WR_INT);
    /*if ((csi_tick_get_ms() - time_start) >= 5) {
      printf("sha timeout\n");
      return CSI_ERROR;
    }*/
  } while (status == 0);
  
  for (i = 0; i < 5; i++) {
    uintptr_t a = SEC_CRYPTODMA_BASE + CRYPTODMA_SHA_PARA + i * 4;
    state[i] = mmio_read_32(a);
    printf("i : %x, 0x%x \n", i, state[i]);
  }

  return 0;
}

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
  int ret = 0;

  ret = testcase_spacc_aes_128_ecb_enc();
  ret |= testcase_spacc_aes_128_ecb_dec();
  ret |= testcase_spacc_aes_128_cbc_enc();
  ret |= testcase_spacc_aes_128_cbc_dec();
  ret |= testcase_spacc_aes_128_ctr_enc();
  ret |= testcase_spacc_aes_128_ctr_dec();
  ret |= testcase_spacc_bypass_mode();
  ret |= testcase_spacc_sha256();
  ret |= testcase_spacc_sha1();

  uartlog("testcase spacc %s\n", ret?"failed":"passed");
  return ret;
}
#endif
