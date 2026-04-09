#ifndef BM_NFC_H
#define BM_NFC_H

#include <stdint.h>

//------------------------------------------------------------------------------
//  constant definitions:
//------------------------------------------------------------------------------

#define DESC_CONV(x)	((((((x[0]<<8)|x[1])<<8)|x[2])<<8)|x[3])

#define CH					1
#define NR_CE_PER_CH		1
#define NR_VLM_PER_CE		1
#define NR_LUN_PER_VLM		1
#define NR_PL_PER_LUN		1
#define NR_BLK_PER_PL	    400
#define NR_WL_PER_BLK		64
#define PLANE_BIT_OFFSET    6
#define BLOCK_BIT_OFFSET    6
#define NR_PG_PER_WL_SLC	1
#define NR_PG_PER_WL_XLC	3
#define NR_PG_PER_NATIVE_BLK (NR_WL_PER_BLK * NR_PG_PER_WL_XLC)
#define NR_PG_PER_SLC_BLK	NR_WL_PER_BLK

#define PG_SZ				2048
#define AU_SZ			    2048
#define CW_SZ				2048
#define SEC_SZ				512

#define META_SZ_PER_PG		(PG_SZ / AU_SZ * 8)
#define FULL_PG_SZ			(PG_SZ + META_SZ_PER_PG)
#define NR_AU_PER_PG		(PG_SZ/AU_SZ)
#define NR_CW_PER_PG		(PG_SZ/CW_SZ)
#define NR_SEC_PER_AU		(AU_SZ/SEC_SZ)
#define NR_PG_PER_NVML		(NR_PL_PER_LUN)
#define NR_AU_PER_NVML		(NR_PG_PER_NVML * NR_AU_PER_PG)

#define NR_DIE				(CH * NR_CE_PER_CH * NR_VLM_PER_CE * NR_LUN_PER_VLM)
#define NR_INTERLEAVE		(NR_PL_PER_LUN * NR_DIE)
#define NATIVE_SBLK_SZ		(NR_INTERLEAVE * NR_PG_PER_NATIVE_BLK * FULL_PG_SZ)
#define SLC_SBLK_SZ			(NR_INTERLEAVE * NR_PG_PER_SLC_BLK * FULL_PG_SZ)
#define FILE_SZ				(NATIVE_SBLK_SZ * NR_BLK_PER_PL)

#define NAND_VECTOR_SIGNATURE DESC_CONV("NANV")
#define PRODUCTION_FW_SIGNATURE DESC_CONV("PTFM")

#define NAND_VECTOR_VERSION (0xF0000001)

#define NAND_BASE_DATA_BACKUP_COPY (2)
#define NAND_FIP_DATA_BLOCK_COUNT (4) // TODO : fip.bin / nk block size
#define NAND_FIP_DATA_BACKUP_COUNT (2) // TODO : fip.bin / nk block size

//#define NAND_BASE_BLOCK_MAX_COUNT (32)
#define NAND_BASE_BLOCK_COUNT (24) // must aligned to 8
#define NAND_BASE_BLOCK_MAX_COUNT_ENTRY (NAND_BASE_BLOCK_COUNT/8)

#define BIT(nr)			    (1UL << (nr))
#define NFC_REG_BASE        (0x500C0000)
#define NFC_SDMA_REG_BASE   (0x500C3000)

#define NFC_REG_CTRL_STATUS         (0x500C0118)
#define NFC_REG_RNB_STATUS          (0x500C1004)
#define NFC_REG_CTRL_STATUS_BUSY    (0x100)
#define TOP_REG_NFC_DD_PINS         (0x50010070)
#define TOP_REG_NFC_BOOT_REG        (0x50010074)
#define TOP_REG_NFC_ID_VALUE        (0x50010078)
#define TOP_REG_NFC_MISC            (0x5001007C)
#define TOP_REG_NFC_REMAP39_TO_32   (0x50010064)
#define TOP_RESET_REG               (0x50010C00)


#define NAND_ID_NUM 8
#define NAND_PROBE_DELAY_TIME (100) //us
#define MAX_NAND_PROBE_DELAY_COUNT (10) // 10 * 100us = 1ms


//------------------------------------------------------------------------------
//  data type definitions: typedef, struct or class
//------------------------------------------------------------------------------

typedef struct nand_info
{
    uint8_t id[8];
    uint16_t pageCnt;
    uint16_t pageSize; // Byte
    uint16_t blockCnt;
    uint8_t planeCnt;
    uint8_t dieCnt;
    uint8_t ceCnt;
    uint8_t chCnt;
    //TIMING_INFO_T timing;
} nand_info_t;

typedef struct _nand_base_vector_t
{
    uint32_t signature;
    uint32_t version;

    uint16_t fip_bin_blk[NAND_BASE_DATA_BACKUP_COPY][NAND_FIP_DATA_BLOCK_COUNT];

    uint16_t erase_count; // erase count for this block
    uint16_t rsvd_block_count; // how many blocks reserved for nand_vect and fip.bin
    uint32_t nand_vector_length;  // nand vector struct length, must less than a page

    uint16_t nand_vector_blk[NAND_BASE_DATA_BACKUP_COPY];
    uint8_t nand_base_block_usage[NAND_BASE_BLOCK_MAX_COUNT_ENTRY];

    nand_info_t nand;

    uint16_t factory_bad_block_count; // How many factory bad block
    uint16_t factory_bad_block_array_offset;

    uint16_t growing_bad_block_count; // How many growing bad block
    uint16_t growing_bad_block_array_offset;

    //uint16_t vector_buf[2048/2]; // 2K buf
} nand_base_vector_t;

#define MAX_OPEN_NAND_PB_COUNT 2

 typedef struct
{
    //uint32_t nr_au_per_page;
    //uint32_t nr_page_per_wl;
    //uint32_t nr_wl_per_blk;
    //uint32_t nr_plane;

    uint32_t block_size;
    uint32_t nr_die;
    uint32_t nr_blk;
    uint32_t nand_size;

    //uint32_t nrAuPerNvml;
    //uint32_t nrAuPerMpwl;
    //uint32_t nrAuPerSblk;
    //uint32_t nrAu;

    uint32_t pg_sz;
    uint32_t spare_sz;
    uint32_t lun_num;
    uint32_t pages_per_block;
    uint32_t number_of_planes;
    uint32_t blocks_per_lun;
    uint32_t block_shift_bit;
    //uint32_t nvml_sz;
    //uint32_t au_sz;
    //uint32_t nr_interleave;
} NAND_GEOMETRY_T;

//------------------------------------------------------------------------------
//  public function proto-type definitions:
//------------------------------------------------------------------------------
extern NAND_GEOMETRY_T gNandGeometry;

#ifdef _MSC_VER // load fip.bin to file
extern void *fake_nand;
#endif

void nfc_init(uintptr_t regs_base, uintptr_t sdma_base);
//void nandbase_init(uintptr_t regs_base, uintptr_t sdma_base);
int nfc_start(void *privateData);
NAND_GEOMETRY_T *nandbase_get_nand_geo(void);

/*
 * \brief        Flush nand vector
 *
 *
 * \param[in]    loaderInfo: if NULL, it means reading load image instead passed by caller
 *
 * \return       Result of flushing nand vector
 */
uint32_t nandbase_flush_vector(void *privateData);

#endif //BM_NFC_H
