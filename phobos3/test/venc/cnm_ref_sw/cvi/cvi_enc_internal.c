
#include "main_helper.h"
#include "vpuapi.h"
#include "vdi.h"
#include "cvi_enc_internal.h"
#include <string.h>

#define ROUND_UP_N_BIT(val, bit)	((((val) + (1 << (bit)) - 1) >> (bit)) << (bit))

static int cviInitPageTable(EncHandle handle, int regFrameBufCount, int comp, Uint32 startAddr);
static int cviConfigPageTable(CodecInst *pCodecInst);
static int cviFindWriteEntry(CodecInst *pCodecInst, int comp);
static int cviConfigPageTableEntry(CodecInst *pCodecInst, int comp);
static void cviPrintPageTable(CodecInst *pCodecInst);

//>>> Confirm this addr from DE
#define REMAP_BASE_ADDR 	0x0B050000
#define CTRL_BASE_ADDR 		0x0B030000
static inline void RemapWriteReg(int coreIdx, unsigned int addr, unsigned int val) {
    *(volatile unsigned int *)(REMAP_BASE_ADDR + addr) = val;
}

static inline unsigned int RemapReadReg(int coreIdx, unsigned int addr) {
    return *(volatile unsigned int *)(REMAP_BASE_ADDR + addr);
}

static inline void CtrlWriteReg(int coreIdx, unsigned int addr, unsigned int val) {
    *(volatile unsigned int *)(CTRL_BASE_ADDR + addr) = val;
}

static inline unsigned int CtrlReadReg(int coreIdx, unsigned int addr) {
    return *(volatile unsigned int *)(CTRL_BASE_ADDR + addr);
}

int cviInitAddrRemap(TestEncConfig *pEncCfg,
					EncOpenParam *pEncOP, EncHandle handle,
					vpu_buffer_t* pVbReconFrameMem, FrameBuffer* pFbRecon,
					TestArgV_t *pTestArgV, int regFrameBufCount)
{
	//TestEncConfig *pEncCfg = &pTestEnc->encConfig;
	//EncOpenParam *pEncOP = &pTestEnc->encOP;
	CodecInst *pCodecInst = (CodecInst *) handle;
	EncInfo *pEncInfo = &pCodecInst->CodecInfo->encInfo;
	AddrRemap *par = &pEncInfo->addrRemap;
	int coreIdx = pEncCfg->coreIdx;
	int extraSizeInPage[2], compSize[2], compSizeInPage[2];
	int framebufStride = 0, ret = 0;
	int comp, productId;

	if (!pEncOP->cbcrInterleave) {
		printf("!!error: Only support cbcr Interleave mode, %d\n", pEncOP->cbcrInterleave);
		return -1;
	}

	par->mode = pTestArgV->ARMode;
	par->numExtraLine = pTestArgV->numExtraLine;
	par->pageSizeSel = AR_PAGE_256KB;

RECALCULATE:
	par->mmuEnable = AR_MMU_BIT30;
	par->pageSizeInBit = par->pageSizeSel + AR_PAGE_SIZE_OFFSET;
	par->pageSize = 1 << par->pageSizeInBit;
	par->lastWriteFb = 1;
	memset(par->u32ReadPhyAddr, 0, sizeof(par->u32ReadPhyAddr));

	printf("[ADDRREMAP]mode = %d, pageSize = 0x%X, numExtraLine = %d\n", par->mode, par->pageSize, par->numExtraLine);

	productId = ProductVpuGetId(coreIdx);
	framebufStride =
		CalcStride(pTestArgV->picWidth,
				pTestArgV->picHeight,
				pEncOP->srcFormat,
				pEncOP->cbcrInterleave,
				(TiledMapType)(pEncCfg->mapType & 0x0f),
				FALSE);

	compSize[AR_COMP_LUMA] =
		CalcLumaSize(productId,
				framebufStride, pTestArgV->picHeight, pEncOP->srcFormat,
				pEncOP->cbcrInterleave, (TiledMapType)(pEncCfg->mapType & 0x0f), NULL);

	compSize[AR_COMP_CHROMA] =
		CalcChromaSize(productId,
				framebufStride, pTestArgV->picHeight, pEncOP->srcFormat,
				pEncOP->cbcrInterleave, (TiledMapType)(pEncCfg->mapType & 0x0f), NULL);
	compSize[AR_COMP_CHROMA] = compSize[AR_COMP_CHROMA] << 1;

	extraSizeInPage[AR_COMP_LUMA] =
		ROUND_UP_N_BIT(framebufStride * par->numExtraLine, par->pageSizeInBit);
	extraSizeInPage[AR_COMP_CHROMA] =
		ROUND_UP_N_BIT(extraSizeInPage[AR_COMP_LUMA] >> 1, par->pageSizeInBit);

	/* STD_AVC Luma use one frame size*/
	if (pEncOP->bitstreamFormat == STD_AVC)
		extraSizeInPage[AR_COMP_LUMA] = 0;

	for (comp = AR_COMP_LUMA; comp < AR_COMP_MAX; comp++) {
		vpu_buffer_t *pvbRecFb = &(pVbReconFrameMem[comp]); //&pTestEnc->vbReconFrameBuf[comp];

		printf("[ADDRREMAP]comp = %d\n", comp);

		compSizeInPage[comp] = ROUND_UP_N_BIT(compSize[comp], par->pageSizeInBit);
		par->numFramePage[comp] = compSizeInPage[comp] >> par->pageSizeInBit;

		compSizeInPage[comp] += extraSizeInPage[comp];

		par->numVirPageInFbs[comp] = compSizeInPage[comp] >> par->pageSizeInBit;

		if (par->mode == AR_MODE_ORIG)
			compSizeInPage[comp] += par->pageSize;

		pvbRecFb->size = compSizeInPage[comp];

		par->numPhyPageInFbs[comp] = compSizeInPage[comp] >> par->pageSizeInBit;

		printf("[ADDRREMAP]compSize = 0x%X, compSizeInPage = 0x%X, extraSizeInPage = 0x%X\n",
				compSize[comp], compSizeInPage[comp], extraSizeInPage[comp]);
		printf("[ADDRREMAP]numPhyPageInFbs = 0x%X, numVirPageInFbs = 0x%X\n",
				par->numPhyPageInFbs[comp], par->numVirPageInFbs[comp]);

	}

	if (par->numPhyPageInFbs[AR_COMP_LUMA] + par->numPhyPageInFbs[AR_COMP_CHROMA] >
		AR_MAX_NUM_PAGE_TABLE_ENTRY / 2) {
		par->pageSizeSel += 1;
		if (par->pageSizeSel > AR_PAGE_1024KB) {
			printf("[ADDRREMAP]!!error:Size too large not support addrremap\n");
			return -1;
		}
		goto RECALCULATE;
	}

	for (comp = AR_COMP_LUMA; comp < AR_COMP_MAX; comp++) {
		vpu_buffer_t *pvbRecFb = &(pVbReconFrameMem[comp]);

		// memset(ionName, 0, MAX_VPU_ION_BUFFER_NAME);
		// sprintf(ionName, "VENC_%d_ReconFb_%d", pCodecInst->s32ChnNum, comp);

		//if (VDI_ALLOCATE_MEMORY(coreIdx, pvbRecFb, 0, ionName) < 0) {
		if(vdi_allocate_dma_memory(coreIdx, pvbRecFb) < 0) {
			printf("[ADDRREMAP]!!error:fail to allocate recon buffer, %d\n", comp);
			return -1;
		}
	}

	for (comp = AR_COMP_LUMA; comp < AR_COMP_MAX; comp++) {
		vpu_buffer_t *pvbRecFb = &(pVbReconFrameMem[comp]);
		FrameBuffer *pfb = &(pFbRecon[comp]);

		pfb->size = pvbRecFb->size;
		pfb->bufCb = (PhysicalAddress) - 1;
		pfb->bufCr = (PhysicalAddress) - 1;
		pfb->updateFbInfo = TRUE;

		if (par->mode == AR_MODE_ORIG) {
			pfb->bufY = ROUND_UP_N_BIT(pvbRecFb->phys_addr, par->pageSizeInBit);
		} else {
			pfb->bufY = pvbRecFb->phys_addr;
		}

		printf("[ADDRREMAP]fbRecon[%d], bufY = 0x%llX, bufCb = 0x%llX, bufCr = 0x%llX\n",
				comp, pfb->bufY, pfb->bufCb, pfb->bufCr);
		printf("[ADDRREMAP]vbReconFrameBuf[%d], phys_addr = 0x%llX\n",
				comp, pvbRecFb->phys_addr);

		ret = cviInitPageTable(handle, regFrameBufCount, comp, pfb->bufY);
		if (ret != 0) {
			printf("[ADDRREMAP]!!error:cviInitPageTable\n");
			return ret;
		}
	}

	return ret;
}

static int cviInitPageTable(EncHandle handle, int regFrameBufCount, int comp, Uint32 startAddr)
{
	CodecInst *pCodecInst = (CodecInst *) handle;
	EncInfo *pEncInfo = &pCodecInst->CodecInfo->encInfo;
	AddrRemap *par = &pEncInfo->addrRemap;
	Uint32 currAddr = startAddr;
	int ret = 0;
	Int32 i, j;

	printf("[ADDRREMAP]PageTable[%d]\n", comp);
	printf("[ADDRREMAP]numPhyPageInFbs = %d\n", par->numPhyPageInFbs[comp]);

	for (i = 0; i < par->numVirPageInFbs[comp]; i++) {
		PageEntry *pEntry = &par->pageTable[comp][i];

		pEntry->phyAddr = currAddr;
		pEntry->flag = regFrameBufCount;

		printf("[ADDRREMAP][%d] phyAddr = 0x%X, flag = 0x%X\n", i, pEntry->phyAddr, pEntry->flag);

		currAddr += par->pageSize;
	}

	if (regFrameBufCount > AR_MAX_FB_NUM) {
		printf("!!error:PageTable[%d]\n", comp);
		return -1;
	}

	for (i = 0; i < regFrameBufCount; i++) {
		for (comp = AR_COMP_LUMA; comp < AR_COMP_MAX; comp++) {
			for (j = 0; j < par->numFramePage[comp]; j++) {
				par->fbPageIndex[i][comp][j] = j;
			}
		}
	}

	return ret;
}

int cviInitAddrRemapFb(CodecInst *pCodecInst)
{
	EncInfo *pEncInfo = &pCodecInst->CodecInfo->encInfo;
	AddrRemap *par = &pEncInfo->addrRemap;
	int coreIdx = pCodecInst->coreIdx;
	Uint32 val, reg;
	int i;

	// Enable vc addr remap, reg VC_FAB_MMU_CTRL
	val = 0;
	val |= (par->mmuEnable & 0x3);
	val |= ((par->mode & 0x1) << 3);
	val |= ((par->pageSizeSel & 0x3) << 4);

	reg = CtrlReadReg(coreIdx, VC_FAB_MMU_CTRL);

	if (pCodecInst->productId == PRODUCT_ID_420L) {
		reg &= 0x0000FFFF;
		reg |= (val << 16);
	} else {
		reg &= 0xFFFF0000;
		reg |= val;
	}
	CtrlWriteReg(coreIdx, VC_FAB_MMU_CTRL, reg);

	val = CtrlReadReg(coreIdx, VC_FAB_MMU_CTRL);
	printf("[ADDRREMAP]VC CTRL Reg 0x%x = 0x%x\n", VC_FAB_MMU_CTRL, val);

	// Setup recon frame YUV virtual address
	val = 1 << (AR_MMU_ENABLE_OFFSET - par->mmuEnable);

	for (i = 0; i < pEncInfo->numFrameBuffers; i++) {
		FrameBuffer *pFb = &pEncInfo->frameBufPool[i];

		pFb->bufY = val;
		val += (par->numFramePage[AR_COMP_LUMA] << par->pageSizeInBit);

		pFb->bufCb = val;
		val += (par->numFramePage[AR_COMP_CHROMA] << par->pageSizeInBit);

		printf("[ADDRREMAP]frameBufPool[%d]  bufY = 0x%llX, bufCb = 0x%llX\n",
			i, pFb->bufY, pFb->bufCb);
	}

	return RETCODE_SUCCESS;
}

int cviSetupPicConfig(EncHandle handle, EncOpenParam *pEncOP, EncParam *pEncParam)
{
	// EncOpenParam *pEncOP = &pTestEnc->encOP;
	// EncParam *pEncParam = &pTestEnc->encParam;
	CodecInst *pCodecInst = (CodecInst *) handle;
	EncInfo *pEncInfo = &pCodecInst->CodecInfo->encInfo;
	AddrRemap *par = &pEncInfo->addrRemap;
	int ret = 0;

	if (pEncInfo->addrRemapEn) {
		// if (pEncOP->bitstreamFormat == STD_HEVC) {
		// 	if (pEncParam->is_idr_frame)
		// 		par->lastWriteFb = 1;
		// }

		ret = cviConfigPageTable(pCodecInst);
		if (ret < 0) {
			printf("[ADDRREMAP]!!error:cviConfigPageTable, %d\n", ret);
			return ret;
		}

		cviPrintPageTable(pCodecInst);
	}

	return ret;
}

static int cviConfigPageTable(CodecInst *pCodecInst)
{
	EncInfo *pEncInfo = &pCodecInst->CodecInfo->encInfo;
	AddrRemap *par = &pEncInfo->addrRemap;
	int comp;
	int ret = 0;

	printf("[ADDRREMAP]frameIdx = %d, lastWriteFb = %d\n", pEncInfo->frameIdx, par->lastWriteFb);

	for (comp = AR_COMP_LUMA; comp < AR_COMP_MAX; comp++) {
		ret = cviFindWriteEntry(pCodecInst, comp);
		if (ret < 0) {
			printf("[ADDRREMAP]!!error:cviFindWriteEntry, comp = %d\n", comp);
			return -1;
		}

		ret = cviConfigPageTableEntry(pCodecInst, comp);
	}

	par->lastWriteFb = (par->lastWriteFb + 1) & 0x1;

	return ret;
}

static int cviFindWriteEntry(CodecInst *pCodecInst, int comp)
{
	EncInfo *pEncInfo = &pCodecInst->CodecInfo->encInfo;
	AddrRemap *par = &pEncInfo->addrRemap;
	int currWriteFb = (par->lastWriteFb + 1) & 0x1;
	int foundPage, currPage;
	int j;

	// step1: find from unused pages
	currPage = 0;
	for (j = 0; j < par->numVirPageInFbs[comp]; j++) {
		if (par->pageTable[comp][j].flag != par->lastWriteFb) {
			foundPage = j;

			par->fbPageIndex[currWriteFb][comp][currPage] = foundPage;
			par->pageTable[comp][foundPage].flag = currWriteFb;

			currPage++;

			if (currPage >= par->numFramePage[comp]) {
				printf("[ADDRREMAP]find finished, currPage = %d\n", currPage);
				return 0;
			}
		}
	}

	// step2:find from the last written Frame Buffer
	for (j = 0; j < par->numVirPageInFbs[comp]; j++) {
		foundPage = par->fbPageIndex[par->lastWriteFb][comp][j];

		par->fbPageIndex[currWriteFb][comp][currPage] = foundPage;
		par->pageTable[comp][foundPage].flag = currWriteFb;

		currPage++;

		if (currPage >= par->numFramePage[comp]) {
			printf("[ADDRREMAP]top of lastWriteFb, currPage = %d\n", currPage);
			return 0;
		}
	}

	printf("[ADDRREMAP]!!error:currPage = %d, not enough free page\n", currPage);

	return -1;
}

static int cviConfigPageTableEntry(CodecInst *pCodecInst, int comp)
{
	EncInfo *pEncInfo = &pCodecInst->CodecInfo->encInfo;
	EncOpenParam *pOpenParam = &pEncInfo->openParam;
	AddrRemap *par = &pEncInfo->addrRemap;
	int regStartAddr;
	int currWriteFb = (par->lastWriteFb + 1) & 0x1;
	int i;

	printf("[ADDRREMAP]PageTable[%d]\n", comp);

	if (par->u32ReadPhyAddr[comp][0]) {
		regStartAddr = (par->lastWriteFb & 0x1) *
			(par->numFramePage[AR_COMP_LUMA] + par->numFramePage[AR_COMP_CHROMA]);
		regStartAddr += comp * par->numFramePage[AR_COMP_LUMA];
		regStartAddr <<= 2;
		// if (pOpenParam->bitstreamFormat == STD_HEVC)
		// 	regStartAddr += H265_MAP_REG_OFFSET;
		for (i = 0; i < par->numFramePage[comp]; i++) {
			RemapWriteReg(pCodecInst->coreIdx, regStartAddr, par->u32ReadPhyAddr[comp][i]);
			printf("[ADDRREMAP][%d] last write, regStartAddr = 0x%X, phyAddr = 0x%X\n",
				i, regStartAddr, par->u32ReadPhyAddr[comp][i]);
			regStartAddr += 4;
		}
	}

	regStartAddr = currWriteFb *
		(par->numFramePage[AR_COMP_LUMA] + par->numFramePage[AR_COMP_CHROMA]);
	regStartAddr += comp * par->numFramePage[AR_COMP_LUMA];
	regStartAddr <<= 2;

	// if (pOpenParam->bitstreamFormat == STD_HEVC)
	// 	regStartAddr += H265_MAP_REG_OFFSET;

	for (i = 0; i < par->numFramePage[comp]; i++) {
		int newEntry = par->fbPageIndex[currWriteFb][comp][i];
		int phyAddr = par->pageTable[comp][newEntry].phyAddr;

		if (newEntry >= AR_MAX_NUM_PAGE_TABLE_ENTRY) {
			printf("!!error:comp = %d, i = %d\n", comp, i);
			return -1;
		}

		printf("[ADDRREMAP][%d] page idx = %d, regStartAddr = 0x%X, phyAddr = 0x%X\n",
				i, newEntry, regStartAddr, phyAddr);

		RemapWriteReg(pCodecInst->coreIdx, regStartAddr, phyAddr);
		par->u32ReadPhyAddr[comp][i] = phyAddr;

		regStartAddr += 4;
	}

	return 0;
}

static void cviPrintPageTable(CodecInst *pCodecInst)
{
	EncInfo *pEncInfo = &pCodecInst->CodecInfo->encInfo;
	EncOpenParam *pOpenParam = &pEncInfo->openParam;
	AddrRemap *par = &pEncInfo->addrRemap;
	PageEntry *pEntry;

	int comp, i, val, addr;
	int fb;

	// if ((vcodec_mask & CVI_MASK_AR) == 0)
	// 	return;

	UNUSED(pEntry);
	printf("[ADDRREMAP]PageTable -------------------------------------\n");

	for (comp = AR_COMP_LUMA; comp < AR_COMP_MAX; comp++) {
		printf("[ADDRREMAP]PageTable[%d]\n", comp);
		printf("[ADDRREMAP]numPhyPageInFbs = %d\n", par->numPhyPageInFbs[comp]);

		for (i = 0; i < par->numVirPageInFbs[comp]; i++) {
			pEntry = &par->pageTable[comp][i];
			printf("[ADDRREMAP][%d] phyAddr = 0x%X, flag = 0x%X\n", i, pEntry->phyAddr, pEntry->flag);
		}
	}

	printf("[ADDRREMAP]Addr Remap registers --------------------------\n");

	addr = (pOpenParam->bitstreamFormat == STD_HEVC) ? H265_MAP_REG_OFFSET : 0;

	for (fb = 0; fb < 2; fb++) {
		printf("[ADDRREMAP]fb = %d\n", fb);

		for (comp = AR_COMP_LUMA; comp < AR_COMP_MAX; comp++) {
			printf("[ADDRREMAP]comp = %d\n", comp);

			for (i = 0; i < par->numFramePage[comp]; i++) {

				val = RemapReadReg(pCodecInst->coreIdx, addr);
				printf("[ADDRREMAP]addr = 0x%X, val = 0x%X\n", addr, val);
				addr += 4;
			}
		}
	}
}