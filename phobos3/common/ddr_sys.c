#include <mmio.h>
#include "ddr_sys.h"
#include "system_common.h"
#include "bitwise_ops.h"

// #define LP4
// #define DDR3
// #define DDR4

// #define SSC_BYPASS
// #define SSC_EN

#if 0
void check_rd32(uintptr_t addr, uint32_t expected)
{
	uint32_t rdata;

	rdata = mmio_rd32(addr);
	if (rdata != expected)
		mmio_wr32(TERM_SIM_ADDR, 0x0B0B0B0B); // fail
}
#endif

void bm_mrw(uint32_t mr_num, uint32_t mr_wrdata, uint32_t mr_cs, uint32_t mr_mode)
{
	uint32_t init_dis_auto_zq;
	uint32_t write_temp = 0;

	// enable dfi_phyupd_mask w/o auto ABR
#ifdef LP4
	// 2666
	mmio_wr32(0x0800a000 + 0x500, 0x0a2a1101);
	mmio_wr32(0x0800a000 + 0x504, 0x00000000);
	mmio_wr32(0x0800a000 + 0x204, 0x00000110);
#elif defined DDR3
	// 2133
	mmio_wr32(0x0800a000 + 0x500, 0x103d1101);
	mmio_wr32(0x0800a000 + 0x504, 0x00c00001); //trfc=350
	mmio_wr32(0x0800a000 + 0x204, 0x00000100);
#elif defined DDR4
	// 2400
	mmio_wr32(0x0800a000 + 0x500, 0x12481101);
	mmio_wr32(0x0800a000 + 0x504, 0x02400002); //trfc=550
	mmio_wr32(0x0800a000 + 0x204, 0x00000100);
#endif
	// ZQCTL0.dis_auto_zq to 1.
	ddr_mmio_rd32(cfg_base + 0x180, rddata);
	if (get_bits_from_value(rddata, 31, 31) == 0) {
		init_dis_auto_zq = 0;
		//rddata[31] = 1;
		rddata = modified_bits_by_value(rddata, 1, 31, 31);
		mmio_wr32(cfg_base + 0x180, rddata);
		uartlog("non-lp4 Write ZQCTL0.dis_auto_zq to 1\n");
		opdelay(256);
		uartlog("Wait tzqcs = 128 cycles\n");
	}
	//CS0 MPW data (single mode)
	write_temp = modified_bits_by_value(write_temp, mr_wrdata, 7, 0);
	//REGWR  (261 + PI_BASE_ADDR, {0x0, mr_wrdata}, 0);
	mmio_wr32(4 * (261 + PI_BASE_ADDR) + CADENCE_PHYD, write_temp);
	//CS1 MPW data (single mode)
	//REGWR  (262 + PI_BASE_ADDR, {0x0, mr_wrdata}, 0);
	//REGWR  (262 + PI_BASE_ADDR, write_temp, 0);
	mmio_wr32(4 * (262 + PI_BASE_ADDR) + CADENCE_PHYD, write_temp);
	//CS2 MPW data (single mode)
	//REGWR  (263 + PI_BASE_ADDR, {0x0, mr_wrdata}, 0);
	mmio_wr32(4 * (263 + PI_BASE_ADDR) + CADENCE_PHYD, write_temp);
	//CS3 MPW data (single mode)
	//REGWR  (264 + PI_BASE_ADDR, {0x0, mr_wrdata}, 0);
	mmio_wr32(4 * (264 + PI_BASE_ADDR) + CADENCE_PHYD, write_temp);
	rddata = 0;
	//REGREAD(98 + PI_BASE_ADDR, rddata);
	ddr_mmio_rd32(4 * (98 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = modified_bits_by_value(rddata, mr_num, 7, 0);
	rddata = modified_bits_by_value(rddata, mr_cs, 15, 8);
	rddata = modified_bits_by_value(rddata, mr_mode, 23, 16);
	//-- Bit [23] = (single) Write a single MRz
	//-- Bit [18] = Write MR16, MR17
	//-- Bit [17] = (base) Write MR0, MR1, MR2, MR3, MR4, MR5, MR6, MR11
	//-- Bit [16] = (all) Write MR0, MR1, MR2, MR3, MR4, MR5, MR6, MR11, MR22
	//REGWR  (98 + PI_BASE_ADDR, rddata, 0);
	mmio_wr32(4 * (98 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	//rddata[25] = 1; //Trigger MRW
	rddata = modified_bits_by_value(rddata, 1, 25, 25);
	//REGWR  (98 + PI_BASE_ADDR, rddata, 0);
	mmio_wr32(4 * (98 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	//rddata[19]=0;
	rddata = modified_bits_by_value(rddata, 0, 19, 19);
	//while(!rddata[19]) {
	while (get_bits_from_value(rddata, 19, 19) == 0) {
		//REGREAD(83 + PI_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (83 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
		opdelay(100);
	}
	//REGWR  (84 + PI_BASE_ADDR, rddata, 0);
	mmio_wr32(4 * (84 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	if (init_dis_auto_zq == 0) {
		// ZQCTL0.dis_auto_zq to 0.
		ddr_mmio_rd32(cfg_base + 0x180, rddata);
		//rddata[31] = 0;
		rddata = modified_bits_by_value(rddata, 0, 31, 31);
		mmio_wr32(cfg_base + 0x180, rddata);
		uartlog("non-lp4 Write ZQCTL0.dis_auto_zq to 0\n");
	}
}

void bm_pi_reinit(void)
{
	// enable dfi_phyupd_mask w/o auto ABR
#ifdef LP4
	// 2666
	mmio_wr32(0x0800a000 + 0x500, 0x0a2a1101);
	mmio_wr32(0x0800a000 + 0x504, 0x00000000);
	mmio_wr32(0x0800a000 + 0x204, 0x00000110);
#elif defined DDR3
	// 2133
	mmio_wr32(0x0800a000 + 0x500, 0x103d1101);
	mmio_wr32(0x0800a000 + 0x504, 0x00c00001); //trfc=350
	mmio_wr32(0x0800a000 + 0x204, 0x00000100);
#elif defined DDR4
	// 2400
	mmio_wr32(0x0800a000 + 0x500, 0x12481101);
	mmio_wr32(0x0800a000 + 0x504, 0x02400002); //trfc=550
	mmio_wr32(0x0800a000 + 0x204, 0x00000100);
#endif
	//------------------------------
	//  param_pi_cke_inactive_r + 10
	//-----------------------------
	//param_pi_cke_inactive_r[7:0] <= `PI_SD int_regin[7:0];
	//REGREAD(95 + PI_BASE_ADDR, rddata);
	ddr_mmio_rd32(4 * (95 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	//REGWR  (95 + PI_BASE_ADDR, rddata+20, 0);
	mmio_wr32(4 * (95 + PI_BASE_ADDR) + CADENCE_PHYD, (rddata + 20));
	uartlog("param_pi_cke_inactive_r\n");
	//------------------------------
	//  param_pi_root_clk_on
	//-----------------------------
	//param_pi_root_clk_on[0:0] <= `PI_SD int_regin[24:24];
	//REGREAD(112 + PI_BASE_ADDR, rddata);
	ddr_mmio_rd32(4 * (112 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	//rddata[24] = 1;
	rddata = modified_bits_by_value(rddata, 1, 24, 24);
	//REGWR  (112 + PI_BASE_ADDR, rddata, 0);
	mmio_wr32(4 * (112 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	uartlog("param_pi_root_clk_on\n");
	//------------------------------
	//  param_pi_init_lvl_en
	//-----------------------------
	//param_pi_init_lvl_en[0:0] <= `PI_SD int_regin[8:8];
	//REGREAD(1 + PI_BASE_ADDR, rddata);
	ddr_mmio_rd32(4 * (1 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	//rddata[8] = 1'b0;
	rddata = modified_bits_by_value(rddata, 0, 8, 8);
	//REGWR  (1 + PI_BASE_ADDR, rddata, 0);
	mmio_wr32(4 * (1 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	uartlog("param_pi_init_lvl_en =0\n");
	//------------------------------
	// param_phyd_reg_pi_init for dram init
	//------------------------------
	//param_phyd_reg_pi_init         <= `PI_SD int_regin[2];
	//REGREAD(241 + PHY_BASE_ADDR, rddata);
	ddr_mmio_rd32(4 * (241 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	//rddata[2] = 1'b1;
	rddata = modified_bits_by_value(rddata, 1, 2, 2);
	//REGWR  (241 + PHY_BASE_ADDR, rddata, 0);
	mmio_wr32(4 * (241 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	uartlog("param_phyd_reg_pi_init sw_reinit_dfi_bus_request\n");
	//------------------------------
	//  param_pi_dram_init_en for dram init
	//------------------------------
	//param_pi_dram_init_en[0:0] <= `PI_SD int_regin[8:8];
	//REGREAD(96 + PI_BASE_ADDR, rddata);
	ddr_mmio_rd32(4 * (96 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	//rddata[8] = 1'b1;
	rddata = modified_bits_by_value(rddata, 1, 8, 8);
	//REGWR  (96 + PI_BASE_ADDR, rddata, 0);
	mmio_wr32(4 * (96 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	uartlog("param_pi_dram_init_en =1\n");
	// read dfi_phymstr_ack/req
	//phy_regout[0]        = param_phyd_to_reg_dfi_phymstr_req;
	//phy_regout[1]        = param_phyd_to_reg_dfi_phymstr_ack;
	//REGREAD(1207 + PHY_BASE_ADDR, rddata);
	ddr_mmio_rd32(4 * (1207 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	uartlog("Wait dfi_phymstr_ack\n");
	//while (rddata[1:0] != 2'b11) {
	while (get_bits_from_value(rddata, 1, 0) != 0x3) {
		// REGREAD(1207 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (1207 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
		opdelay(10);
	}
	//------------------------------
	//  param_pi_start =0
	//-----------------------------
	//param_pi_start[0:0] <= `PI_SD int_regin[0:0];
	//REGREAD(0 + PI_BASE_ADDR, rddata);
	ddr_mmio_rd32(4 * (0 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	//rddata[0] = 1'b0;
	rddata = modified_bits_by_value(rddata, 0, 0, 0);
	//REGWR  (0 + PI_BASE_ADDR, rddata, 0);
	mmio_wr32(4 * (0 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	uartlog("PI_START =0\n");
	opdelay(20);
	//------------------------------
	//  param_pi_start =1
	//-----------------------------
	//param_pi_start[0:0] <= `PI_SD int_regin[0:0];
	//REGREAD(0 + PI_BASE_ADDR, rddata);
	ddr_mmio_rd32(4 * (0 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	//rddata[0] = 1'b1;
	rddata = modified_bits_by_value(rddata, 1, 0, 0);
	//REGWR  (0 + PI_BASE_ADDR, rddata, 0);
	mmio_wr32(4 * (0 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	uartlog("PI_START =1\n");
	//param_pi_int_status <= `PI_SD {`PI_NUM_INT_SOURCES+1{1'b0}}
	//REGREAD(83 + PI_BASE_ADDR, rddata);
	ddr_mmio_rd32(4 * (83 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	//while (!rddata[0]) {
	while (get_bits_from_value(rddata, 0, 0) == 0) {
		//Read PHY interrupt status
		// REGREAD(1095 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (1095 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
		if ((rddata != 0)) {
			//param_phyd_clkctrl_init_complete   <= int_regin[24];
			// REGREAD(133 + PHY_BASE_ADDR, rddata);
			ddr_mmio_rd32(4 * (133 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
			// rddata[24] = 0;
			rddata = modified_bits_by_value(rddata, 0, 24, 24);
			// REGWR  (133 + PHY_BASE_ADDR, rddata, 0);
			mmio_wr32(4 * (133 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
			uartlog("init_complete =0\n");
			//param_phyd_clkctrl_init_complete   <= int_regin[24];
			//REGREAD(133 + PHY_BASE_ADDR, rddata);
			ddr_mmio_rd32(4 * (133 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
			//rddata[24] = 1;
			rddata = modified_bits_by_value(rddata, 1, 24, 24);
			//REGWR  (133 + PHY_BASE_ADDR, rddata, 0);
			mmio_wr32(4 * (133 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
			//param_phyd_phy_int_ack
			//REGWR  (179 + PHY_BASE_ADDR, 0xffffffff, 0);
			mmio_wr32(4 * (179 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
			uartlog("clear phy_int\n");
		}
		//REGREAD(83 + PI_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (83 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	}
	opdelay(200);
	//REGWR  (84 + PI_BASE_ADDR, rddata, 0);
	mmio_wr32(4 * (84 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	uartlog("param_pi_int_ack\n");
	//------------------------------
	// param_phyd_reg_pi_init for dram init
	//------------------------------
	//param_phyd_reg_pi_init         <= `PI_SD int_regin[2];
	//REGREAD(241 + PHY_BASE_ADDR, rddata);
	ddr_mmio_rd32(4 * (241 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	//rddata[2] = 1'b0;
	rddata = modified_bits_by_value(rddata, 0, 2, 2);
	//REGWR  (241 + PHY_BASE_ADDR, rddata, 0);
	mmio_wr32(4 * (241 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	uartlog("param_phyd_reg_pi_init sw_reinit_dfi_bus_request\n");
	uartlog("param_phyd_reg_pi_init =0\n");
	//------------------------------
	//  param_pi_root_clk_on
	//-----------------------------
	//param_pi_root_clk_on[0:0] <= `PI_SD int_regin[24:24];
	//REGREAD(112 + PI_BASE_ADDR, rddata);
	//rddata[24] = 0;
	//REGWR  (112 + PI_BASE_ADDR, rddata, 0);
	ddr_mmio_rd32(4 * (112 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = modified_bits_by_value(rddata, 0, 24, 24);
	mmio_wr32(4 * (112 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	uartlog("param_pi_root_clk_on =0\n");
	//------------------------------
	//  param_pi_dram_init_en for dram init
	//------------------------------
	//param_pi_dram_init_en[0:0] <= `PI_SD int_regin[8:8];
	//REGREAD(96 + PI_BASE_ADDR, rddata);
	//rddata[8] = 1'b0;
	//REGWR  (96 + PI_BASE_ADDR, rddata, 0);
	ddr_mmio_rd32(4 * (96 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = modified_bits_by_value(rddata, 0, 8, 8);
	mmio_wr32(4 * (96 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	uartlog("param_pi_dram_init_en =0\n");
	// enable dfi_phyupd_mask w/ auto ABR
#ifdef LP4
	// 2666
	mmio_wr32(0x0800a000 + 0x500, 0x0a2a1111);
	mmio_wr32(0x0800a000 + 0x504, 0x00000000);
	mmio_wr32(0x0800a000 + 0x204, 0x00000110);
#elif defined DDR3
	// 2133
	mmio_wr32(0x0800a000 + 0x500, 0x103d1111);
	mmio_wr32(0x0800a000 + 0x504, 0x00c00001); //trfc=350
	mmio_wr32(0x0800a000 + 0x204, 0x00000100);
#elif defined DDR4
	// 2400
	mmio_wr32(0x0800a000 + 0x500, 0x12481111);
	mmio_wr32(0x0800a000 + 0x504, 0x02400002); //trfc=550
	mmio_wr32(0x0800a000 + 0x204, 0x00000100);
#endif
}

void bm_synp_mrr_lp4(uint32_t addr)
{
	uartlog("bm_synp_mrr_lp4\n");
	// Poll MRSTAT.mr_wr_busy until it is ????
	while (1) {
		ddr_mmio_rd32(cfg_base + 0x18, rddata);
		if (get_bits_from_value(rddata, 0, 0) == 0)
			break;
	}
	// Write the MRCTRL0.mr_type, MRCTRL0.mr_addr, MRCTRL0.mr_rank and (for MRWs) MRCTRL1.mr_data
	//rddata[31:0] = 0;
	//rddata[0] = 1;       // mr_type  0:write   1:read
	//rddata[5:4] = 1;     // mr_rank
	mmio_wr32(cfg_base + 0x10, rddata);
	uartlog("lp4 Write the MRCTRL0\n");
	// rddata[31:0] = 0;
	// rddata[15:8] = addr;     // mr_addr
	// rddata[ 7:0] = 0;        // mr_data
	rddata = modified_bits_by_value(rddata, 0, 31, 0);
	rddata = modified_bits_by_value(rddata, addr, 15, 8);
	rddata = modified_bits_by_value(rddata, 0, 7, 0);
	mmio_wr32(cfg_base + 0x14, rddata);
	uartlog("lp4 Write the MRCTRL1\n");
	// Write MRCTRL0.mr_wr to 1
	ddr_mmio_rd32(cfg_base + 0x10, rddata);
	//rddata[31] = 1;
	rddata = modified_bits_by_value(rddata, 1, 31, 31);
	mmio_wr32(cfg_base + 0x10, rddata);
	uartlog("lp4 Write MRCTRL0.mr_wr to 1\n");
	while (1) {
		ddr_mmio_rd32(cfg_base + 0x18, rddata);
		//} while (rddata[0] != 0);
		if (get_bits_from_value(rddata, 0, 0) == 0)
			break;
	}
}

void bm_bist_rw_prbs(void)
{
	//reg [31:0] DDR_BIST_BASE;
	//reg [31:0] cmd[5:0];
	//reg [31:0] sram_st;
	//reg [31:0] sram_sp;
	uint32_t cmd[6];
	uint32_t sram_st;
	uint32_t sram_sp;
	int i;

	uartlog("bm_bist_rw_prbs\n");
	//DDR_BIST_BASE = 0x08010000;
	sram_st = 0;
	sram_sp = 511;
	// cmd queue
	//        op_code         start              stop        pattern    dq_inv     dm_inv    dq_rotate   repeat
	cmd[0] = (1 << 30) | (sram_st << 21) | (sram_sp << 12) | (5 << 9) | (0 << 8) | (0 << 7) | (0 << 4) | (0 << 0); // W  1~17  prbs  repeat0
	cmd[1] = (2 << 30) | (sram_st << 21) | (sram_sp << 12) | (5 << 9) | (0 << 8) | (0 << 7) | (0 << 4) | (0 << 0); // R  1~17  prbs  repeat0
	cmd[2] = 0; // NOP
	cmd[3] = 0; // NOP
	cmd[4] = 0; // NOP
	cmd[5] = 0; // NOP
	// write cmd queue
	for (i = 0; i < 6; i = i + 1) {
		mmio_wr32(DDR_BIST_BASE + 0x40 + i * 4, cmd[i]);
	};
	// specified DDR space
	mmio_wr32(DDR_BIST_BASE + 0x10, 0x00100000);
	mmio_wr32(DDR_BIST_BASE + 0x14, 0x001fffff);
	// bist enable
	mmio_wr32(DDR_BIST_BASE + 0x0, 0x00010001);
	uartlog("bist start\n");
	// polling bist done
	//    while (get_bits_from_value(ddr_mmio_rd32(DDR_BIST_BASE + 0x80),2,2)  == 0x0);
	while (1) {
		ddr_mmio_rd32(DDR_BIST_BASE + 0x80, rddata);
		if (get_bits_from_value(rddata, 2, 2) == 1)
			break;
	}
	//   } while (rddata[2] != 'b1);
	uartlog("bist done\n");
	//   if (rddata[3] == 1'b1) {
	if (get_bits_from_value(rddata, 3, 3) == 1) {
		opdelay(10);
		uartlog("bist fail\n");
	} else {
		opdelay(10);
		uartlog("bist pass\n");
	}
	// bist disable
	mmio_wr32(DDR_BIST_BASE + 0x0, 0x00010000);
	// report pass/fail
	//    if (get_bits_from_value(ddr_mmio_rd32(DDR_BIST_BASE + 0x80),3,3)  == 0x0) {
	//        mmio_wr32(TERM_SIM_ADDR,0x0A0A0A0A);    // pass
	//    } else {
	//        mmio_wr32(TERM_SIM_ADDR,0x0B0B0B0B);    // fail
	//    }
}

void bm_bist_wr_prbs_init(void)
{
	//reg [31:0] cmd[5:0];
	//reg [31:0] sram_st;
	//reg [31:0] sram_sp;
	uint32_t cmd[6];
	uint32_t sram_st;
	uint32_t sram_sp;
	int i;

	uartlog("bist_wr_prbs_init\n");
	uartlog("[KC_DBG] , bist_wr_prbs_init\n");
	sram_st = 0;
	sram_sp = 511;
	// cmd queue
	//        op_code         start              stop        pattern    dq_inv     dm_inv    dq_rotate   repeat
	cmd[0] = (1 << 30) | (sram_st << 21) | (sram_sp << 12) | (5 << 9) | (0 << 8) | (0 << 7) | (0 << 4) | (0 << 0); // W  1~17  prbs  repeat0
	cmd[1] = (2 << 30) | (sram_st << 21) | (sram_sp << 12) | (5 << 9) | (0 << 8) | (0 << 7) | (0 << 4) | (0 << 0); // R  1~17  prbs  repeat0
	cmd[2] = 0; // NOP
	cmd[3] = 0; // NOP
	cmd[4] = 0; // NOP
	cmd[5] = 0; // NOP
	// write cmd queue
	for (i = 0; i < 6; i = i + 1) {
		mmio_wr32(DDR_BIST_BASE + 0x40 + i * 4, cmd[i]);
	};
	// specified DDR space
	mmio_wr32(DDR_BIST_BASE + 0x10, 0x00100000);
	mmio_wr32(DDR_BIST_BASE + 0x14, 0x001fffff);
	uartlog("bist_wr_prbs_init done\n");
}

void bm_bist_start_check(uint32_t *bist_result, uint64_t *err_data_odd, uint64_t *err_data_even)
{
	uint64_t err_data_even_l;
	uint64_t err_data_even_h;
	uint64_t err_data_odd_l;
	uint64_t err_data_odd_h;

	uartlog("bm_bist_start_check\n");
#ifdef X16_MODE
	// bist enable
	mmio_wr32(DDR_BIST_BASE + 0x0, 0x00030003);
	uartlog("bist start\n");
#else
	// bist enable
	mmio_wr32(DDR_BIST_BASE + 0x0, 0x00010001);
	uartlog("bist start\n");
#endif
	// polling bist done
	//    while (get_bits_from_value(ddr_mmio_rd32(DDR_BIST_BASE + 0x80),2,2)  == 0x0);
	while (1) {
		ddr_mmio_rd32(DDR_BIST_BASE + 0x80, rddata);
		if (get_bits_from_value(rddata, 2, 2) == 1)
			break;
	}
	uartlog("bist done\n");
	if (get_bits_from_value(rddata, 3, 3) == 1) {
		opdelay(10);
		*bist_result = 0;
		uartlog("bist fail\n");
		uartlog("err data\n");
		// read err_data
		ddr_mmio_rd32(DDR_BIST_BASE + 0x88, err_data_odd_l);
		ddr_mmio_rd32(DDR_BIST_BASE + 0x8c, err_data_odd_h);
		ddr_mmio_rd32(DDR_BIST_BASE + 0x90, err_data_even_l);
		ddr_mmio_rd32(DDR_BIST_BASE + 0x94, err_data_even_h);
		*err_data_odd = err_data_odd_h << 32 | err_data_odd_l;
		*err_data_even = err_data_even_h << 32 | err_data_even_l;
	} else {
		opdelay(10);
		*bist_result = 1;
		uartlog("bist pass\n");
		*err_data_odd = 0;
		*err_data_even = 0;
	}
	// bist disable
	mmio_wr32(DDR_BIST_BASE + 0x0, 0x00010000);
}

void bm_bist_rd_prbs_init(void)
{
	uint32_t cmd[6];
	uint32_t sram_st;
	uint32_t sram_sp;
	int i;

	uartlog("bist_rd_prbs_init\n");
	sram_st = 0;
	sram_sp = 511;
	// cmd queue
	//        op_code         start              stop        pattern    dq_inv     dm_inv    dq_rotate   repeat
	cmd[0] = (2 << 30) | (sram_st << 21) | (sram_sp << 12) | (5 << 9) | (0 << 8) | (0 << 7) | (0 << 4) | (0 << 0); // W  1~17  prbs  repeat0
	cmd[1] = 0; // NOP
	cmd[2] = 0; // NOP
	cmd[3] = 0; // NOP
	cmd[4] = 0; // NOP
	cmd[5] = 0; // NOP
	// write cmd queue
	for (i = 0; i < 6; i = i + 1) {
		mmio_wr32(DDR_BIST_BASE + 0x40 + i * 4, cmd[i]);
	};
	// specified DDR space
	mmio_wr32(DDR_BIST_BASE + 0x10, 0x00100000);
	mmio_wr32(DDR_BIST_BASE + 0x14, 0x001fffff);
	uartlog("bist_wr_prbs_init done\n");
}

void bm_bist_rx_delay(uint32_t delay)
{
	uartlog("bm_bist_rx_delay\n");
	//dqs delay
	opdelay(10);
	uartlog("byte0 dqs delay\n");
	//rddata = {0x00, delay, 0x00, delay};
	rddata = 0;
	rddata = modified_bits_by_value(rddata, delay, 7, 0);
	rddata = modified_bits_by_value(rddata, delay, 23, 16);
	//original      REGWR  (170 + PHY_BASE_ADDR, rddata, 0);
	mmio_wr32(4 * (170 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	uartlog("byte1 dqs delay\n");
	//rddata = {0x00, delay, 0x00, delay};
	//rddata = modified_bits_by_value(rddata, delay, 7, 0);
	//rddata = modified_bits_by_value(rddata, delay, 23, 16);
	//original      REGWR  (171 + PHY_BASE_ADDR, rddata, 0);
	mmio_wr32(4 * (171 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
#ifndef X16_MODE
	uartlog("byte2 dqs delay\n");
	//rddata = {0x00, delay, 0x00, delay};
	//        rddata=modified_bits_by_value(rddata, delay, 7,0);
	//        rddata=modified_bits_by_value(rddata, delay, 23,16);
	//original      REGWR  (172 + PHY_BASE_ADDR, rddata, 0);
	mmio_wr32(4 * (172 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	uartlog("byte1 dqs delay\n");
	//rddata = {0x00, delay, 0x00, delay};
	//        rddata=modified_bits_by_value(rddata, delay, 7,0);
	//        rddata=modified_bits_by_value(rddata, delay, 23,16);
	//original      REGWR  (173 + PHY_BASE_ADDR, rddata, 0);
	mmio_wr32(4 * (173 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
#endif
	uartlog("dll_sw_clr\n");
	//param_phyd_dll_sw_clr          <= `PI_SD int_regin[7];
	////original      REGREAD(169 + PHY_BASE_ADDR, rddata);
	ddr_mmio_rd32(4 * (169 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	//rddata[7] = 1;
	//rddata[6:3] = 0;
	rddata = modified_bits_by_value(rddata, 0, 6, 3);
	rddata = modified_bits_by_value(rddata, 1, 7, 7);
	//original      REGWR  (169 + PHY_BASE_ADDR, rddata, 0);
	mmio_wr32(4 * (169 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	//rddata[7] = 0;
	rddata = modified_bits_by_value(rddata, 0, 7, 7);
	//original      REGWR  (169 + PHY_BASE_ADDR, rddata, 0);
	mmio_wr32(4 * (169 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	opdelay(200);

	uartlog("wait ack\n");
	while (1) {
		//param_phyd_to_reg_init_upd_ack[31:0]
		////original          REGREAD(1109 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (1109 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		if (rddata == 0xffffffff)
			break;
	}
	uartlog("dll_sw_clr finish\n");
}

void bm_bist_rx_deskew_delay(uint32_t delay)
{
	uartlog("bm_bist_rx_deskew_delay\n");
	//deskew delay
	opdelay(10);
	uartlog("byte0 deskew delay\n");
	//rddata = {1'b0, delay, 1'b0, delay, 1'b0, delay, 1'b0, delay};
	rddata = 0;
	rddata = modified_bits_by_value(rddata, delay, 6, 0);
	rddata = modified_bits_by_value(rddata, delay, 14, 8);
	rddata = modified_bits_by_value(rddata, delay, 22, 16);
	rddata = modified_bits_by_value(rddata, delay, 30, 24);
	//original      REGWR  (202 + PHY_BASE_ADDR, rddata, 0);
	mmio_wr32(4 * (202 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	//original      REGWR  (203 + PHY_BASE_ADDR, rddata, 0);
	mmio_wr32(4 * (203 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	//rddata = {0, 1'b0, delay};
	rddata = 0;
	rddata = modified_bits_by_value(rddata, delay, 6, 0);
	//original      REGWR  (204 + PHY_BASE_ADDR, rddata, 0);
	mmio_wr32(4 * (204 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	uartlog("byte1 deskew delay\n");
	//rddata = {1'b0, delay, 1'b0, delay, 1'b0, delay, 1'b0, delay};
	rddata = modified_bits_by_value(rddata, delay, 6, 0);
	rddata = modified_bits_by_value(rddata, delay, 14, 8);
	rddata = modified_bits_by_value(rddata, delay, 22, 16);
	rddata = modified_bits_by_value(rddata, delay, 30, 24);
	//original      REGWR  (205 + PHY_BASE_ADDR, rddata, 0);
	mmio_wr32(4 * (205 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	//original      REGWR  (206 + PHY_BASE_ADDR, rddata, 0);
	mmio_wr32(4 * (206 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	//rddata = {0, 1'b0, delay};
	rddata = 0;
	rddata = modified_bits_by_value(rddata, delay, 6, 0);
	//original      REGWR  (207 + PHY_BASE_ADDR, rddata, 0);
	mmio_wr32(4 * (207 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
#ifndef X16_MODE
	uartlog("byte0 deskew delay\n");
	//rddata = {1'b0, delay, 1'b0, delay, 1'b0, delay, 1'b0, delay};
	rddata = modified_bits_by_value(rddata, delay, 6, 0);
	rddata = modified_bits_by_value(rddata, delay, 14, 8);
	rddata = modified_bits_by_value(rddata, delay, 22, 16);
	rddata = modified_bits_by_value(rddata, delay, 30, 24);
	//original      REGWR  (208 + PHY_BASE_ADDR, rddata, 0);
	mmio_wr32(4 * (208 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	//original      REGWR  (209 + PHY_BASE_ADDR, rddata, 0);
	mmio_wr32(4 * (209 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	//rddata = {0, 1'b0, delay};
	rddata = 0;
	rddata = modified_bits_by_value(rddata, delay, 6, 0);
	//original      REGWR  (210 + PHY_BASE_ADDR, rddata, 0);
	mmio_wr32(4 * (210 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	uartlog("byte1 deskew delay\n");
	//rddata = {1'b0, delay, 1'b0, delay, 1'b0, delay, 1'b0, delay};
	rddata = modified_bits_by_value(rddata, delay, 6, 0);
	rddata = modified_bits_by_value(rddata, delay, 14, 8);
	rddata = modified_bits_by_value(rddata, delay, 22, 16);
	rddata = modified_bits_by_value(rddata, delay, 30, 24);
	//original      REGWR  (211 + PHY_BASE_ADDR, rddata, 0);
	mmio_wr32(4 * (211 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	//original      REGWR  (212 + PHY_BASE_ADDR, rddata, 0);
	mmio_wr32(4 * (212 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	//rddata = {0, 1'b0, delay};
	rddata = 0;
	rddata = modified_bits_by_value(rddata, delay, 6, 0);
	//original      REGWR  (213 + PHY_BASE_ADDR, rddata, 0);
	mmio_wr32(4 * (213 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
#endif
	uartlog("dll_sw_clr\n");
	//param_phyd_dll_sw_clr          <= `PI_SD int_regin[7];
	////original      REGREAD(169 + PHY_BASE_ADDR, rddata);
	ddr_mmio_rd32(4 * (169 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	//rddata[7] = 1;
	//rddata[6:3] = 0;
	rddata = modified_bits_by_value(rddata, 1, 7, 7);
	rddata = modified_bits_by_value(rddata, 0, 6, 3);
	//original      REGWR  (169 + PHY_BASE_ADDR, rddata, 0);
	mmio_wr32(4 * (169 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	//rddata[7] = 0;
	rddata = modified_bits_by_value(rddata, 0, 7, 7);
	//original      REGWR  (169 + PHY_BASE_ADDR, rddata, 0);
	mmio_wr32(4 * (169 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	opdelay(200);
	uartlog("wait ack\n");
	while (1) {
		//param_phyd_to_reg_init_upd_ack[31:0]
		////original          REGREAD(1109 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (1109 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		if (rddata == 0xffffffff)
			break;
	}
	uartlog("dll_sw_clr finish\n");
}

void bm_dqsosc_upd(uint32_t dqsosc_offset)
{
	uartlog("bm_dqsosc_upd\n");
	bm_synp_mrw_lp4(0xff, 0x4b);
	opdelay(5);
	bm_synp_mrw_lp4(0xff, 0x4d);
	while (1) {
		// read dqsosc_updata
		//param_phyd_dqsosc_update       <= `PI_SD int_regin[16];
		////original             REGREAD(238 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (238 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		if (get_bits_from_value(rddata, 16, 16) == 0)
			break;
	}
	uartlog("dqsosc_updata = 0\n");
	//param_phyd_dqsosc_offset[7:0]  <= `PI_SD int_regin[7:0]
	//param_phyd_dqsosc_update       <= `PI_SD int_regin[16];
	////original           REGREAD(238 + PHY_BASE_ADDR, rddata);
	ddr_mmio_rd32(4 * (238 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	//rddata[7:0] = dqsosc_offset;
	//rddata[16]  = 1'b0;
	rddata = modified_bits_by_value(rddata, dqsosc_offset, 7, 0);
	rddata = modified_bits_by_value(rddata, 0, 16, 16);
	//original           REGWR  (238 + PHY_BASE_ADDR, rddata, 0);
	mmio_wr32(4 * (238 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	////original           REGREAD(238 + PHY_BASE_ADDR, rddata);
	ddr_mmio_rd32(4 * (238 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	//rddata[16]  = 1'b1;
	rddata = modified_bits_by_value(rddata, 1, 16, 16);
	//original           REGWR  (238 + PHY_BASE_ADDR, rddata, 0);
	mmio_wr32(4 * (238 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	uartlog("DQ delay line update\n");
	bm_synp_mrr_lp4(0x12);
	uartlog("MRR DQS OSC count LSB\n");
	ddr_mmio_rd32(0x0800a000 + 0x244, rddata);
	ddr_mmio_rd32(0x0800a000 + 0x248, rddata);
	ddr_mmio_rd32(0x0800a000 + 0x24c, rddata);
	ddr_mmio_rd32(0x0800a000 + 0x250, rddata);
	bm_synp_mrr_lp4(0x13);
	uartlog("MRR DQS OSC count MSB\n");
	ddr_mmio_rd32(0x0800a000 + 0x244, rddata);
	ddr_mmio_rd32(0x0800a000 + 0x248, rddata);
	ddr_mmio_rd32(0x0800a000 + 0x24c, rddata);
	ddr_mmio_rd32(0x0800a000 + 0x250, rddata);
}

void bm_synp_mrw_lp4(uint32_t addr, uint32_t data)
{
	uartlog("bm_synp_mrw_lp4\n");
	// Poll MRSTAT.mr_wr_busy until it is 0
	while (1) {
		ddr_mmio_rd32(cfg_base + 0x18, rddata);
		uartlog("Poll MRSTAT.mr_wr_busy until it is 0\n");
		//} while (rddata[0] != 0);
		if (get_bits_from_value(rddata, 0, 0) == 0)
			break;
	}
	uartlog("lp4 Poll MRSTAT.mr_wr_busy finish\n");
	// Write the MRCTRL0.mr_type, MRCTRL0.mr_addr, MRCTRL0.mr_rank and (for MRWs) MRCTRL1.mr_data
	//rddata = 0;
	//rddata[0] = 0;       // mr_type  0:write   1:read
	//rddata[5:4] = 1;     // mr_rank
	rddata = modified_bits_by_value(rddata, 0, 31, 0);
	rddata = modified_bits_by_value(rddata, 0, 0, 0);
	rddata = modified_bits_by_value(rddata, 1, 5, 4);
	mmio_wr32(cfg_base + 0x10, rddata);
	uartlog("lp4 Write the MRCTRL0\n");
	//    rddata[31:0] = 0;
	//    rddata[15:8] = addr;     // mr_addr
	//    rddata[ 7:0] = data;     // mr_data
	rddata = modified_bits_by_value(rddata, 0, 31, 0);
	rddata = modified_bits_by_value(rddata, addr, 15, 8);
	rddata = modified_bits_by_value(rddata, data, 7, 0);
	mmio_wr32(cfg_base + 0x14, rddata);
	uartlog("lp4 Write the MRCTRL1\n");
	// Write MRCTRL0.mr_wr to 1
	ddr_mmio_rd32(cfg_base + 0x10, rddata);
	//rddata[31] = 1;
	rddata = modified_bits_by_value(rddata, 1, 31, 31);
	mmio_wr32(cfg_base + 0x10, rddata);
	uartlog("lp4 Write MRCTRL0.mr_wr to 1\n");
	while (1) {
		ddr_mmio_rd32(cfg_base + 0x18, rddata);
		//} while (rddata[0] != 0);
		if (get_bits_from_value(rddata, 0, 0) == 0)
			break;
	}
	uartlog("lp4 Poll MRSTAT.mr_wr_busy finish\n");
}

void bm_synp_mrw(uint32_t addr, uint32_t data)
{
	uint32_t init_dis_auto_zq;
	uartlog("bm_synp_mrw\n");
	// ZQCTL0.dis_auto_zq to 1.
	ddr_mmio_rd32(cfg_base + 0x180, rddata);
	// if (rddata[31] == 1'b0) {
	if (get_bits_from_value(rddata, 31, 31) == 0) {
		init_dis_auto_zq = 0;
		//rddata[31] = 1;
		rddata = modified_bits_by_value(rddata, 1, 31, 31);
		mmio_wr32(cfg_base + 0x180, rddata);
		uartlog("non-lp4 Write ZQCTL0.dis_auto_zq to 1\n");
		opdelay(256);
		uartlog("Wait tzqcs = 128 cycles\n");
	}
	// Poll MRSTAT.mr_wr_busy until it is 0
	uartlog("Poll MRSTAT.mr_wr_busy until it is 0\n");
	rddata = 0;
	while (1) {
		ddr_mmio_rd32(cfg_base + 0x18, rddata);
		if (get_bits_from_value(rddata, 0, 0) == 0)
			break;
	}
	//} while (rddata[0] != 0);
	uartlog("non-lp4 Poll MRSTAT.mr_wr_busy finish\n");
	// Write the MRCTRL0.mr_type, MRCTRL0.mr_addr, MRCTRL0.mr_rank and (for MRWs) MRCTRL1.mr_data
	//rddata[31:0]  = 0;
	//rddata[0]     = 0;       // mr_type  0:write   1:read
	//rddata[5:4]   = 1;       // mr_rank
	//rddata[15:12] = addr;    // mr_addr
	rddata = modified_bits_by_value(rddata, 0, 31, 0);
	rddata = modified_bits_by_value(rddata, 0, 0, 0);
	rddata = modified_bits_by_value(rddata, 1, 5, 4);
	rddata = modified_bits_by_value(rddata, addr, 15, 12);
	mmio_wr32(cfg_base + 0x10, rddata);
	uartlog("non-lp4 Write the MRCTRL0\n");
	//rddata[31:0] = 0;
	//rddata[15:0] = data;     // mr_data
	rddata = modified_bits_by_value(rddata, 0, 31, 0);
	rddata = modified_bits_by_value(rddata, data, 31, 0);
	mmio_wr32(cfg_base + 0x14, rddata);
	uartlog("non-lp4 Write the MRCTRL1\n");
	// Write MRCTRL0.mr_wr to 1
	ddr_mmio_rd32(cfg_base + 0x10, rddata);
	//rddata[31] = 1;
	rddata = modified_bits_by_value(rddata, 1, 31, 31);
	mmio_wr32(cfg_base + 0x10, rddata);
	uartlog("non-lp4 Write MRCTRL0.mr_wr to 1\n");
	if (init_dis_auto_zq == 0) {
		// ZQCTL0.dis_auto_zq to 0.
		ddr_mmio_rd32(cfg_base + 0x180, rddata);
		//rddata[31] = 0;
		rddata = modified_bits_by_value(rddata, 0, 31, 31);
		mmio_wr32(cfg_base + 0x180, rddata);
		uartlog("non-lp4 Write ZQCTL0.dis_auto_zq to 0\n");
	}
}

void bm_sync_mr_to_pi(void)
{
	uint32_t read_data;
#ifdef LP4
	uint32_t read_data1;
	uint32_t read_temp;
	uint32_t temp_value;
#endif

	uartlog("bm_sync_mr_to_pi\n");
#ifdef LP4
	ddr_mmio_rd32(cfg_base + 0xdc, read_data); // MR1/2
	uartlog("lp4_bm_sync_mr_to_pi\n");
	for (int i = 0; i < 12; i++) {
		//REGWR(PI_BASE_ADDR+268 + 8 * i, read_data[31:16], 0);
		//REGWR(PI_BASE_ADDR+269 + 8 * i, read_data[15:0], 0);
		//original                 REGWR(PI_BASE_ADDR+268 + 8 * i, get_bits_from_value(read_data, 31, 16), 0);
		mmio_wr32(4 * (PI_BASE_ADDR + 268 + 8 * i) + CADENCE_PHYD, get_bits_from_value(read_data, 31, 16));
		//original                 REGWR(PI_BASE_ADDR+269 + 8 * i, get_bits_from_value(read_data, 15, 0), 0);
		mmio_wr32(4 * (PI_BASE_ADDR + 269 + 8 * i) + CADENCE_PHYD, get_bits_from_value(read_data, 15, 0));
	}
	ddr_mmio_rd32(cfg_base + 0xe0, read_data); // MR3/13
	for (int i = 0; i < 12; i++) {
		//REGWR(PI_BASE_ADDR+270 + 8 * i, read_data[31:16], 0);
		//original                 REGWR(PI_BASE_ADDR+270 + 8 * i, get_bits_from_value(read_data, 31, 16), 0);
		mmio_wr32(4 * (PI_BASE_ADDR + 270 + 8 * i) + CADENCE_PHYD, get_bits_from_value(read_data, 31, 16));
	}
	////original             REGREAD(PI_BASE_ADDR+247, read_temp);
	ddr_mmio_rd32(4 * (PI_BASE_ADDR + 247) + CADENCE_PHYD, read_temp);
	//read_temp[7:0] = read_data[7:0];
	read_temp = modified_bits_by_value(read_temp, get_bits_from_value(read_data, 7, 0), 7, 0);
	//original             REGWR(PI_BASE_ADDR+247, read_temp, 0);
	mmio_wr32(4 * (PI_BASE_ADDR + 247) + CADENCE_PHYD, read_temp);
	////original             REGREAD(PI_BASE_ADDR+249, read_temp);
	ddr_mmio_rd32(4 * (PI_BASE_ADDR + 249) + CADENCE_PHYD, read_temp);
	//read_temp[15:8] = read_data[7:0];
	read_temp = modified_bits_by_value(read_temp, get_bits_from_value(read_data, 7, 0), 15, 8);
	//original             REGWR(PI_BASE_ADDR+249, read_temp, 0);
	mmio_wr32(4 * (PI_BASE_ADDR + 249) + CADENCE_PHYD, read_temp);
	////original             REGREAD(PI_BASE_ADDR+252, read_temp);
	ddr_mmio_rd32(4 * (PI_BASE_ADDR + 252) + CADENCE_PHYD, read_temp);
	//read_temp[7:0] = read_data[7:0];
	read_temp = modified_bits_by_value(read_temp, get_bits_from_value(read_data, 7, 0), 7, 0);
	//original             REGWR(PI_BASE_ADDR+252, read_temp, 0);
	mmio_wr32(4 * (PI_BASE_ADDR + 252) + CADENCE_PHYD, read_temp);
	////original             REGREAD(PI_BASE_ADDR+254, read_temp);
	ddr_mmio_rd32(4 * (PI_BASE_ADDR + 254) + CADENCE_PHYD, read_temp);
	//read_temp[15:8] = read_data[7:0];
	read_temp = modified_bits_by_value(read_temp, get_bits_from_value(read_data, 7, 0), 15, 8);
	//original             REGWR(PI_BASE_ADDR+254, read_temp, 0);
	mmio_wr32(4 * (PI_BASE_ADDR + 254) + CADENCE_PHYD, read_temp);
	ddr_mmio_rd32(cfg_base + 0xe8, read_data); // MR11/12
	ddr_mmio_rd32(cfg_base + 0xec, read_data1); // MR22/14
	for (int i = 0; i < 12; i++) {
		// REGWR(PI_BASE_ADDR+273 + 8 * i, {read_data[23:16],0x0}, 0);
		temp_value = 0;
		temp_value = modified_bits_by_value(temp_value, get_bits_from_value(read_data, 23, 16), 31, 24);
		//original                 REGWR(PI_BASE_ADDR+273 + 8 * i, temp_value, 0);
		mmio_wr32(4 * (PI_BASE_ADDR + 273 + 8 * i) + CADENCE_PHYD, temp_value);
		//REGWR(PI_BASE_ADDR+274 + 8 * i, {read_data1[23:16],read_data1[7:0],read_data[7:0]}, 0);
		temp_value = 0;
		temp_value = modified_bits_by_value(temp_value, get_bits_from_value(read_data, 7, 0), 7, 0);
		temp_value = modified_bits_by_value(temp_value, get_bits_from_value(read_data1, 7, 0), 15, 8);
		temp_value = modified_bits_by_value(temp_value, get_bits_from_value(read_data1, 23, 16), 23, 16);
		//original                 REGWR(PI_BASE_ADDR+274 + 8 * i, temp_value, 0);
		mmio_wr32(4 * (PI_BASE_ADDR + 274 + 8 * i) + CADENCE_PHYD, temp_value);
	}
#endif
#ifdef DDR3
	uartlog("ddr3_bm_sync_mr_to_pi\n");
	ddr_mmio_rd32(cfg_base + 0xdc, read_data); // MR0/1
	//if ({read_data[9],read_data[6],read_data[2]} == 3'b0) {
	//    {read_data[9],read_data[6],read_data[2]} = 3'b001;
	//}
	if ((get_bits_from_value(read_data, 9, 9) == 0) && (get_bits_from_value(read_data, 6, 6) == 0) && (get_bits_from_value(read_data, 9, 9) == 0)) {
		//{read_data[9],read_data[6],read_data[2]} = 3'b001;
		read_data = modified_bits_by_value(read_data, 1, 2, 2);
	}
	for (int i = 0; i < 12; i++) {
		//REGWR(PI_BASE_ADDR+267 + 8 * i, read_data[31:16], 0);
		//original                 REGWR(PI_BASE_ADDR+267 + 8 * i, get_bits_from_value(read_data, 31, 16), 0);
		mmio_wr32(4 * (PI_BASE_ADDR + 267 + 8 * i) + CADENCE_PHYD, get_bits_from_value(read_data, 31, 16));
		//REGWR(PI_BASE_ADDR+268 + 8 * i, read_data[15:0], 0);
		//original                 REGWR(PI_BASE_ADDR+268 + 8 * i, get_bits_from_value(read_data, 15, 0), 0);
		mmio_wr32(4 * (PI_BASE_ADDR + 268 + 8 * i) + CADENCE_PHYD, get_bits_from_value(read_data, 15, 0));
	}
	ddr_mmio_rd32(cfg_base + 0xe0, read_data); // MR2/3
	for (int i = 0; i < 12; i++) {
		//REGWR(PI_BASE_ADDR+269 + 8 * i, read_data[31:16], 0);
		//original                 REGWR(PI_BASE_ADDR+269 + 8 * i, get_bits_from_value(read_data , 31, 16), 0);
		mmio_wr32(4 * (PI_BASE_ADDR + 269 + 8 * i) + CADENCE_PHYD, get_bits_from_value(read_data, 31, 16));
		//REGWR(PI_BASE_ADDR+270 + 8 * i, read_data[15:0], 0);
		//original                 REGWR(PI_BASE_ADDR+270 + 8 * i, get_bits_from_value(read_data , 15, 0), 0);
		mmio_wr32(4 * (PI_BASE_ADDR + 270 + 8 * i) + CADENCE_PHYD, get_bits_from_value(read_data, 15, 0));
	}
#endif
#ifdef DDR4
	uartlog("ddr4 bm_sync_mr_to_pi\n");
	ddr_mmio_rd32(cfg_base + 0xdc, read_data); // MR0/1
	//read_data[24] = 1'b0;       // DLL reset disable
	read_data = modified_bits_by_value(read_data, 0, 24, 24); // DLL reset disable
	for (int i = 0; i < 12; i++) {
		//    REGWR(PI_BASE_ADDR+267 + 8 * i, read_data[31:16], 0);
		mmio_wr32(4 * (PI_BASE_ADDR + 267 + 8 * i) + CADENCE_PHYD, get_bits_from_value(read_data, 31, 16));
		//    REGWR(PI_BASE_ADDR+268 + 8 * i, read_data[15:0], 0);
		mmio_wr32(4 * (PI_BASE_ADDR + 268 + 8 * i) + CADENCE_PHYD, get_bits_from_value(read_data, 15, 0));
	}
	ddr_mmio_rd32(cfg_base + 0xe0, read_data); // MR2/3
	for (int i = 0; i < 12; i++) {
		//    REGWR(PI_BASE_ADDR+269 + 8 * i, read_data[31:16], 0);
		mmio_wr32(4 * (PI_BASE_ADDR + 269 + 8 * i) + CADENCE_PHYD, get_bits_from_value(read_data, 31, 16));
		//    REGWR(PI_BASE_ADDR+270 + 8 * i, read_data[15:0], 0);
		mmio_wr32(4 * (PI_BASE_ADDR + 270 + 8 * i) + CADENCE_PHYD, get_bits_from_value(read_data, 15, 0));
	}
	ddr_mmio_rd32(cfg_base + 0xe8, read_data); // MR4/5
	for (int i = 0; i < 12; i++) {
		//REGWR(PI_BASE_ADDR+271 + 8 * i, read_data[31:16], 0);
		//REGWR(PI_BASE_ADDR+272 + 8 * i, read_data[15:0], 0);
		mmio_wr32(4 * (PI_BASE_ADDR + 271 + 8 * i) + CADENCE_PHYD, get_bits_from_value(read_data, 31, 16));
		mmio_wr32(4 * (PI_BASE_ADDR + 272 + 8 * i) + CADENCE_PHYD, get_bits_from_value(read_data, 15, 0));
	}
	ddr_mmio_rd32(cfg_base + 0xec, read_data); // MR6
	for (int i = 0; i < 12; i++) {
		//    REGWR(PI_BASE_ADDR+273 + 8 * i, read_data[15:0], 0);
		mmio_wr32(4 * (PI_BASE_ADDR + 273 + 8 * i) + CADENCE_PHYD, get_bits_from_value(read_data, 15, 0));
	}
#endif
}

void bm_synp_mrs_ddr3_ddr4(uint32_t addr, uint32_t data)
{
	// Poll MRSTAT.mr_wr_busy until it is 0
	uartlog("Poll MRSTAT.mr_wr_busy until it is 0\n");
	rddata = 0;
	while (1) {
		ddr_mmio_rd32(cfg_base + 0x18, rddata);
		//} while (rddata[0] != 0);
		if (get_bits_from_value(rddata, 0, 0) == 0)
			break;
	}
	uartlog("lp4 Poll MRSTAT.mr_wr_busy finish\n");
	// Write the MRCTRL0.mr_type, MRCTRL0.mr_addr, MRCTRL0.mr_rank and (for MRWs) MRCTRL1.mr_data
	//         rddata[31:0] = 0;
	//         rddata[0] = 0;       // mr_type  0:write   1:read
	//         rddata[5:4] = 1;     // mr_rank
	//         rddata[15:12] = {1'b0,addr};
	rddata = modified_bits_by_value(rddata, 0, 31, 0); //
	rddata = modified_bits_by_value(rddata, 0, 0, 0); //
	rddata = modified_bits_by_value(rddata, 1, 5, 4); //
	rddata = modified_bits_by_value(rddata, addr, 15, 12); //
	mmio_wr32(cfg_base + 0x10, rddata);
	uartlog("lp4 Write the MRCTRL0\n");
	//rddata[31:0] = 0;
	//rddata[17:0] = data;     // mr_data
	rddata = modified_bits_by_value(rddata, 0, 31, 0); //
	rddata = modified_bits_by_value(rddata, data, 17, 0); //
	mmio_wr32(cfg_base + 0x14, rddata);
	uartlog("lp4 Write the MRCTRL1\n");
	// Write MRCTRL0.mr_wr to 1
	ddr_mmio_rd32(cfg_base + 0x10, rddata);
	//rddata[31] = 1;
	rddata = modified_bits_by_value(rddata, 1, 31, 31); //
	mmio_wr32(cfg_base + 0x10, rddata);
	uartlog("lp4 Write MRCTRL0.mr_wr to 1\n");
	while (1) {
		ddr_mmio_rd32(cfg_base + 0x18, rddata);
		if (get_bits_from_value(rddata, 0, 0) == 0)
			break;
	}
	uartlog("lp4 Poll MRSTAT.mr_wr_busy finish\n");
}

void bm_soft_zq(void)
{
#ifdef LP4
	uint32_t read_data;
	uint32_t read_tmp;
	uint32_t zq_ori;

	uartlog("lp4 bm_soft_zq\n");
	ddr_mmio_rd32(cfg_base + 0xec, read_data); // MR22/14
	//bm_synp_mrw_lp4(22, read_data[23:16]);
	bm_synp_mrw_lp4(22, get_bits_from_value(read_data, 23, 16));
	//software ZQ
	ddr_mmio_rd32(cfg_base + 0x180, zq_ori);
	ddr_mmio_rd32(0x4c + CADENCE_PHYD_APB, read_tmp);
	read_data = zq_ori;
	//read_data[31] = 1'b1;                       // dis_auto_zq;
	read_data = modified_bits_by_value(read_data, 1, 31, 31);
	// dis_auto_zq;
	// if (read_data[9:0] != 4) {
	if (get_bits_from_value(read_tmp, 5, 4) == 0) {
		//read_data[26:16] = read_data[26:16] / 32 + 1;
		read_data = modified_bits_by_value(read_data, ((get_bits_from_value(read_data, 26, 16) / 32) + 1), 26, 16);
		//if (read_data[9:0] / 32 + 1 < 4)
		if (((get_bits_from_value(read_data, 9, 0) / 32) + 1) < 4) {
			// read_data[9:0] = 4;
			read_data = modified_bits_by_value(read_data, 4, 9, 0);
		} else
			//read_data[9:0] = read_data[9:0] / 32 + 1;
			read_data = modified_bits_by_value(read_data, ((get_bits_from_value(read_data, 9, 0) / 32) + 1), 9, 0);
	}
	mmio_wr32(cfg_base + 0x180, read_data);
	mmio_wr32(cfg_base + 0x30c, 0x10); // zq_calib_short
	//opdelay(10)us;
	opdelay(10000);
	mmio_wr32(cfg_base + 0x180, zq_ori);
#endif
}

void bm_soft_mr(void)
{
#ifdef LP4
	uint32_t read_data;
	// uint32_t read_tmp;
	// uint32_t zq_ori;

	uartlog("bm_soft_mr\n");
	// write FSP1
	// MR3 = [31:16]   MR13 = [15:0]
	read_data = mmio_rd32(cfg_base + 0xe0);
	read_data = modified_bits_by_value(read_data, 1, 6, 6);
	bm_synp_mrw_lp4(13, get_bits_from_value(read_data, 7, 0));
	bm_synp_mrw_lp4(3, get_bits_from_value(read_data, 23, 16));
	// MR1 = [31:16]   MR2 = [15:0]
	read_data = mmio_rd32(cfg_base + 0xdc);
	bm_synp_mrw_lp4(2, get_bits_from_value(read_data, 7, 0));
	bm_synp_mrw_lp4(1, get_bits_from_value(read_data, 23, 16));
	// MR11 = [31:16]   MR12 = [15:0]
	read_data = mmio_rd32(cfg_base + 0xe8);
	bm_synp_mrw_lp4(12, get_bits_from_value(read_data, 7, 0));
	bm_synp_mrw_lp4(11, get_bits_from_value(read_data, 23, 16));
	// MR22 = [31:16]   MR14 = [15:0]
	read_data = mmio_rd32(cfg_base + 0xec);
	bm_synp_mrw_lp4(14, get_bits_from_value(read_data, 7, 0));
	bm_synp_mrw_lp4(22, get_bits_from_value(read_data, 23, 16));

	// write FSP0
	// MR3 = [31:16]   MR13 = [15:0]
	read_data = mmio_rd32(cfg_base + 0xe0);
	read_data = modified_bits_by_value(read_data, 0, 6, 6);
	bm_synp_mrw_lp4(13, get_bits_from_value(read_data, 7, 0));
	bm_synp_mrw_lp4(3, get_bits_from_value(read_data, 23, 16));
	// MR1 = [31:16]   MR2 = [15:0]
	read_data = mmio_rd32(cfg_base + 0xdc);
	bm_synp_mrw_lp4(2, get_bits_from_value(read_data, 7, 0));
	bm_synp_mrw_lp4(1, get_bits_from_value(read_data, 23, 16));
	// MR11 = [31:16]   MR12 = [15:0]
	read_data = mmio_rd32(cfg_base + 0xe8);
	bm_synp_mrw_lp4(12, get_bits_from_value(read_data, 7, 0));
	bm_synp_mrw_lp4(11, get_bits_from_value(read_data, 23, 16));
	// MR22 = [31:16]   MR14 = [15:0]
	read_data = mmio_rd32(cfg_base + 0xec);
	bm_synp_mrw_lp4(14, get_bits_from_value(read_data, 7, 0));
	bm_synp_mrw_lp4(22, get_bits_from_value(read_data, 23, 16));
#endif //LP4
}

void chg_pll_freq(void)
{
	//Change PLL frequency
	uartlog("chg_pll_freq\n");
	//original       apb_read(0x04, apb_rddata);
	ddr_mmio_rd32(0x04 + CADENCE_PHYD_APB, apb_rddata);
	//original       apb_rddata[0]   = 0;    //TOP_REG_RESETZ_DIV
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 0, 0); //
	////original       apb_write(0x04, apb_rddata);
	mmio_wr32(0x04 + CADENCE_PHYD_APB, apb_rddata);
	//original       apb_read(0x08, apb_rddata);
	ddr_mmio_rd32(0x08 + CADENCE_PHYD_APB, apb_rddata);
	//original       apb_rddata[0]   = 0;    //TOP_REG_RESETZ_DQS
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 0, 0); //
	////original       apb_write(0x08, apb_rddata);
	mmio_wr32(0x08 + CADENCE_PHYD_APB, apb_rddata);
	//original       apb_read(0x0C, apb_rddata);
	ddr_mmio_rd32(0x0C + CADENCE_PHYD_APB, apb_rddata);
	//original       apb_rddata[7]   = 0;    //TOP_REG_DDRPLL_MAS_RSTZ_DIV
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 7, 7); //
	////original       apb_write(0x0C, apb_rddata);
	mmio_wr32(0x0C + CADENCE_PHYD_APB, apb_rddata);
	uartlog("RSTZ_DIV=0\n");
	//TOP_REG_EN_PLL_SPEED_CHG <= #RD (~pwstrb_mask[0] & TOP_REG_EN_PLL_SPEED_CHG) |  pwstrb_mask_pwdata[0];
	//TOP_REG_CUR_PLL_SPEED   [1:0] <= #RD (~pwstrb_mask[5:4] & TOP_REG_CUR_PLL_SPEED[1:0]) |  pwstrb_mask_pwdata[5:4];
	//TOP_REG_NEXT_PLL_SPEED  [1:0] <= #RD (~pwstrb_mask[9:8] & TOP_REG_NEXT_PLL_SPEED[1:0]) |  pwstrb_mask_pwdata[9:8];
	//original       apb_read(0x4c, apb_rddata);
	ddr_mmio_rd32(0x4c + CADENCE_PHYD_APB, apb_rddata);
	//EN_PLL_SPEED_CHG = apb_rddata[0];
	EN_PLL_SPEED_CHG = get_bits_from_value(apb_rddata, 0, 0);
	//CUR_PLL_SPEED    = apb_rddata[5:4];
	CUR_PLL_SPEED = get_bits_from_value(apb_rddata, 5, 4);
	//NEXT_PLL_SPEED   = apb_rddata[9:8];
	NEXT_PLL_SPEED = get_bits_from_value(apb_rddata, 9, 8);
	if (EN_PLL_SPEED_CHG) {
		if (NEXT_PLL_SPEED == 0) { // next clk_div40
			//original            apb_rddata[5:4]   = NEXT_PLL_SPEED;
			apb_rddata = modified_bits_by_value(apb_rddata, NEXT_PLL_SPEED, 5, 4); //
			//original            apb_rddata[9:8]   = CUR_PLL_SPEED;
			apb_rddata = modified_bits_by_value(apb_rddata, CUR_PLL_SPEED, 9, 8); //
			////original            apb_write(0x4c, apb_rddata);
			mmio_wr32(0x4c + CADENCE_PHYD_APB, apb_rddata);
			clk_div40();
			uartlog("clk_div40\n");
		} else {
			if (NEXT_PLL_SPEED == 0x2) { // next clk normal
				//original              apb_rddata[5:4]   = NEXT_PLL_SPEED;
				apb_rddata = modified_bits_by_value(apb_rddata, NEXT_PLL_SPEED, 5, 4); //
				//original              apb_rddata[9:8]   = CUR_PLL_SPEED;
				apb_rddata = modified_bits_by_value(apb_rddata, CUR_PLL_SPEED, 9, 8); //
				////original              apb_write(0x4c, apb_rddata);
				mmio_wr32(0x4c + CADENCE_PHYD_APB, apb_rddata);
				clk_normal();
				uartlog("clk_normal\n");
			} else {
				if (NEXT_PLL_SPEED == 0x1) { // next clk normal div_2
					//original                apb_rddata[5:4]   = NEXT_PLL_SPEED;
					apb_rddata = modified_bits_by_value(apb_rddata, NEXT_PLL_SPEED, 5, 4); //
					//original                apb_rddata[9:8]   = CUR_PLL_SPEED;
					apb_rddata = modified_bits_by_value(apb_rddata, CUR_PLL_SPEED, 9, 8); //
					////original                apb_write(0x4c, apb_rddata);
					mmio_wr32(0x4c + CADENCE_PHYD_APB, apb_rddata);
					clk_div2();
					uartlog("clk_div2\n");
				}
			}
		}
		//         opdelay(100000);  //  1000ns
	}
	//original       apb_read(0x04, apb_rddata);
	ddr_mmio_rd32(0x04 + CADENCE_PHYD_APB, apb_rddata);
	//original       apb_rddata[0]   = 1;    //TOP_REG_RESETZ_DIV
	apb_rddata = modified_bits_by_value(apb_rddata, 1, 0, 0); //
	////original       apb_write(0x04, apb_rddata);
	mmio_wr32(0x04 + CADENCE_PHYD_APB, apb_rddata);
	//original       apb_read(0x0C, apb_rddata);
	ddr_mmio_rd32(0x0C + CADENCE_PHYD_APB, apb_rddata);
	//original       apb_rddata[7]   = 1;    //TOP_REG_DDRPLL_MAS_RSTZ_DIV
	apb_rddata = modified_bits_by_value(apb_rddata, 1, 7, 7); //
	////original       apb_write(0x0C, apb_rddata);
	mmio_wr32(0x0C + CADENCE_PHYD_APB, apb_rddata);
	uartlog("RSTZ_DIV=1\n");
	//original       apb_read(0x08, apb_rddata);
	ddr_mmio_rd32(0x08 + CADENCE_PHYD_APB, apb_rddata);
	//original       apb_rddata[0]   = 1;    //TOP_REG_RESETZ_DQS
	apb_rddata = modified_bits_by_value(apb_rddata, 1, 0, 0); //
	////original       apb_write(0x08, apb_rddata);
	mmio_wr32(0x08 + CADENCE_PHYD_APB, apb_rddata);
	uartlog("TOP_REG_RESETZ_DQS\n");
#ifdef REAL_LOCK
	//original         apb_rddata[15]=0;
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 15, 15); //
	//while(~apb_rddata[15]) {
	while (get_bits_from_value(apb_rddata, 15, 15) == 0) {
		//original           apb_read(0x10, apb_rddata);
		ddr_mmio_rd32(0x10 + CADENCE_PHYD_APB, apb_rddata);
		opdelay(200);
	}
#else
#endif
}

void dll_cal(void)
{
		uartlog("dll_cal\n");
		//TOP_REG_EN_PLL_SPEED_CHG <= #RD (~pwstrb_mask[0] & TOP_REG_EN_PLL_SPEED_CHG) |  pwstrb_mask_pwdata[0];
		//TOP_REG_CUR_PLL_SPEED   [1:0] <= #RD (~pwstrb_mask[5:4] & TOP_REG_CUR_PLL_SPEED[1:0]) |  pwstrb_mask_pwdata[5:4];
		//TOP_REG_NEXT_PLL_SPEED  [1:0] <= #RD (~pwstrb_mask[9:8] & TOP_REG_NEXT_PLL_SPEED[1:0]) |  pwstrb_mask_pwdata[9:8];
		ddr_mmio_rd32(0x4c + CADENCE_PHYD_APB, apb_rddata);
		EN_PLL_SPEED_CHG = get_bits_from_value(apb_rddata, 0, 0);
		CUR_PLL_SPEED = get_bits_from_value(apb_rddata, 5, 4);
		NEXT_PLL_SPEED = get_bits_from_value(apb_rddata, 9, 8);
		if (CUR_PLL_SPEED != 0) { //only do calibration and update when high speed
			//param_phyd_dll_rx_start_cal <= int_regin[17];
			//param_phyd_dll_tx_start_cal <= int_regin[21];
			ddr_mmio_rd32(4 * (133 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
			rddata = modified_bits_by_value(rddata, 0, 17, 17);
			rddata = modified_bits_by_value(rddata, 0, 21, 21);
			mmio_wr32(4 * (133 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
			//param_phyd_dll_rx_start_cal <= int_regin[17];
			//param_phyd_dll_tx_start_cal <= int_regin[21];
			ddr_mmio_rd32(4 * (133 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
			rddata = modified_bits_by_value(rddata, 1, 17, 17);
			rddata = modified_bits_by_value(rddata, 1, 21, 21);
			mmio_wr32(4 * (133 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
			rddata = modified_bits_by_value(rddata, 0, 9, 9);
			while (get_bits_from_value(rddata, 9, 9) == 0)
				ddr_mmio_rd32(4 * (1062 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);

			uartlog("[DBG] DLL lock !\n");
			uartlog("DLL lock\n");
			opdelay(1000);
			//Do DLLUPD if necessary
			uartlog("Do DLLUPD\n");
			dll_cal_status();
		} else { //stop calibration and update when low speed
			//param_phyd_dll_rx_start_cal <= int_regin[17];
			//param_phyd_dll_tx_start_cal <= int_regin[21];
			ddr_mmio_rd32(4 * (133 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
			rddata = modified_bits_by_value(rddata, 0, 17, 17);
			rddata = modified_bits_by_value(rddata, 0, 21, 21);
			mmio_wr32(4 * (133 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		}
		uartlog("Do DLLCAL Finish\n");
}

void ddr_zqcal_hw_isr8(void)
{
	// int i;
	uartlog("ddr_zqcal_hw_isr8\n");
	//VDDQ_TXr = 0.6;
	//------------------------------
	// Init setting
	//------------------------------
	//param_phya_reg_zqcal_done  <= `PI_SD int_regin[14];
	////original      REGREAD(85 + PHY_BASE_ADDR, rddata);
	ddr_mmio_rd32(4 * (85 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	//     rddata[14] = 0;
	//     REGWR  (85 + PHY_BASE_ADDR, rddata, 0);
	//if (rddata[14] == 1'b0) { // initial ZQCAL
	if (get_bits_from_value(rddata, 14, 14) == 0) { // initial ZQCAL
		//param_pi_dram_class[3:0] <= `PI_SD int_regin[11:8];
		////original      REGREAD(0 + PI_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (0 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
		//dram_class = rddata[11:8];  //DDR3: 4'b0110, DDR4: 4'b1010, LPDDR3: 4'b0111, LPDDR4: 4'b1011
		dram_class = get_bits_from_value(rddata, 11, 8); //DDR3: 4'b0110, DDR4: 4'b1010, LPDDR3: 4'b0111, LPDDR4: 4'b1011
		//    ->ydh_zq_event;
		//a.
		//TOP_REG_TX_ZQ_PD        <= #RD (~pwstrb_mask[31] & TOP_REG_TX_ZQ_PD) |  pwstrb_mask_pwdata[31];
		//original      apb_read(0x40, apb_rddata);
		ddr_mmio_rd32(0x40 + CADENCE_PHYD_APB, apb_rddata);
		//original      apb_rddata[31] = 1'b0;
		apb_rddata = modified_bits_by_value(apb_rddata, 0, 31, 31); //
		////original      apb_write(0x40, apb_rddata);
		mmio_wr32(0x40 + CADENCE_PHYD_APB, apb_rddata);
		//param_phya_reg_sel_zq_high_swing  <= `PI_SD int_regin[1];
		////original      REGREAD(87 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (87 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//rddata[1] = 0;
		rddata = modified_bits_by_value(rddata, 0, 1, 1); //
		//original      REGWR  (87 + PHY_BASE_ADDR, rddata, 0);
		mmio_wr32(4 * (87 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//param_phya_reg_tx_sel_lpddr4_pmos_ph <= `PI_SD int_regin[2];
		////original      REGREAD(68 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//rddata[2] = 0;
		rddata = modified_bits_by_value(rddata, 0, 2, 2); //
		//original      REGWR  (68 + PHY_BASE_ADDR, rddata, 0);
		mmio_wr32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//    ->ydh_zq_event;
		//b.
		//param_phya_reg_tx_zq_cmp_en    <= `PI_SD int_regin[3];
		//param_phya_reg_tx_zq_cmp_offset_cal_en <= `PI_SD int_regin[4];
		//param_phya_reg_tx_zq_ph_en     <= `PI_SD int_regin[5];
		//param_phya_reg_tx_zq_pl_en     <= `PI_SD int_regin[6];
		////original      REGREAD(68 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//rddata[6:3] = 4'b0000;
		rddata = modified_bits_by_value(rddata, 0, 6, 3); //
		//original      REGWR  (68 + PHY_BASE_ADDR, rddata, 0);
		mmio_wr32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//param_phya_reg_tx_zq_step2_en  <= `PI_SD int_regin[16];
		////original      REGREAD(87 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (87 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//rddata[16] = 0;
		rddata = modified_bits_by_value(rddata, 0, 16, 16); //
		//original      REGWR  (87 + PHY_BASE_ADDR, rddata, 0);
		mmio_wr32(4 * (87 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//param_phya_reg_tx_zq_cmp_offset[4:0] <= `PI_SD int_regin[30:26];
		////original      REGREAD(68 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//rddata[30:26] = 5'b10000;
		rddata = modified_bits_by_value(rddata, 0x10, 30, 26); //
		//original      REGWR  (68 + PHY_BASE_ADDR, rddata, 0);
		mmio_wr32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//param_phya_reg_tx_zq_sel_vref[4:0] <= `PI_SD int_regin[4:0];
		////original      REGREAD(69 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (69 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//rddata[4:0] = 5'b10000;
		rddata = modified_bits_by_value(rddata, 0x10, 4, 0); //
		//original      REGWR  (69 + PHY_BASE_ADDR, rddata, 0);
		mmio_wr32(4 * (69 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//param_phya_reg_tx_zq_drvp[5:0] <= `PI_SD int_regin[20:15];
		////original      REGREAD(68 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//rddata[20:16] = 5'b01001;
		rddata = modified_bits_by_value(rddata, 0x9, 20, 16); //
		//original      REGWR  (68 + PHY_BASE_ADDR, rddata, 0);
		mmio_wr32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//param_phya_reg_tx_zq_drvn[5:0] <= `PI_SD int_regin[13:8];
		////original      REGREAD(68 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//rddata[12:8] = 5'b01001;
		rddata = modified_bits_by_value(rddata, 0x9, 12, 8); //
		//original      REGWR  (68 + PHY_BASE_ADDR, rddata, 0);
		mmio_wr32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//    ->ydh_zq_event;
		//c.
		//param_phya_reg_vref_pd  <= `PI_SD int_regin[9];
		//     REGREAD(4 + PHY_BASE_ADDR, rddata);
		//     rddata[9] = 0;
		//     REGWR  (4 + PHY_BASE_ADDR, rddata, 0);
		//param_phya_reg_vref_sel[4:0] <= `PI_SD int_regin[14:10];
		//     REGREAD(4 + PHY_BASE_ADDR, rddata);
		//     rddata[14:10] = 5'b01111;
		//     REGWR  (4 + PHY_BASE_ADDR, rddata, 0);
		//    ->ydh_zq_event;
		//d.
		//param_phya_reg_tx_zq_en_test_aux <= `PI_SD int_regin[13];
		////original      REGREAD(85 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (85 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//rddata[13] = 0;
		rddata = modified_bits_by_value(rddata, 0, 13, 13); //
		//original      REGWR  (85 + PHY_BASE_ADDR, rddata, 0);
		mmio_wr32(4 * (85 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//param_phya_reg_tx_zq_en_test_mux  <= `PI_SD int_regin[15];
		////original      REGREAD(87 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (87 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//rddata[15] = 0;
		rddata = modified_bits_by_value(rddata, 0, 15, 15); //
		//original      REGWR  (87 + PHY_BASE_ADDR, rddata, 0);
		mmio_wr32(4 * (87 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//TOP_REG_TX_SEL_GPIO         <= #RD (~pwstrb_mask[7] & TOP_REG_TX_SEL_GPIO) |  pwstrb_mask_pwdata[7];
		//original      apb_read(0x1c, rddata);
		ddr_mmio_rd32(0x1c + CADENCE_PHYD_APB, rddata);
		//rddata[7] = 0;
		rddata = modified_bits_by_value(rddata, 0, 7, 7); //
		////original      apb_write(0x04, rddata);
		mmio_wr32(0x04 + CADENCE_PHYD_APB, rddata);
		// TOP_REG_TX_GPIO_OENZ        <= #RD (~pwstrb_mask[6] & TOP_REG_TX_GPIO_OENZ) |  pwstrb_mask_pwdata[6];
		//original      apb_read(0x1c, rddata);
		ddr_mmio_rd32(0x1c + CADENCE_PHYD_APB, rddata);
		//rddata[6] = 1;
		rddata = modified_bits_by_value(rddata, 1, 6, 6); //
		////original      apb_write(0x04, rddata);
		mmio_wr32(0x04 + CADENCE_PHYD_APB, rddata);
		//------------------------------
		// CMP offset Cal.
		//------------------------------
		//    ->ydh_zq_event;
		//param_phya_reg_tx_zq_cmp_en    <= `PI_SD int_regin[3];
		//param_phya_reg_tx_zq_cmp_offset_cal_en <= `PI_SD int_regin[4];
		//param_phya_reg_tx_zq_ph_en     <= `PI_SD int_regin[5];
		//param_phya_reg_tx_zq_pl_en     <= `PI_SD int_regin[6];
		////original      REGREAD(68 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//rddata[6:3] = 4'b0001;
		rddata = modified_bits_by_value(rddata, 1, 6, 3); //
		//original      REGWR  (68 + PHY_BASE_ADDR, rddata, 0);
		mmio_wr32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//------------------------------
		// ZQ Complete
		//------------------------------
		uartlog("wait hw_done\n");
		////original      REGREAD(1206 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (1206 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//while (!rddata[16]) {
		while (get_bits_from_value(rddata, 16, 16) == 0) {
			////original          REGREAD(1206 + PHY_BASE_ADDR, rddata);
			ddr_mmio_rd32(4 * (1206 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
			opdelay(100);
		}
		uartlog("hw_done\n");
		//param_phya_reg_zqcal_done  <= `PI_SD int_regin[14];
		////original      REGREAD(85 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (85 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//rddata[14] = 1;
		rddata = modified_bits_by_value(rddata, 1, 14, 14); //
		//original      REGWR  (85 + PHY_BASE_ADDR, rddata, 0);
		mmio_wr32(4 * (85 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//param_phya_reg_tx_zq_cmp_en    <= `PI_SD int_regin[3];
		//param_phya_reg_tx_zq_cmp_offset_cal_en <= `PI_SD int_regin[4];
		//param_phya_reg_tx_zq_ph_en     <= `PI_SD int_regin[5];
		//param_phya_reg_tx_zq_pl_en     <= `PI_SD int_regin[6];
		////original      REGREAD(68 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//rddata[6:3] = 4'b0000;
		rddata = modified_bits_by_value(rddata, 0, 6, 3); //
		//original      REGWR  (68 + PHY_BASE_ADDR, rddata, 0);
		mmio_wr32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//TOP_REG_TX_ZQ_PD        <= #RD (~pwstrb_mask[31] & TOP_REG_TX_ZQ_PD) |  pwstrb_mask_pwdata[31];
		//original      apb_read(0x40, apb_rddata);
		ddr_mmio_rd32(0x40 + CADENCE_PHYD_APB, apb_rddata);
		//original      apb_rddata[31] = 1'b1;
		apb_rddata = modified_bits_by_value(apb_rddata, 1, 31, 31); //
		////original      apb_write(0x40, apb_rddata);
		mmio_wr32(0x40 + CADENCE_PHYD_APB, apb_rddata);
		opdelay(200);
		zqcal_status();
	} else {
		uartlog("Not need ZQCAL\n");
	}
}

void ddr_zqcal_isr8(void)
{
	// uint32_t KP40_GOLDEN;
	// uint32_t KN40_GOLDEN;
	int i;

	//VDDQ_TXr = 0.6;
	//------------------------------
	// Init setting
	//------------------------------
	//param_phyd_zqcal_hw_mode       <= `PI_SD int_regin[24];
	//sw mode
	////original      REGREAD(69 + PHY_BASE_ADDR, rddata);
	ddr_mmio_rd32(4 * (69 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	//original rddata[24] = 1'b0;
	rddata = modified_bits_by_value(rddata, 0, 24, 24); //
	//original      REGWR  (69 + PHY_BASE_ADDR, rddata, 0);
	mmio_wr32(4 * (69 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	//param_phya_reg_zqcal_done  <= `PI_SD int_regin[14];
	////original      REGREAD(85 + PHY_BASE_ADDR, rddata);
	ddr_mmio_rd32(4 * (85 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	//     rddata[14] = 0;
	//     REGWR  (85 + PHY_BASE_ADDR, rddata, 0);
	if (get_bits_from_value(rddata, 14, 14) == 0) { // initial ZQCAL
		//param_pi_dram_class[3:0] <= `PI_SD int_regin[11:8];
		////original      REGREAD(0 + PI_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (0 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
		dram_class = get_bits_from_value(rddata, 11, 8); //DDR3: 4'b0110, DDR4: 4'b1010, LPDDR3: 4'b0111, LPDDR4: 4'b1011
		//    ->ydh_zq_event;
		//a.
		//TOP_REG_TX_ZQ_PD        <= #RD (~pwstrb_mask[31] & TOP_REG_TX_ZQ_PD) |  pwstrb_mask_pwdata[31];
		//original      apb_read(0x40, apb_rddata);
		ddr_mmio_rd32(0x40 + CADENCE_PHYD_APB, apb_rddata);
		//original      apb_rddata[31] = 1'b0;
		apb_rddata = modified_bits_by_value(apb_rddata, 0, 31, 31); //
		////original      apb_write(0x40, apb_rddata);
		mmio_wr32(0x40 + CADENCE_PHYD_APB, apb_rddata);
		//param_phya_reg_sel_zq_high_swing  <= `PI_SD int_regin[1];
		////original      REGREAD(87 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (87 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//original rddata[1] = 0;
		rddata = modified_bits_by_value(rddata, 0, 1, 1); //
		//original      REGWR  (87 + PHY_BASE_ADDR, rddata, 0);
		mmio_wr32(4 * (87 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//param_phya_reg_tx_sel_lpddr4_pmos_ph <= `PI_SD int_regin[2];
		////original      REGREAD(68 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//original rddata[2] = 0;
		rddata = modified_bits_by_value(rddata, 0, 2, 2); //
		//original      REGWR  (68 + PHY_BASE_ADDR, rddata, 0);
		mmio_wr32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//    ->ydh_zq_event;
		//b.
		//param_phya_reg_tx_zq_cmp_en    <= `PI_SD int_regin[3];
		//param_phya_reg_tx_zq_cmp_offset_cal_en <= `PI_SD int_regin[4];
		//param_phya_reg_tx_zq_ph_en     <= `PI_SD int_regin[5];
		//param_phya_reg_tx_zq_pl_en     <= `PI_SD int_regin[6];
		////original      REGREAD(68 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//original rddata[6:3] = 4'b0000;
		rddata = modified_bits_by_value(rddata, 0, 6, 3); //
		//original      REGWR  (68 + PHY_BASE_ADDR, rddata, 0);
		mmio_wr32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//param_phya_reg_tx_zq_step2_en  <= `PI_SD int_regin[16];
		////original      REGREAD(87 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (87 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//original rddata[16] = 0;
		rddata = modified_bits_by_value(rddata, 0, 16, 16); //
		//original      REGWR  (87 + PHY_BASE_ADDR, rddata, 0);
		mmio_wr32(4 * (87 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//param_phya_reg_tx_zq_cmp_offset[4:0] <= `PI_SD int_regin[30:26];
		////original      REGREAD(68 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//original rddata[30:26] = 5'b10000;
		rddata = modified_bits_by_value(rddata, 0x10, 30, 26); //
		//original      REGWR  (68 + PHY_BASE_ADDR, rddata, 0);
		mmio_wr32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//param_phya_reg_tx_zq_sel_vref[4:0] <= `PI_SD int_regin[4:0];
		////original      REGREAD(69 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (69 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//original rddata[4:0] = 5'b10000;
		rddata = modified_bits_by_value(rddata, 0x10, 4, 0); //
		//original      REGWR  (69 + PHY_BASE_ADDR, rddata, 0);
		mmio_wr32(4 * (69 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//param_phya_reg_tx_zq_drvp[5:0] <= `PI_SD int_regin[20:15];
		////original      REGREAD(68 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//original rddata[20:16] = 5'b01001;
		rddata = modified_bits_by_value(rddata, 0x9, 20, 16); //
		//original      REGWR  (68 + PHY_BASE_ADDR, rddata, 0);
		mmio_wr32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//param_phya_reg_tx_zq_drvn[5:0] <= `PI_SD int_regin[13:8];
		////original      REGREAD(68 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//original rddata[12:8] = 5'b01001;
		rddata = modified_bits_by_value(rddata, 0x9, 12, 8); //
		//original      REGWR  (68 + PHY_BASE_ADDR, rddata, 0);
		mmio_wr32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//    ->ydh_zq_event;
		//c.
		//param_phya_reg_vref_pd  <= `PI_SD int_regin[9];
		//     REGREAD(4 + PHY_BASE_ADDR, rddata);
		//     rddata[9] = 0;
		//     REGWR  (4 + PHY_BASE_ADDR, rddata, 0);
		//param_phya_reg_vref_sel[4:0] <= `PI_SD int_regin[14:10];
		//     REGREAD(4 + PHY_BASE_ADDR, rddata);
		//     rddata[14:10] = 5'b01111;
		//     REGWR  (4 + PHY_BASE_ADDR, rddata, 0);
		//    ->ydh_zq_event;
		//d.
		//param_phya_reg_tx_zq_en_test_aux <= `PI_SD int_regin[13];
		////original      REGREAD(85 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (85 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//original rddata[13] = 0;
		rddata = modified_bits_by_value(rddata, 0, 13, 13); //
		//original      REGWR  (85 + PHY_BASE_ADDR, rddata, 0);
		mmio_wr32(4 * (85 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//param_phya_reg_tx_zq_en_test_mux  <= `PI_SD int_regin[15];
		////original      REGREAD(87 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (87 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//original rddata[15] = 0;
		rddata = modified_bits_by_value(rddata, 0, 15, 15); //
		//original      REGWR  (87 + PHY_BASE_ADDR, rddata, 0);
		mmio_wr32(4 * (87 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//TOP_REG_TX_SEL_GPIO         <= #RD (~pwstrb_mask[7] & TOP_REG_TX_SEL_GPIO) |  pwstrb_mask_pwdata[7];
		//original      apb_read(0x1c, rddata);
		ddr_mmio_rd32(0x1c + CADENCE_PHYD_APB, rddata);
		//original rddata[7] = 0;
		rddata = modified_bits_by_value(rddata, 0, 7, 7); //
		////original      apb_write(0x04, rddata);
		mmio_wr32(0x04 + CADENCE_PHYD_APB, rddata);
		// TOP_REG_TX_GPIO_OENZ        <= #RD (~pwstrb_mask[6] & TOP_REG_TX_GPIO_OENZ) |  pwstrb_mask_pwdata[6];
		//original      apb_read(0x1c, rddata);
		ddr_mmio_rd32(0x1c + CADENCE_PHYD_APB, rddata);
		//original rddata[6] = 1;
		rddata = modified_bits_by_value(rddata, 1, 6, 6); //
		////original      apb_write(0x04, rddata);
		mmio_wr32(0x04 + CADENCE_PHYD_APB, rddata);
		//------------------------------
		// CMP offset Cal.
		//------------------------------
		//    ->ydh_zq_event;
		//param_phya_reg_tx_zq_cmp_en    <= `PI_SD int_regin[3];
		//param_phya_reg_tx_zq_cmp_offset_cal_en <= `PI_SD int_regin[4];
		//param_phya_reg_tx_zq_ph_en     <= `PI_SD int_regin[5];
		//param_phya_reg_tx_zq_pl_en     <= `PI_SD int_regin[6];
		////original      REGREAD(68 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//original rddata[6:3] = 4'b0011;
		rddata = modified_bits_by_value(rddata, 0x3, 6, 3); //
		//original      REGWR  (68 + PHY_BASE_ADDR, rddata, 0);
		mmio_wr32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//param_phya_reg_tx_zq_step2_en  <= `PI_SD int_regin[16];
		////original      REGREAD(87 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (87 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//original rddata[16] = 0;
		rddata = modified_bits_by_value(rddata, 0, 16, 16); //
		//original      REGWR  (87 + PHY_BASE_ADDR, rddata, 0);
		mmio_wr32(4 * (87 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//param_phya_reg_tx_zq_cmp_offset[4:0] <= `PI_SD int_regin[30:26];
		////original      REGREAD(68 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//original rddata[30:26] = 5'b10000;
		rddata = modified_bits_by_value(rddata, 0x10, 30, 26); //
		//original      REGWR  (68 + PHY_BASE_ADDR, rddata, 0);
		mmio_wr32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//phy_regout[23]       = param_phya_to_reg_zq_cmp_out;
		for (i = 0; i < 32; i = i + 1) {
			//        repeat(128) @(posedge clk);
			opdelay(128);
			////original         REGREAD(1059 + PHY_BASE_ADDR, rddata);
			ddr_mmio_rd32(4 * (1059 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
			if (get_bits_from_value(rddata, 23, 23)) {
				////original            REGREAD(68 + PHY_BASE_ADDR, rddata);
				ddr_mmio_rd32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
				if (get_bits_from_value(rddata, 30, 26) < 0x1f) {
					//original rddata[30:26] = (rddata[30:26] + 1);
					rddata = modified_bits_by_value(rddata, (get_bits_from_value(rddata, 30, 26) + 1), 30, 26); //
					//original              REGWR  (68 + PHY_BASE_ADDR, rddata, 0);
					mmio_wr32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
				} else {
					uartlog("ZQ Calibration overflow - param_phya_reg_tx_zq_cmp_offset[4:0]: %x\n", get_bits_from_value(rddata, 30, 26));
				}
			} else {
				////original            REGREAD(68 + PHY_BASE_ADDR, rddata);
				ddr_mmio_rd32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
				//if (rddata[30:26] > 5'b00000) {
				if (get_bits_from_value(rddata, 30, 26) > 0) {
					//original rddata[30:26] = (rddata[30:26] - 1);
					rddata = modified_bits_by_value(rddata, (get_bits_from_value(rddata, 30, 26) - 1), 30, 26); //
					//original              REGWR  (68 + PHY_BASE_ADDR, rddata, 0);
					mmio_wr32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
				} else {
					uartlog("ZQ Calibration underflow - param_phya_reg_tx_zq_cmp_offset[4:0]: %x\n", get_bits_from_value(rddata, 30, 26));
				}
			}
		}
		//------------------------------
		// ZQ PL
		//------------------------------
		//    ->ydh_zq_event;
		//param_phya_reg_tx_zq_cmp_en    <= `PI_SD int_regin[3];
		//param_phya_reg_tx_zq_cmp_offset_cal_en <= `PI_SD int_regin[4];
		//param_phya_reg_tx_zq_ph_en     <= `PI_SD int_regin[5];
		//param_phya_reg_tx_zq_pl_en     <= `PI_SD int_regin[6];
		////original      REGREAD(68 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//original rddata[6:3] = 4'b1001;
		rddata = modified_bits_by_value(rddata, 0x9, 6, 3); //
		//original      REGWR  (68 + PHY_BASE_ADDR, rddata, 0);
		mmio_wr32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//param_phya_reg_tx_zq_step2_en  <= `PI_SD int_regin[16];
		////original      REGREAD(87 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (87 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//original rddata[16] = 0;
		rddata = modified_bits_by_value(rddata, 0, 16, 16); //
		//original      REGWR  (87 + PHY_BASE_ADDR, rddata, 0);
		mmio_wr32(4 * (87 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//param_phya_reg_tx_zq_drvn[5:0] <= `PI_SD int_regin[13:8];
		////original      REGREAD(68 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//original rddata[12:8] = 5'b01001;
		rddata = modified_bits_by_value(rddata, 0x9, 12, 8); //
		//original      REGWR  (68 + PHY_BASE_ADDR, rddata, 0);
		mmio_wr32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//phy_regout[23]       = param_phya_to_reg_zq_cmp_out;
		for (i = 0; i < 32; i = i + 1) {
			//        repeat(128) @(posedge clk);
			opdelay(128);
			////original         REGREAD(1059 + PHY_BASE_ADDR, rddata);
			ddr_mmio_rd32(4 * (1059 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
			//if (rddata[23]) {
			if (get_bits_from_value(rddata, 23, 23)) {
				////original            REGREAD(68 + PHY_BASE_ADDR, rddata);
				ddr_mmio_rd32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
				//if (rddata[12:8] < 5'b11111) {
				if (get_bits_from_value(rddata, 12, 8) < 0x1f) {
					//original rddata[12:8] = (rddata[12:8] + 1);
					rddata = modified_bits_by_value(rddata, (get_bits_from_value(rddata, 12, 8) + 1), 12, 8); //
					//original              REGWR  (68 + PHY_BASE_ADDR, rddata, 0);
					mmio_wr32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
				} else {
					uartlog("ZQ Calibration overflow - param_phya_reg_tx_zq_drvn[5:0]: %x\n", get_bits_from_value(rddata, 13, 8));
				}
			} else {
				////original            REGREAD(68 + PHY_BASE_ADDR, rddata);
				ddr_mmio_rd32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
				//if (rddata[12:8] > 5'b00000) {
				if (get_bits_from_value(rddata, 12, 8) > 0) {
					//original rddata[12:8] = (rddata[12:8] - 1);
					rddata = modified_bits_by_value(rddata, (get_bits_from_value(rddata, 12, 8) - 1), 12, 8); //
					//original              REGWR  (68 + PHY_BASE_ADDR, rddata, 0);
					mmio_wr32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
				} else {
					uartlog("ZQ Calibration underflow - param_phya_reg_tx_zq_drvn[5:0]: %x\n", get_bits_from_value(rddata, 13, 8));
				}
			}
		}
		//KN40_GOLDEN = rddata[12:8];
		//KN40_GOLDEN = get_bits_from_value(rddata, 12, 8);
		//------------------------------
		// ZQ PH
		//------------------------------
		//    ->ydh_zq_event;
		//param_phya_reg_tx_zq_cmp_en    <= `PI_SD int_regin[3];
		//param_phya_reg_tx_zq_cmp_offset_cal_en <= `PI_SD int_regin[4];
		//param_phya_reg_tx_zq_ph_en     <= `PI_SD int_regin[5];
		//param_phya_reg_tx_zq_pl_en     <= `PI_SD int_regin[6];
		////original      REGREAD(68 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//original rddata[6:3] = 4'b1001;
		rddata = modified_bits_by_value(rddata, 0x9, 6, 3); //
		//original      REGWR  (68 + PHY_BASE_ADDR, rddata, 0);
		mmio_wr32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//param_phya_reg_tx_zq_step2_en  <= `PI_SD int_regin[16];
		////original      REGREAD(87 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (87 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//original rddata[16] = 1;
		rddata = modified_bits_by_value(rddata, 1, 16, 16); //
		//original      REGWR  (87 + PHY_BASE_ADDR, rddata, 0);
		mmio_wr32(4 * (87 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//param_phya_reg_tx_zq_drvp[5:0] <= `PI_SD int_regin[20:15];
		////original      REGREAD(68 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//original rddata[20:16] = 5'b01001;
		rddata = modified_bits_by_value(rddata, 0x9, 20, 16); //
		//original      REGWR  (68 + PHY_BASE_ADDR, rddata, 0);
		mmio_wr32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//phy_regout[23]       = param_phya_to_reg_zq_cmp_out;
		for (i = 0; i < 32; i = i + 1) {
			//        repeat(128) @(posedge clk);
			opdelay(128);
			////original         REGREAD(1059 + PHY_BASE_ADDR, rddata);
			ddr_mmio_rd32(4 * (1059 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
			//if (rddata[23]) {
			if (get_bits_from_value(rddata, 23, 23)) {
				////original            REGREAD(68 + PHY_BASE_ADDR, rddata);
				ddr_mmio_rd32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
				//if (rddata[20:16] > 5'b00000) {
				if (get_bits_from_value(rddata, 20, 16) > 0) {
					//original rddata[20:16] = (rddata[20:16] - 1);
					rddata = modified_bits_by_value(rddata, (get_bits_from_value(rddata, 20, 16) - 1), 20, 16); //
					//original              REGWR  (68 + PHY_BASE_ADDR, rddata, 0);
					mmio_wr32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
				} else {
					uartlog("ZQ Calibration underflow - param_phya_reg_tx_zq_drvp[5:0]: %x\n", get_bits_from_value(rddata, 20, 15));
				}
			} else {
				////original            REGREAD(68 + PHY_BASE_ADDR, rddata);
				ddr_mmio_rd32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
				//if (rddata[20:16] < 5'b11111) {
				if (get_bits_from_value(rddata, 20, 16) < 0x1f) {
					//original rddata[20:16] = (rddata[20:16] + 1);
					rddata = modified_bits_by_value(rddata, (get_bits_from_value(rddata, 20, 16) + 1), 20, 16); //
					//original              REGWR  (68 + PHY_BASE_ADDR, rddata, 0);
					mmio_wr32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
				} else {
				}
			}
		}
		//KP40_GOLDEN = rddata[20:16];
		// KP40_GOLDEN = get_bits_from_value(rddata, 20, 16);
		//------------------------------
		// ZQ Complete
		//------------------------------
		//param_phya_reg_zqcal_done  <= `PI_SD int_regin[14];
		////original      REGREAD(85 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (85 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//original rddata[14] = 1;
		rddata = modified_bits_by_value(rddata, 1, 14, 14); //
		//original      REGWR  (85 + PHY_BASE_ADDR, rddata, 0);
		mmio_wr32(4 * (85 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//param_phya_reg_tx_zq_cmp_en    <= `PI_SD int_regin[3];
		//param_phya_reg_tx_zq_cmp_offset_cal_en <= `PI_SD int_regin[4];
		//param_phya_reg_tx_zq_ph_en     <= `PI_SD int_regin[5];
		//param_phya_reg_tx_zq_pl_en     <= `PI_SD int_regin[6];
		////original      REGREAD(68 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//original rddata[6:3] = 4'b0000;
		rddata = modified_bits_by_value(rddata, 0, 6, 3); //
		//original      REGWR  (68 + PHY_BASE_ADDR, rddata, 0);
		mmio_wr32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//param_phya_reg_tx_zq_step2_en  <= `PI_SD int_regin[16];
		////original      REGREAD(87 + PHY_BASE_ADDR, rddata);
		ddr_mmio_rd32(4 * (87 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//original rddata[16] = 0;
		rddata = modified_bits_by_value(rddata, 0, 16, 16); //
		//original      REGWR  (87 + PHY_BASE_ADDR, rddata, 0);
		mmio_wr32(4 * (87 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//TOP_REG_TX_ZQ_PD        <= #RD (~pwstrb_mask[31] & TOP_REG_TX_ZQ_PD) |  pwstrb_mask_pwdata[31];
		//original      apb_read(0x40, apb_rddata);
		ddr_mmio_rd32(0x40 + CADENCE_PHYD_APB, apb_rddata);
		//original      apb_rddata[31] = 1'b1;
		apb_rddata = modified_bits_by_value(apb_rddata, 1, 31, 31); //
		////original      apb_write(0x40, apb_rddata);
		mmio_wr32(0x40 + CADENCE_PHYD_APB, apb_rddata);
		//     repeat(200) @(posedge clk);
		opdelay(200);
		// //if ((KP40_GOLDEN[4:0] >= 5'b01111) & (KP40_GOLDEN[4:0] <= 5'b10001))
		// if ((get_bits_from_value(KP40_GOLDEN, 4, 0) >= 0xf) & (get_bits_from_value(KP40_GOLDEN, 4, 0) <= 0x11)) {
		// } else {
		// }
		// //if ((KN40_GOLDEN[4:0] >= 5'b01111) & (KN40_GOLDEN[4:0] <= 5'b10001))
		// if ((get_bits_from_value(KP40_GOLDEN, 4, 0) >= 0xf) & (get_bits_from_value(KP40_GOLDEN, 4, 0) <= 0x11)) {
		// } else {
		// }
		opdelay(200);
		//      opdelay(200000);
	} //} initial ZQCAL
	else
		uartlog("Not need ZQCAL\n");
}

void clk_normal(void)
{
	//original apb_read(0x0c, ken_rddata);
	ddr_mmio_rd32(0x0c + CADENCE_PHYD_APB, apb_rddata);
	//ken_rddata[13] = 0  ;  // TOP_REG_DDRPLL_SEL_LOW_SPEED 0
	//ken_rddata[14] = 0  ;  // TOP_REG_DDRPLL_MAS_DIV_OUT_SEL 0
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 13, 13); //TOP_REG_SSC_SET
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 14, 14); //TOP_REG_SSC_SET
	////original apb_write(0x0c, ken_rddata);
	mmio_wr32(0x0c + CADENCE_PHYD_APB, apb_rddata);
#ifdef SSC_EN
	//==============================================================
	//Enable SSC
	//==============================================================
	//original apb_read(0x34, apb_rddata);
	ddr_mmio_rd32(0x34 + CADENCE_PHYD_APB, apb_rddata);
	//original apb_rddata[31:0] = reg_set[31:0];         //TOP_REG_SSC_SET
	apb_rddata = modified_bits_by_value(apb_rddata, reg_set, 31, 0); //TOP_REG_SSC_SET
	////original apb_write(0x34, apb_rddata);
	mmio_wr32(0x34 + CADENCE_PHYD_APB, apb_rddata);
	//original apb_read(0x38, apb_rddata);
	ddr_mmio_rd32(0x38 + CADENCE_PHYD_APB, apb_rddata);
	//original apb_rddata[31:0] = reg_span[15:0];        //TOP_REG_SSC_SPAN
	apb_rddata = modified_bits_by_value(apb_rddata, get_bits_from_value(reg_span, 15, 0), 31, 0); //TOP_REG_SSC_SPAN
	////original apb_write(0x38, apb_rddata);
	mmio_wr32(0x38 + CADENCE_PHYD_APB, apb_rddata);
	//original apb_read(0x3C, apb_rddata);
	ddr_mmio_rd32(0x3C + CADENCE_PHYD_APB, apb_rddata);
	//original apb_rddata[31:0] = reg_step[23:0];        //TOP_REG_SSC_STEP
	apb_rddata = modified_bits_by_value(apb_rddata, get_bits_from_value(reg_span, 23, 0), 31, 0); //TOP_REG_SSC_STEP
	////original apb_write(0x3C, apb_rddata);
	mmio_wr32(0x3C + CADENCE_PHYD_APB, apb_rddata);
	//original apb_read(0x30, apb_rddata);
	ddr_mmio_rd32(0x30 + CADENCE_PHYD_APB, apb_rddata);
	//original apb_rddata[0]     = ~apb_rddata[0];       //TOP_REG_SSC_SW_UP
	apb_rddata = modified_bits_by_value(apb_rddata, ~get_bits_from_value(apb_rddata, 0, 0), 0, 0); //
	//original apb_rddata[1]     = 1;                    //TOP_REG_SSC_EN_SSC
	apb_rddata = modified_bits_by_value(apb_rddata, 1, 1, 1); //
	//original apb_rddata[3:2]   = 2'b00;                //TOP_REG_SSC_SSC_MODE
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 3, 2); //TOP_REG_SSC_SSC_MODE
	//original apb_rddata[4]     = 1;                    //TOP_REG_SSC_SEL
	apb_rddata = modified_bits_by_value(apb_rddata, 1, 4, 4); //
	//original apb_rddata[8]     = 0;                    //TOP_REG_CLKDIV_DIV6EN
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 8, 8); //
	//original apb_rddata[9]     = 0;                    //TOP_REG_CLKDIV_DIV7EN
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 9, 9); //
	//original apb_rddata[10]    = 0;                    //TOP_REG_CLKDIV_DIV8EN
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 10, 10); //
	//original apb_rddata[11]    = 0;                    //TOP_REG_CLKDIV_DIV12EN
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 11, 11); //
	//original apb_rddata[13:12] = 2'b00;                //TOP_REG_CLKDIV_CLKSEL
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 13, 12); //TOP_REG_CLKDIV_CLKSEL
	////original apb_write(0x30, apb_rddata);
	mmio_wr32(0x30 + CADENCE_PHYD_APB, apb_rddata);
	uartlog("SSC_EN\n");
#else
#ifdef SSC_BYPASS
	//==============================================================
	//Bypass SSC
	//==============================================================
	//1.5G / 6 = 250M, clk = 2G
	//1.5G / 7 = 214.3M clk = 1.7G
	//1.5G / 8 = 187.5M clk = 1.5G
	//1.5G /12 = 125M clk = 1G
	//original apb_read(0x30, apb_rddata);
	ddr_mmio_rd32(0x30 + CADENCE_PHYD_APB, apb_rddata);
	//apb_rddata[0]     = 0;                    //TOP_REG_SSC_SW_UP
	//original apb_rddata[1]     = 0;                    //TOP_REG_SSC_EN_SSC
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 1, 1); //
	//original apb_rddata[3:2]   = 2'b00;                //TOP_REG_SSC_SSC_MODE
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 3, 2); //TOP_REG_SSC_SSC_MODE
	//original apb_rddata[4]     = 0;                    //TOP_REG_SSC_SEL
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 4, 4); //
	//original apb_rddata[8]     = mem_freq_2133;       //TOP_REG_CLKDIV_DIV6EN
	apb_rddata = modified_bits_by_value(apb_rddata, mem_freq_2133, 8, 8); //
	//original apb_rddata[9]     = 0;                    //TOP_REG_CLKDIV_DIV7EN
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 9, 9); //
	//original apb_rddata[10]    = mem_freq_1600;       //TOP_REG_CLKDIV_DIV8EN
	apb_rddata = modified_bits_by_value(apb_rddata, mem_freq_1600, 10, 10); //
	//original apb_rddata[11]    = mem_freq_1066;       //TOP_REG_CLKDIV_DIV12EN
	apb_rddata = modified_bits_by_value(apb_rddata, mem_freq_1066, 11, 11); //
	//original apb_rddata[13:12] = clkdiv_clksel;        //TOP_REG_CLKDIV_CLKSEL
	apb_rddata = modified_bits_by_value(apb_rddata, clkdiv_clksel, 13, 12); //TOP_REG_CLKDIV_CLKSEL
	////original apb_write(0x30, apb_rddata);
	mmio_wr32(0x30 + CADENCE_PHYD_APB, apb_rddata);
	uartlog("SSC_BYPASS\n");
#else
	//==============================================================
	//SSC_EN =0
	//==============================================================
	uartlog("SSC_EN =0\n");
	//original apb_read(0x34, apb_rddata);
	ddr_mmio_rd32(0x34 + CADENCE_PHYD_APB, apb_rddata);
	//original apb_rddata[31:0] = reg_set[31:0];         //TOP_REG_SSC_SET
	apb_rddata = modified_bits_by_value(apb_rddata, reg_set, 31, 0); //TOP_REG_SSC_SET
	////original apb_write(0x34, apb_rddata);
	mmio_wr32(0x34 + CADENCE_PHYD_APB, apb_rddata);
	//original apb_read(0x38, apb_rddata);
	ddr_mmio_rd32(0x38 + CADENCE_PHYD_APB, apb_rddata);
	//original apb_rddata[31:0] = 0;                 //TOP_REG_SSC_SPAN
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 31, 0); //TOP_REG_SSC_SPAN
	////original apb_write(0x38, apb_rddata);
	mmio_wr32(0x38 + CADENCE_PHYD_APB, apb_rddata);
	//original apb_read(0x3C, apb_rddata);
	ddr_mmio_rd32(0x3C + CADENCE_PHYD_APB, apb_rddata);
	//original apb_rddata[31:0] = reg_step[23:0];        //TOP_REG_SSC_STEP
	apb_rddata = modified_bits_by_value(apb_rddata, get_bits_from_value(reg_step, 23, 0), 31, 0); //TOP_REG_SSC_STEP
	////original apb_write(0x3C, apb_rddata);
	mmio_wr32(0x3C + CADENCE_PHYD_APB, apb_rddata);
	//original apb_read(0x30, apb_rddata);
	ddr_mmio_rd32(0x30 + CADENCE_PHYD_APB, apb_rddata);
	//original apb_rddata[0]     = ~apb_rddata[0];       //TOP_REG_SSC_SW_UP
	apb_rddata = modified_bits_by_value(apb_rddata, ~get_bits_from_value(apb_rddata, 0, 0), 0, 0); //
	//original apb_rddata[1]     = 0;                    //TOP_REG_SSC_EN_SSC
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 1, 1); //
	//original apb_rddata[3:2]   = 2'b00;                //TOP_REG_SSC_SSC_MODE
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 3, 2); //TOP_REG_SSC_SSC_MODE
	//original apb_rddata[4]     = 1;                    //TOP_REG_SSC_SEL
	apb_rddata = modified_bits_by_value(apb_rddata, 1, 4, 4); //
	////original apb_write(0x30, apb_rddata);
	mmio_wr32(0x30 + CADENCE_PHYD_APB, apb_rddata);
	uartlog("SSC_OFF\n");
#endif //SSC_BYPASS
#endif //SSC_EN
	uartlog("back to original frequency\n");
}

void clk_div2(void)
{
	uartlog("clk_div2\n");
	//original apb_read(0x0c, ken_rddata);
	ddr_mmio_rd32(0x0c + CADENCE_PHYD_APB, ken_rddata);
	//ken_rddata[14] = 1  ;  // TOP_REG_DDRPLL_MAS_DIV_OUT_SEL 1
	ken_rddata = modified_bits_by_value(ken_rddata, 1, 14, 14); //TOP_REG_SSC_SET
	////original apb_write(0x0c, ken_rddata);
	mmio_wr32(0x0c + CADENCE_PHYD_APB, ken_rddata);
	uartlog("div2 original frequency\n");
}

void INT_ISR_02(void)
{
	uartlog("INT_ISR_02\n");
	//param_phyd_to_reg_calvl_set_vref_req
	//axi_rddata[1] = 1'b0;
	axi_rddata = modified_bits_by_value(axi_rddata, 0, 1, 1); //
	while (get_bits_from_value(axi_rddata, 1, 1) == 0) {
		////original         REGREAD(1087 + PHY_BASE_ADDR, axi_rddata);
		ddr_mmio_rd32(4 * (1087 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
	}
	/*
	//if this time is last VREF, set param_phyd_calvl_resp_sw = 2'b11
	//param_phyd_to_reg_calvl_done
	//phy_regout[4]       = param_phyd_to_reg_calvl_done;
	////original             REGREAD(1087 + PHY_BASE_ADDR, axi_rddata);
	ddr_mmio_rd32(4 * (1087 + PHY_BASE_ADDR)+CADENCE_PHYD, axi_rddata);
	if (axi_rddata[4]) {
		//param_phyd_calvl_resp_sw[1:0]  <= int_regin[5:4];
		////original               REGREAD(168 + PHY_BASE_ADDR, axi_rddata);
		ddr_mmio_rd32(4 * (168 + PHY_BASE_ADDR)+CADENCE_PHYD, axi_rddata);
		//original axi_rddata[5:4] = 2'b11;
		axi_rddata = modified_bits_by_value(axi_rddata, 2'b11, 5, 4);//
		//original               REGWR  (168 + PHY_BASE_ADDR, axi_rddata, 0);
		mmio_wr32(4 * (168 + PHY_BASE_ADDR)+CADENCE_PHYD, axi_rddata);
		} else {
			//param_phyd_calvl_resp_sw[1:0]  <= int_regin[5:4];
			////original               REGREAD(168 + PHY_BASE_ADDR, axi_rddata);
			ddr_mmio_rd32(4 * (168 + PHY_BASE_ADDR)+CADENCE_PHYD, axi_rddata);
			//original axi_rddata[5:4] = 2'b01;
			axi_rddata = modified_bits_by_value(axi_rddata, 2'b01, 5, 4);//
			//original               REGWR  (168 + PHY_BASE_ADDR, axi_rddata, 0);
			mmio_wr32(4 * (168 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
		}
		*/
	////original             REGREAD(168 + PHY_BASE_ADDR, axi_rddata);
	ddr_mmio_rd32(4 * (168 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
	//axi_rddata[3] = 1;
	axi_rddata = modified_bits_by_value(axi_rddata, 1, 3, 3); //
	//original             REGWR  (168 + PHY_BASE_ADDR, axi_rddata, 0);
	mmio_wr32(4 * (168 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
	////original             REGREAD(168 + PHY_BASE_ADDR, axi_rddata);
	ddr_mmio_rd32(4 * (168 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
	//axi_rddata[3] = 0;
	axi_rddata = modified_bits_by_value(axi_rddata, 0, 3, 3); //
	//original             REGWR  (168 + PHY_BASE_ADDR, axi_rddata, 0);
	mmio_wr32(4 * (168 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
}

void INT_ISR_08(void)
{
	uartlog("INT_ISR_08\n");
	//param_phyd_clkctrl_init_complete   <= int_regin[24];
	////original       REGREAD(133 + PHY_BASE_ADDR, axi_rddata);
	ddr_mmio_rd32(4 * (133 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
	axi_rddata = modified_bits_by_value(axi_rddata, 0, 24, 24); //
	//original       REGWR  (133 + PHY_BASE_ADDR, axi_rddata, 0);
	mmio_wr32(4 * (133 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
	//----------------------------------------------------
	//Set LPDDR4 delay value when CA training is done
	////original       REGREAD(240 + PHY_BASE_ADDR, axi_rddata);
	ddr_mmio_rd32(4 * (240 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
	lpddr4_calvl_done = get_bits_from_value(axi_rddata, 0, 0);
	//original       apb_read(0x4c, apb_rddata);
	ddr_mmio_rd32(0x4c + CADENCE_PHYD_APB, apb_rddata);
	EN_PLL_SPEED_CHG = get_bits_from_value(apb_rddata, 0, 0);
	CUR_PLL_SPEED = get_bits_from_value(apb_rddata, 5, 4);
	NEXT_PLL_SPEED = get_bits_from_value(apb_rddata, 9, 8);
	uartlog("lpddr4_calvl_done=%x, CUR_PLL_SPEED=%x\n", lpddr4_calvl_done, CUR_PLL_SPEED);
	//if ((|lpddr4_calvl_done) && (CUR_PLL_SPEED==0)) { //
	if ((lpddr4_calvl_done != 0) && (CUR_PLL_SPEED == 0)) { //
		//      if (`lpddr4_calvl_done & ~`CUR_PLL_SPEED) {
		//----------------------------------------------------
		//param_phyd_calvl_resp_auto     <= `PI_SD int_regin[31];
		//resp auto ?
		////original       REGREAD(165 + PHY_BASE_ADDR, axi_rddata);
		ddr_mmio_rd32(4 * (165 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
		if (get_bits_from_value(axi_rddata, 31, 31) == 0) {
			//min_pass_code
			////original                                      REGREAD(1074 + PHY_BASE_ADDR, axi_rddata1);
			ddr_mmio_rd32(4 * (1074 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata1);
			////original                                      REGREAD(1075 + PHY_BASE_ADDR, axi_rddata2);
			ddr_mmio_rd32(4 * (1075 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata2);
			////original                                      REGREAD(1076 + PHY_BASE_ADDR, axi_rddata3);
			ddr_mmio_rd32(4 * (1076 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata3);
			//max_pass_code
			////original                                      REGREAD(1084 + PHY_BASE_ADDR, axi_rddata6);
			ddr_mmio_rd32(4 * (1084 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata6);
			////original                                      REGREAD(1085 + PHY_BASE_ADDR, axi_rddata7);
			ddr_mmio_rd32(4 * (1085 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata7);
			////original                                      REGREAD(1086 + PHY_BASE_ADDR, axi_rddata8);
			ddr_mmio_rd32(4 * (1086 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata8);
			lpddr4_bit0_min_pass_save = get_bits_from_value(axi_rddata1, 13, 0);
			lpddr4_bit1_min_pass_save = get_bits_from_value(axi_rddata1, 29, 16);
			lpddr4_bit2_min_pass_save = get_bits_from_value(axi_rddata2, 13, 0);
			lpddr4_bit3_min_pass_save = get_bits_from_value(axi_rddata2, 29, 16);
			lpddr4_bit4_min_pass_save = get_bits_from_value(axi_rddata3, 13, 0);
			lpddr4_bit5_min_pass_save = get_bits_from_value(axi_rddata3, 29, 16);
			lpddr4_bit0_max_pass_save = get_bits_from_value(axi_rddata6, 13, 0);
			lpddr4_bit1_max_pass_save = get_bits_from_value(axi_rddata6, 29, 16);
			lpddr4_bit2_max_pass_save = get_bits_from_value(axi_rddata7, 13, 0);
			lpddr4_bit3_max_pass_save = get_bits_from_value(axi_rddata7, 29, 16);
			lpddr4_bit4_max_pass_save = get_bits_from_value(axi_rddata8, 13, 0);
			lpddr4_bit5_max_pass_save = get_bits_from_value(axi_rddata8, 29, 16);
			// min+max/2
			lpddr4_bit0_pass_save = (lpddr4_bit0_min_pass_save + lpddr4_bit0_max_pass_save) >> 1;
			lpddr4_bit1_pass_save = (lpddr4_bit1_min_pass_save + lpddr4_bit1_max_pass_save) >> 1;
			lpddr4_bit2_pass_save = (lpddr4_bit2_min_pass_save + lpddr4_bit2_max_pass_save) >> 1;
			lpddr4_bit3_pass_save = (lpddr4_bit3_min_pass_save + lpddr4_bit3_max_pass_save) >> 1;
			lpddr4_bit4_pass_save = (lpddr4_bit4_min_pass_save + lpddr4_bit4_max_pass_save) >> 1;
			lpddr4_bit5_pass_save = (lpddr4_bit5_min_pass_save + lpddr4_bit5_max_pass_save) >> 1;
			uartlog("[ISR_08] lpddr4_bit0_min_pass_save=%x\n", lpddr4_bit0_min_pass_save);
			uartlog("[ISR_08] lpddr4_bit1_min_pass_save=%x\n", lpddr4_bit1_min_pass_save);
			uartlog("[ISR_08] lpddr4_bit2_min_pass_save=%x\n", lpddr4_bit2_min_pass_save);
			uartlog("[ISR_08] lpddr4_bit3_min_pass_save=%x\n", lpddr4_bit3_min_pass_save);
			uartlog("[ISR_08] lpddr4_bit4_min_pass_save=%x\n", lpddr4_bit4_min_pass_save);
			uartlog("[ISR_08] lpddr4_bit5_min_pass_save=%x\n", lpddr4_bit5_min_pass_save);
			uartlog("[ISR_08] lpddr4_bit0_max_pass_save=%x\n", lpddr4_bit0_max_pass_save);
			uartlog("[ISR_08] lpddr4_bit1_max_pass_save=%x\n", lpddr4_bit1_max_pass_save);
			uartlog("[ISR_08] lpddr4_bit2_max_pass_save=%x\n", lpddr4_bit2_max_pass_save);
			uartlog("[ISR_08] lpddr4_bit3_max_pass_save=%x\n", lpddr4_bit3_max_pass_save);
			uartlog("[ISR_08] lpddr4_bit4_max_pass_save=%x\n", lpddr4_bit4_max_pass_save);
			uartlog("[ISR_08] lpddr4_bit5_max_pass_save=%x\n", lpddr4_bit5_max_pass_save);
			uartlog("[ISR_08] lpddr4_bit0_pass_save =%x\n", lpddr4_bit0_pass_save);
			uartlog("[ISR_08] lpddr4_bit1_pass_save =%x\n", lpddr4_bit1_pass_save);
			uartlog("[ISR_08] lpddr4_bit2_pass_save =%x\n", lpddr4_bit2_pass_save);
			uartlog("[ISR_08] lpddr4_bit3_pass_save =%x\n", lpddr4_bit3_pass_save);
			uartlog("[ISR_08] lpddr4_bit4_pass_save =%x\n", lpddr4_bit4_pass_save);
			uartlog("[ISR_08] lpddr4_bit5_pass_save =%x\n", lpddr4_bit5_pass_save);
			// param_phyd_tx_ca3_shift_sel[5:0] <= int_regin[29:24];
			// param_phyd_tx_ca2_shift_sel[5:0] <= int_regin[21:16];
			// param_phyd_tx_ca1_shift_sel[5:0] <= int_regin[13:8];
			// param_phyd_tx_ca0_shift_sel[5:0] <= int_regin[5:0];
			//REGREAD(134 + PHY_BASE_ADDR, axi_rddata);
			axi_rddata = 0;
			//axi_rddata[29:24] = 4;
			//axi_rddata[21:16] = 4;
			//axi_rddata[13:8]  = 4;
			//axi_rddata[5:0]   = 4;
			//axi_rddata[29:24] = lpddr4_bit3_pass_save[12:7];
			axi_rddata = modified_bits_by_value(axi_rddata, get_bits_from_value(lpddr4_bit3_pass_save, 12, 7), 12, 7);
			//axi_rddata[21:16] = lpddr4_bit2_pass_save[12:7];
			axi_rddata = modified_bits_by_value(axi_rddata, get_bits_from_value(lpddr4_bit2_pass_save, 12, 7), 21, 16);
			//axi_rddata[13:8]  = lpddr4_bit1_pass_save[12:7];
			axi_rddata = modified_bits_by_value(axi_rddata, get_bits_from_value(lpddr4_bit1_pass_save, 12, 7), 13, 8);
			//axi_rddata[5:0]   = lpddr4_bit0_pass_save[12:7];
			axi_rddata = modified_bits_by_value(axi_rddata, get_bits_from_value(lpddr4_bit0_pass_save, 12, 7), 5, 0);
			//original                                     REGWR  (134 + PHY_BASE_ADDR, axi_rddata, 0);
			mmio_wr32(4 * (134 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
			// param_phyd_tx_ca7_shift_sel[5:0] <= int_regin[29:24];
			// param_phyd_tx_ca6_shift_sel[5:0] <= int_regin[21:16];
			// param_phyd_tx_ca5_shift_sel[5:0] <= int_regin[13:8];
			// param_phyd_tx_ca4_shift_sel[5:0] <= int_regin[5:0];
			//REGREAD(135 + PHY_BASE_ADDR, axi_rddata);
			//axi_rddata[31:0]  = 0x0;
			//axi_rddata[29:24] = 4;
			//axi_rddata[21:16] = 4;
			//axi_rddata[13:8]  = 4;
			//axi_rddata[5:0]   = 4;
			//axi_rddata[29:24] = lpddr4_bit1_pass_save[12:7];
			axi_rddata = modified_bits_by_value(axi_rddata, get_bits_from_value(lpddr4_bit1_pass_save, 12, 7), 29, 24);
			//axi_rddata[21:16] = lpddr4_bit0_pass_save[12:7];
			axi_rddata = modified_bits_by_value(axi_rddata, get_bits_from_value(lpddr4_bit0_pass_save, 12, 7), 21, 16);
			//axi_rddata[13:8]  = lpddr4_bit5_pass_save[12:7];
			axi_rddata = modified_bits_by_value(axi_rddata, get_bits_from_value(lpddr4_bit5_pass_save, 12, 7), 13, 8);
			//axi_rddata[5:0]   = lpddr4_bit4_pass_save[12:7];
			axi_rddata = modified_bits_by_value(axi_rddata, get_bits_from_value(lpddr4_bit4_pass_save, 12, 7), 5, 0);
			//original                                     REGWR  (135 + PHY_BASE_ADDR, axi_rddata, 0);
			mmio_wr32(4 * (135 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
			// param_phyd_tx_ca11_shift_sel[5:0] <= int_regin[29:24];
			// param_phyd_tx_ca10_shift_sel[5:0] <= int_regin[21:16];
			// param_phyd_tx_ca9_shift_sel[5:0] <= int_regin[13:8];
			// param_phyd_tx_ca8_shift_sel[5:0] <= int_regin[5:0];
			axi_rddata = 0;
			//axi_rddata[13:8]  = 4;
			//axi_rddata[5:0]   = 4;
			//axi_rddata[29:24] = lpddr4_bit5_pass_save[12:7];
			axi_rddata = modified_bits_by_value(axi_rddata, get_bits_from_value(lpddr4_bit5_pass_save, 12, 7), 29, 24);
			//axi_rddata[21:16] = lpddr4_bit4_pass_save[12:7];
			axi_rddata = modified_bits_by_value(axi_rddata, get_bits_from_value(lpddr4_bit4_pass_save, 12, 7), 21, 16);
			//axi_rddata[13:8]  = lpddr4_bit3_pass_save[12:7];
			axi_rddata = modified_bits_by_value(axi_rddata, get_bits_from_value(lpddr4_bit3_pass_save, 12, 7), 13, 8);
			//axi_rddata[5:0]   = lpddr4_bit2_pass_save[12:7];
			axi_rddata = modified_bits_by_value(axi_rddata, get_bits_from_value(lpddr4_bit2_pass_save, 12, 7), 5, 0);
			//original                                     REGWR  (136 + PHY_BASE_ADDR, axi_rddata, 0);
			mmio_wr32(4 * (136 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
			// param_phyd_reg_tx_ca_tx_dline_code_ca3_sw[6:0] <= int_regin[30:24];
			// param_phyd_reg_tx_ca_tx_dline_code_ca2_sw[6:0] <= int_regin[22:16];
			// param_phyd_reg_tx_ca_tx_dline_code_ca1_sw[6:0] <= int_regin[14:8];
			// param_phyd_reg_tx_ca_tx_dline_code_ca0_sw[6:0] <= int_regin[6:0];
			//REGREAD(145 + PHY_BASE_ADDR, axi_rddata);
			axi_rddata = 0;
			//axi_rddata[30:24] = lpddr4_bit3_pass_save[6:0];
			axi_rddata = modified_bits_by_value(axi_rddata, get_bits_from_value(lpddr4_bit3_pass_save, 6, 0), 30, 24);
			//axi_rddata[22:16] = lpddr4_bit2_pass_save[6:0];
			axi_rddata = modified_bits_by_value(axi_rddata, get_bits_from_value(lpddr4_bit2_pass_save, 6, 0), 22, 16);
			//axi_rddata[14:8]  = lpddr4_bit1_pass_save[6:0];
			axi_rddata = modified_bits_by_value(axi_rddata, get_bits_from_value(lpddr4_bit1_pass_save, 6, 0), 14, 8);
			//axi_rddata[6:0]   = lpddr4_bit0_pass_save[6:0];
			axi_rddata = modified_bits_by_value(axi_rddata, get_bits_from_value(lpddr4_bit0_pass_save, 6, 0), 6, 0);
			//original                                     REGWR  (145 + PHY_BASE_ADDR, axi_rddata, 0);
			mmio_wr32(4 * (145 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
			// param_phyd_reg_tx_ca_tx_dline_code_ca7_sw[6:0] <= int_regin[30:24];
			// param_phyd_reg_tx_ca_tx_dline_code_ca6_sw[6:0] <= int_regin[22:16];
			// param_phyd_reg_tx_ca_tx_dline_code_ca5_sw[6:0] <= int_regin[14:8];
			// param_phyd_reg_tx_ca_tx_dline_code_ca4_sw[6:0] <= int_regin[6:0];
			//REGREAD(146 + PHY_BASE_ADDR, axi_rddata);
			axi_rddata = 0;
			//axi_rddata[30:24] = lpddr4_bit1_pass_save[6:0];
			axi_rddata = modified_bits_by_value(axi_rddata, get_bits_from_value(lpddr4_bit1_pass_save, 6, 0), 30, 24);
			//axi_rddata[22:16] = lpddr4_bit0_pass_save[6:0];
			axi_rddata = modified_bits_by_value(axi_rddata, get_bits_from_value(lpddr4_bit0_pass_save, 6, 0), 22, 16);
			//axi_rddata[14:8]  = lpddr4_bit5_pass_save[6:0];
			axi_rddata = modified_bits_by_value(axi_rddata, get_bits_from_value(lpddr4_bit5_pass_save, 6, 0), 14, 8);
			//axi_rddata[6:0]   = lpddr4_bit4_pass_save[6:0];
			axi_rddata = modified_bits_by_value(axi_rddata, get_bits_from_value(lpddr4_bit4_pass_save, 6, 0), 6, 0);
			//original                                     REGWR  (146 + PHY_BASE_ADDR, axi_rddata, 0);
			mmio_wr32(4 * (146 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
			// param_phyd_reg_tx_ca_tx_dline_code_ca9_sw[6:0] <= int_regin[14:8];
			// param_phyd_reg_tx_ca_tx_dline_code_ca8_sw[6:0] <= int_regin[6:0];
			//REGREAD(147 + PHY_BASE_ADDR, axi_rddata);
			axi_rddata = 0;
			//axi_rddata[30:24] = lpddr4_bit5_pass_save[6:0];
			axi_rddata = modified_bits_by_value(axi_rddata, get_bits_from_value(lpddr4_bit5_pass_save, 6, 0), 30, 24);
			//axi_rddata[22:16] = lpddr4_bit4_pass_save[6:0];
			axi_rddata = modified_bits_by_value(axi_rddata, get_bits_from_value(lpddr4_bit4_pass_save, 6, 0), 22, 16);
			//axi_rddata[14:8]  = lpddr4_bit3_pass_save[6:0];
			axi_rddata = modified_bits_by_value(axi_rddata, get_bits_from_value(lpddr4_bit3_pass_save, 6, 0), 14, 8);
			//axi_rddata[6:0]   = lpddr4_bit2_pass_save[6:0];
			axi_rddata = modified_bits_by_value(axi_rddata, get_bits_from_value(lpddr4_bit2_pass_save, 6, 0), 6, 0);
			//original                                     REGWR  (147 + PHY_BASE_ADDR, axi_rddata, 0);
			mmio_wr32(4 * (147 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
		} //resp auto =0
		//------------------------------
		//  CALVL- 1: cha/chb different cs @calvl
		//------------------------------
		//param_phyd_calvl_cs_diff       <= `PI_SD int_regin[24];
		////original                                     REGREAD(166 + PHY_BASE_ADDR, axi_rddata);
		ddr_mmio_rd32(4 * (166 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
		//axi_rddata[24] = 0;
		axi_rddata = modified_bits_by_value(axi_rddata, 0, 24, 24);
		//original                                     REGWR  (166 + PHY_BASE_ADDR, axi_rddata, 0);
		mmio_wr32(4 * (166 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
		//Do SW update
		//param_phyd_calvl_sw_upd        <= int_regin[6];
		////original                                     REGREAD(168 + PHY_BASE_ADDR, axi_rddata);
		ddr_mmio_rd32(4 * (168 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
		//axi_rddata[6] = 1;
		axi_rddata = modified_bits_by_value(axi_rddata, 1, 6, 6);
		//original                                     REGWR  (168 + PHY_BASE_ADDR, axi_rddata, 0);
		mmio_wr32(4 * (168 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
		//Wait for SW update ACK
		//axi_rddata[0] = 0;
		axi_rddata = modified_bits_by_value(axi_rddata, 0, 0, 0);
		while (get_bits_from_value(axi_rddata, 0, 0) == 0) {
			////original                                       REGREAD(1087 + PHY_BASE_ADDR, axi_rddata);
			ddr_mmio_rd32(4 * (1087 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
			//repeat(10) @(posedge`clk_soc);
			//opdelay(80) ;
		}
		//Clear SW update
		//param_phyd_calvl_sw_upd        <= int_regin[6];
		////original                                     REGREAD(168 + PHY_BASE_ADDR, axi_rddata);
		ddr_mmio_rd32(4 * (168 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
		//axi_rddata[6] = 0;
		axi_rddata = modified_bits_by_value(axi_rddata, 0, 6, 6);
		//original                                     REGWR  (168 + PHY_BASE_ADDR, axi_rddata, 0);
		mmio_wr32(4 * (168 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
	}
	//----------------------------------------------------
	ddr_phy_power_on_seq2();
	set_dfi_init_complete();
}

void INT_ISR_00(void)
{
	uartlog("INT_ISR_00\n");
	//      repeat(7) @(posedge`clk_soc);
	//      force `semaphore = 1;
	//      repeat(10) @(posedge`clk_soc);
	//      force `semaphore = 0;
}

void INT_ISR_01(void)
{
	uartlog("INT_ISR_01\n");
	//param_phyd_to_reg_calvl_per_vref_done
	//axi_rddata[2] = 1'b0;
	axi_rddata = modified_bits_by_value(axi_rddata, 0, 2, 2);
	while (get_bits_from_value(axi_rddata, 2, 2) == 0) {
		////original                       REGREAD(1087 + PHY_BASE_ADDR, axi_rddata);
		ddr_mmio_rd32(4 * (1087 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
	}
	//if (~`top_module.u_bm_ddr_phyd_top.u_phyd_tc_top.u_train_calvl.calvl_mr42_wnd) {
	//param_phyd_to_reg_calvl_cs0_6~0_min_pass_valid[05:00] 1067
	//param_phyd_to_reg_calvl_cs1_6~0_min_pass_valid[21:16] 1067
	//param_phyd_to_reg_calvl_cs0_6~0_max_pass_valid[05:00] 1068
	//param_phyd_to_reg_calvl_cs1_6~0_max_pass_valid[21:16] 1068
	////original                                      REGREAD(1067 + PHY_BASE_ADDR, axi_rddata);
	ddr_mmio_rd32(4 * (1067 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
	////original                                      REGREAD(1068 + PHY_BASE_ADDR, axi_rddata_);
	ddr_mmio_rd32(4 * (1068 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata_);
	//min_pass_code
	////original                                      REGREAD(1069 + PHY_BASE_ADDR, axi_rddata1);
	ddr_mmio_rd32(4 * (1069 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata1);
	////original                                      REGREAD(1070 + PHY_BASE_ADDR, axi_rddata2);
	ddr_mmio_rd32(4 * (1070 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata2);
	////original                                      REGREAD(1071 + PHY_BASE_ADDR, axi_rddata3);
	ddr_mmio_rd32(4 * (1071 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata3);
	//max_pass_code
	////original                                      REGREAD(1079 + PHY_BASE_ADDR, axi_rddata6);
	ddr_mmio_rd32(4 * (1079 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata6);
	////original                                      REGREAD(1080 + PHY_BASE_ADDR, axi_rddata7);
	ddr_mmio_rd32(4 * (1080 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata7);
	////original                                      REGREAD(1081 + PHY_BASE_ADDR, axi_rddata8);
	ddr_mmio_rd32(4 * (1081 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata8);
	//if (~|axi_rddata[05:02])
	//if ((get_bits_from_value(axi_rddata, 5, 2) == 0))
	//if (~|axi_rddata1[13:00])
	//if ((get_bits_from_value(axi_rddata1, 13, 0) == 0))
	//if (~|axi_rddata_[05:00])
	//if ((get_bits_from_value(axi_rddata_, 5, 0) == 0))
	//if (~|axi_rddata6[13:00])
	//if ((get_bits_from_value(axi_rddata6, 13, 0) == 0))
#ifndef X16_MODE
	//param_phyd_to_reg_calvl_cs0_6~0_min_pass_valid[05:00] 1067
	//param_phyd_to_reg_calvl_cs1_6~0_min_pass_valid[21:16] 1067
	//param_phyd_to_reg_calvl_cs0_6~0_max_pass_valid[05:00] 1068
	//param_phyd_to_reg_calvl_cs1_6~0_max_pass_valid[21:16] 1068
	////original                                      REGREAD(1067 + PHY_BASE_ADDR, axi_rddata);
	ddr_mmio_rd32(4 * (1067 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
	////original                                      REGREAD(1068 + PHY_BASE_ADDR, axi_rddata_);
	ddr_mmio_rd32(4 * (1068 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata_);
	//min_pass_code
	////original                                      REGREAD(1074 + PHY_BASE_ADDR, axi_rddata1);
	ddr_mmio_rd32(4 * (1074 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata1);
	////original                                      REGREAD(1075 + PHY_BASE_ADDR, axi_rddata2);
	ddr_mmio_rd32(4 * (1075 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata2);
	////original                                      REGREAD(1076 + PHY_BASE_ADDR, axi_rddata3);
	ddr_mmio_rd32(4 * (1076 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata3);
	//max_pass_code
	////original                                      REGREAD(1084 + PHY_BASE_ADDR, axi_rddata6);
	ddr_mmio_rd32(4 * (1084 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata6);
	////original                                      REGREAD(1085 + PHY_BASE_ADDR, axi_rddata7);
	ddr_mmio_rd32(4 * (1085 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata7);
	////original                                      REGREAD(1086 + PHY_BASE_ADDR, axi_rddata8);
	ddr_mmio_rd32(4 * (1086 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata8);
	//if (~|axi_rddata[21:16])
	// if ((get_bits_from_value(axi_rddata, 21, 16)) == 0)
	// //if (~|axi_rddata1[13:00])
	// if ((get_bits_from_value(axi_rddata1, 13, 0)) == 0)
	// //if (~|axi_rddata_[21:16])
	// if ((get_bits_from_value(axi_rddata_, 21, 16)) == 0)
	// //if (~|axi_rddata6[13:00])
	// if ((get_bits_from_value(axi_rddata6, 13, 0)) == 0)
#endif
	ddr_mmio_rd32(4 * (1087 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
	if ((get_bits_from_value(axi_rddata, 4, 4) == 0x1) || (get_bits_from_value(axi_rddata, 2, 2) == 0x1)) {
		//Save LPDDR4 training information for further usage
		//lpddr4_bit0_min_pass_save = axi_rddata1[13:00];
		lpddr4_bit0_min_pass_save = get_bits_from_value(axi_rddata1, 13, 0);
		//lpddr4_bit1_min_pass_save = axi_rddata1[29:16];
		lpddr4_bit1_min_pass_save = get_bits_from_value(axi_rddata1, 29, 16);
		//lpddr4_bit2_min_pass_save = axi_rddata2[13:00];
		lpddr4_bit2_min_pass_save = get_bits_from_value(axi_rddata2, 13, 0);
		//lpddr4_bit3_min_pass_save = axi_rddata2[29:16];
		lpddr4_bit3_min_pass_save = get_bits_from_value(axi_rddata2, 29, 16);
		//lpddr4_bit4_min_pass_save = axi_rddata3[13:00];
		lpddr4_bit4_min_pass_save = get_bits_from_value(axi_rddata3, 13, 0);
		//lpddr4_bit5_min_pass_save = axi_rddata3[29:16];
		lpddr4_bit5_min_pass_save = get_bits_from_value(axi_rddata3, 29, 16);
		//lpddr4_bit0_max_pass_save = axi_rddata6[13:00];
		lpddr4_bit0_max_pass_save = get_bits_from_value(axi_rddata6, 13, 0);
		//lpddr4_bit1_max_pass_save = axi_rddata6[29:16];
		lpddr4_bit1_max_pass_save = get_bits_from_value(axi_rddata6, 29, 16);
		//lpddr4_bit2_max_pass_save = axi_rddata7[13:00];
		lpddr4_bit2_max_pass_save = get_bits_from_value(axi_rddata7, 13, 0);
		//lpddr4_bit3_max_pass_save = axi_rddata7[29:16];
		lpddr4_bit3_max_pass_save = get_bits_from_value(axi_rddata7, 29, 16);
		//lpddr4_bit4_max_pass_save = axi_rddata8[13:00];
		lpddr4_bit4_max_pass_save = get_bits_from_value(axi_rddata8, 13, 0);
		//lpddr4_bit5_max_pass_save = axi_rddata8[29:16];
		lpddr4_bit5_max_pass_save = get_bits_from_value(axi_rddata8, 29, 16);
		// min+max/2
		lpddr4_bit0_pass_save = (lpddr4_bit0_min_pass_save + lpddr4_bit0_max_pass_save) >> 1;
		lpddr4_bit1_pass_save = (lpddr4_bit1_min_pass_save + lpddr4_bit1_max_pass_save) >> 1;
		lpddr4_bit2_pass_save = (lpddr4_bit2_min_pass_save + lpddr4_bit2_max_pass_save) >> 1;
		lpddr4_bit3_pass_save = (lpddr4_bit3_min_pass_save + lpddr4_bit3_max_pass_save) >> 1;
		lpddr4_bit4_pass_save = (lpddr4_bit4_min_pass_save + lpddr4_bit4_max_pass_save) >> 1;
		lpddr4_bit5_pass_save = (lpddr4_bit5_min_pass_save + lpddr4_bit5_max_pass_save) >> 1;
		uartlog("[ISR01] lpddr4_bit0_min_pass_save=%x\n", lpddr4_bit0_min_pass_save);
		uartlog("[ISR01] lpddr4_bit1_min_pass_save=%x\n", lpddr4_bit1_min_pass_save);
		uartlog("[ISR01] lpddr4_bit2_min_pass_save=%x\n", lpddr4_bit2_min_pass_save);
		uartlog("[ISR01] lpddr4_bit3_min_pass_save=%x\n", lpddr4_bit3_min_pass_save);
		uartlog("[ISR01] lpddr4_bit4_min_pass_save=%x\n", lpddr4_bit4_min_pass_save);
		uartlog("[ISR01] lpddr4_bit5_min_pass_save=%x\n", lpddr4_bit5_min_pass_save);
		uartlog("[ISR01] lpddr4_bit0_max_pass_save=%x\n", lpddr4_bit0_max_pass_save);
		uartlog("[ISR01] lpddr4_bit1_max_pass_save=%x\n", lpddr4_bit1_max_pass_save);
		uartlog("[ISR01] lpddr4_bit2_max_pass_save=%x\n", lpddr4_bit2_max_pass_save);
		uartlog("[ISR01] lpddr4_bit3_max_pass_save=%x\n", lpddr4_bit3_max_pass_save);
		uartlog("[ISR01] lpddr4_bit4_max_pass_save=%x\n", lpddr4_bit4_max_pass_save);
		uartlog("[ISR01] lpddr4_bit5_max_pass_save=%x\n", lpddr4_bit5_max_pass_save);
		uartlog("[ISR01] lpddr4_bit0_pass_save =%x\n", lpddr4_bit0_pass_save);
		uartlog("[ISR01] lpddr4_bit1_pass_save =%x\n", lpddr4_bit1_pass_save);
		uartlog("[ISR01] lpddr4_bit2_pass_save =%x\n", lpddr4_bit2_pass_save);
		uartlog("[ISR01] lpddr4_bit3_pass_save =%x\n", lpddr4_bit3_pass_save);
		uartlog("[ISR01] lpddr4_bit4_pass_save =%x\n", lpddr4_bit4_pass_save);
		uartlog("[ISR01] lpddr4_bit5_pass_save =%x\n", lpddr4_bit5_pass_save);
		//force `lpddr4_calvl_done = 1;
		//param_phyd_reg_lpddr4_calvl_done <= `PI_SD int_regin[0];
		////original                                     REGREAD(240 + PHY_BASE_ADDR, axi_rddata);
		ddr_mmio_rd32(4 * (240 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
		//axi_rddata[0] = 1'b1;
		axi_rddata = modified_bits_by_value(axi_rddata, 1, 0, 0); //
		//original                                     REGWR  (240 + PHY_BASE_ADDR, axi_rddata, 0);
		mmio_wr32(4 * (240 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
		//param_phyd_calvl_sel_div40 =1
		//param_phyd_calvl_div40_set[12:0] <= `PI_SD int_regin[28:16];
		//param_phyd_calvl_sel_div40     <= `PI_SD int_regin[31];
		////original                                     REGREAD(168 + PHY_BASE_ADDR, axi_rddata);
		ddr_mmio_rd32(4 * (168 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
		//axi_rddata[28:16] = 13'b0001000000000;
		axi_rddata = modified_bits_by_value(axi_rddata, 0x200, 28, 16); //
		//axi_rddata[31] = 1'b1;
		axi_rddata = modified_bits_by_value(axi_rddata, 1, 31, 31); //
		//original                                     REGWR  (168 + PHY_BASE_ADDR, axi_rddata, 0);
		mmio_wr32(4 * (168 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
		//                                    force `lpddr4_calvl_done = 1;
		//
		//----------------------------------------------------
		//param_phyd_calvl_resp_auto     <= `PI_SD int_regin[31];
		//resp auto ?
		////original       REGREAD(165 + PHY_BASE_ADDR, axi_rddata);
		ddr_mmio_rd32(4 * (165 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
		//if (axi_rddata[31] == 1'b0) {
		if (get_bits_from_value(axi_rddata, 31, 31) == 0) {
			// param_phyd_tx_ca3_shift_sel[5:0] <= int_regin[29:24];
			// param_phyd_tx_ca2_shift_sel[5:0] <= int_regin[21:16];
			// param_phyd_tx_ca1_shift_sel[5:0] <= int_regin[13:8];
			// param_phyd_tx_ca0_shift_sel[5:0] <= int_regin[5:0];
			//REGREAD(134 + PHY_BASE_ADDR, axi_rddata);
			//axi_rddata[31:0]  = 0x0;
			//axi_rddata[29:24] = 4;
			//axi_rddata[21:16] = 4;
			//axi_rddata[13:8]  = 4;
			//axi_rddata[5:0]   = 4;
			axi_rddata = modified_bits_by_value(axi_rddata, 0x0, 31, 0); //
			axi_rddata = modified_bits_by_value(axi_rddata, 0x4, 29, 24); //
			axi_rddata = modified_bits_by_value(axi_rddata, 0x4, 21, 16); //
			axi_rddata = modified_bits_by_value(axi_rddata, 0x4, 13, 8); //
			axi_rddata = modified_bits_by_value(axi_rddata, 0x4, 5, 0); //
			//axi_rddata[29:24] = lpddr4_bit3_pass_save[12:7];
			//axi_rddata[21:16] = lpddr4_bit2_pass_save[12:7];
			//axi_rddata[13:8]  = lpddr4_bit1_pass_save[12:7];
			//axi_rddata[5:0]   = lpddr4_bit0_pass_save[12:7];
			//original                                     REGWR  (134 + PHY_BASE_ADDR, axi_rddata, 0);
			mmio_wr32(4 * (134 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
			// param_phyd_tx_ca7_shift_sel[5:0] <= int_regin[29:24];
			// param_phyd_tx_ca6_shift_sel[5:0] <= int_regin[21:16];
			// param_phyd_tx_ca5_shift_sel[5:0] <= int_regin[13:8];
			// param_phyd_tx_ca4_shift_sel[5:0] <= int_regin[5:0];
			//REGREAD(135 + PHY_BASE_ADDR, axi_rddata);
			//axi_rddata[31:0]  = 0x0;
			//axi_rddata[29:24] = 4;
			//axi_rddata[21:16] = 4;
			//axi_rddata[13:8]  = 4;
			//axi_rddata[5:0]   = 4;
			axi_rddata = modified_bits_by_value(axi_rddata, 0x0, 31, 0); //
			axi_rddata = modified_bits_by_value(axi_rddata, 0x4, 29, 24); //
			axi_rddata = modified_bits_by_value(axi_rddata, 0x4, 21, 16); //
			axi_rddata = modified_bits_by_value(axi_rddata, 0x4, 13, 8); //
			axi_rddata = modified_bits_by_value(axi_rddata, 0x4, 5, 0); //
			//axi_rddata[29:24] = lpddr4_bit7_pass_save[12:7];
			//axi_rddata[21:16] = lpddr4_bit6_pass_save[12:7];
			//axi_rddata[13:8]  = lpddr4_bit5_pass_save[12:7];
			//axi_rddata[5:0]   = lpddr4_bit4_pass_save[12:7];
			//original                                     REGWR  (135 + PHY_BASE_ADDR, axi_rddata, 0);
			mmio_wr32(4 * (135 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
			// param_phyd_tx_ca11_shift_sel[5:0] <= int_regin[29:24];
			// param_phyd_tx_ca10_shift_sel[5:0] <= int_regin[21:16];
			// param_phyd_tx_ca9_shift_sel[5:0] <= int_regin[13:8];
			// param_phyd_tx_ca8_shift_sel[5:0] <= int_regin[5:0];
			//axi_rddata[31:0]  = 0x0;
			//axi_rddata[29:24] = 4;
			//axi_rddata[21:16] = 4;
			//axi_rddata[13:8]  = 4;
			//axi_rddata[5:0]   = 4;
			axi_rddata = modified_bits_by_value(axi_rddata, 0x0, 31, 0); //
			axi_rddata = modified_bits_by_value(axi_rddata, 0x4, 29, 24); //
			axi_rddata = modified_bits_by_value(axi_rddata, 0x4, 21, 16); //
			axi_rddata = modified_bits_by_value(axi_rddata, 0x4, 13, 8); //
			axi_rddata = modified_bits_by_value(axi_rddata, 0x4, 5, 0); //
			//axi_rddata[13:8]  = lpddr4_bit9_pass_save[12:7];
			//axi_rddata[5:0]   = lpddr4_bit8_pass_save[12:7];
			//original                                     REGWR  (136 + PHY_BASE_ADDR, axi_rddata, 0);
			mmio_wr32(4 * (136 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
			// param_phyd_reg_tx_ca_tx_dline_code_ca3_sw[6:0] <= int_regin[30:24];
			// param_phyd_reg_tx_ca_tx_dline_code_ca2_sw[6:0] <= int_regin[22:16];
			// param_phyd_reg_tx_ca_tx_dline_code_ca1_sw[6:0] <= int_regin[14:8];
			// param_phyd_reg_tx_ca_tx_dline_code_ca0_sw[6:0] <= int_regin[6:0];
			//REGREAD(145 + PHY_BASE_ADDR, axi_rddata);
			axi_rddata = 0;
			//axi_rddata[30:24] = lpddr4_bit3_pass_save[6:0];
			//axi_rddata[22:16] = lpddr4_bit2_pass_save[6:0];
			//axi_rddata[14:8]  = lpddr4_bit1_pass_save[6:0];
			//axi_rddata[6:0]   = lpddr4_bit0_pass_save[6:0];
			//original                                     REGWR  (145 + PHY_BASE_ADDR, axi_rddata, 0);
			mmio_wr32(4 * (145 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
			// param_phyd_reg_tx_ca_tx_dline_code_ca7_sw[6:0] <= int_regin[30:24];
			// param_phyd_reg_tx_ca_tx_dline_code_ca6_sw[6:0] <= int_regin[22:16];
			// param_phyd_reg_tx_ca_tx_dline_code_ca5_sw[6:0] <= int_regin[14:8];
			// param_phyd_reg_tx_ca_tx_dline_code_ca4_sw[6:0] <= int_regin[6:0];
			//REGREAD(146 + PHY_BASE_ADDR, axi_rddata);
			axi_rddata = 0x0;
			//axi_rddata[30:24] = lpddr4_bit7_pass_save[6:0];
			//axi_rddata[22:16] = lpddr4_bit6_pass_save[6:0];
			//axi_rddata[14:8]  = lpddr4_bit5_pass_save[6:0];
			//axi_rddata[6:0]   = lpddr4_bit4_pass_save[6:0];
			//original                                     REGWR  (146 + PHY_BASE_ADDR, axi_rddata, 0);
			mmio_wr32(4 * (146 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
			// param_phyd_reg_tx_ca_tx_dline_code_ca9_sw[6:0] <= int_regin[14:8];
			// param_phyd_reg_tx_ca_tx_dline_code_ca8_sw[6:0] <= int_regin[6:0];
			//REGREAD(147 + PHY_BASE_ADDR, axi_rddata);
			axi_rddata = 0x0;
			//axi_rddata[14:8]  = lpddr4_bit9_pass_save[6:0];;
			//axi_rddata[6:0]   = lpddr4_bit8_pass_save[6:0];;
			//original                                     REGWR  (147 + PHY_BASE_ADDR, axi_rddata, 0);
			mmio_wr32(4 * (147 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
		} //resp auto =0
		//Do SW update
		//param_phyd_calvl_sw_upd        <= int_regin[6];
		////original                                     REGREAD(168 + PHY_BASE_ADDR, axi_rddata);
		ddr_mmio_rd32(4 * (168 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
		//axi_rddata[6] = 1;
		axi_rddata = modified_bits_by_value(axi_rddata, 0x1, 6, 6); //
		//original                                     REGWR  (168 + PHY_BASE_ADDR, axi_rddata, 0);
		mmio_wr32(4 * (168 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
		//Wait for SW update ACK
		//axi_rddata[0] = 0;
		axi_rddata = modified_bits_by_value(axi_rddata, 0x0, 0, 0); //
		while (get_bits_from_value(axi_rddata, 0, 0) == 0) {
			////original                                       REGREAD(1087 + PHY_BASE_ADDR, axi_rddata);
			ddr_mmio_rd32(4 * (1087 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
		}
		//Clear SW update
		//param_phyd_calvl_sw_upd        <= int_regin[6];
		////original                                     REGREAD(168 + PHY_BASE_ADDR, axi_rddata);
		ddr_mmio_rd32(4 * (168 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
		//axi_rddata[6] = 0;
		axi_rddata = modified_bits_by_value(axi_rddata, 0x0, 6, 6); //
		//original                                     REGWR  (168 + PHY_BASE_ADDR, axi_rddata, 0);
		mmio_wr32(4 * (168 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
		//param_phyd_calvl_sel_div40 =0
		//param_phyd_calvl_sel_div40     <= `PI_SD int_regin[31];
		////original                                     REGREAD(168 + PHY_BASE_ADDR, axi_rddata);
		ddr_mmio_rd32(4 * (168 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
		//axi_rddata[31] = 1'b0;
		axi_rddata = modified_bits_by_value(axi_rddata, 0x0, 31, 31); //
		//original                                     REGWR  (168 + PHY_BASE_ADDR, axi_rddata, 0);
		mmio_wr32(4 * (168 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
		//param_phyd_calvl_stop_ack      <= int_regin[7]
		////original                                     REGREAD(168 + PHY_BASE_ADDR, axi_rddata);
		ddr_mmio_rd32(4 * (168 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
		//axi_rddata[7] = 1;
		axi_rddata = modified_bits_by_value(axi_rddata, 0x1, 7, 7); //
		//original                                     REGWR  (168 + PHY_BASE_ADDR, axi_rddata, 0);
		mmio_wr32(4 * (168 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
		//param_phyd_calvl_stop_ack      <= int_regin[7]
		////original                                     REGREAD(168 + PHY_BASE_ADDR, axi_rddata);
		ddr_mmio_rd32(4 * (168 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
		//axi_rddata[7] = 0;
		axi_rddata = modified_bits_by_value(axi_rddata, 0x0, 7, 7); //
		//original                                     REGWR  (168 + PHY_BASE_ADDR, axi_rddata, 0);
		mmio_wr32(4 * (168 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
	} //if
}

void isr_process(void)
{
	//Read interrupt status
	while (1) {
		ddr_mmio_rd32(4 * (1095 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
		if (axi_rddata != 0)
			break;
	}
	uartlog("ISR enters\n");
	int_sts_keep = axi_rddata;
	if (get_bits_from_value(int_sts_keep, 8, 8) == 1)
		INT_ISR_08();

	if (get_bits_from_value(int_sts_keep, 0, 0) == 1) {
		INT_ISR_00();
		//Clear interrupt
		////original          REGREAD(179 + PHY_BASE_ADDR, axi_rddata);
		ddr_mmio_rd32(4 * (179 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
		//axi_rddata[0] = 1;
		axi_rddata = modified_bits_by_value(axi_rddata, 1, 0, 0); //
		//original          REGWR  (179 + PHY_BASE_ADDR, axi_rddata, 0);
		mmio_wr32(4 * (179 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
	}
	if (get_bits_from_value(int_sts_keep, 1, 1) == 1) {
		INT_ISR_01();
		//Clear interrupt
		////original          REGREAD(179 + PHY_BASE_ADDR, axi_rddata);
		ddr_mmio_rd32(4 * (179 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
		//axi_rddata[1] = 1;
		axi_rddata = modified_bits_by_value(axi_rddata, 1, 1, 1); //
		//original          REGWR  (179 + PHY_BASE_ADDR, axi_rddata, 0);
		mmio_wr32(4 * (179 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
	}
	if (get_bits_from_value(int_sts_keep, 2, 2) == 1) {
		INT_ISR_02();
		//Clear interrupt
		////original          REGREAD(179 + PHY_BASE_ADDR, axi_rddata);
		ddr_mmio_rd32(4 * (179 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
		//axi_rddata[2] = 1;
		axi_rddata = modified_bits_by_value(axi_rddata, 1, 2, 2); //
		//original          REGWR  (179 + PHY_BASE_ADDR, axi_rddata, 0);
		mmio_wr32(4 * (179 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
	}
}

void clk_div40(void)
{
	//
	//original apb_read(0x0c, ken_rddata);
	ddr_mmio_rd32(0x0c + CADENCE_PHYD_APB, apb_rddata);
	//ken_rddata[13] = 1  ;  // TOP_REG_DDRPLL_SEL_LOW_SPEED 1
	apb_rddata = modified_bits_by_value(apb_rddata, 1, 13, 13); //
	////original apb_write(0x0c, ken_rddata);
	mmio_wr32(0x0c + CADENCE_PHYD_APB, apb_rddata);
	uartlog("Enter low D40 frequency\n");
#ifdef SSC_BYPASS
	//==============================================================
	//Bypass SSC for clk_div40 (jitters issue)
	//Fix clock = 1.5G/40 = 37.5 ???
	//==============================================================
	//1.5G / 6 = 250M, clk = 2G
	//1.5G / 7 = 214.3M clk = 1.7G
	//1.5G / 8 = 187.5M clk = 1.5G
	//1.5G /12 = 125M clk = 1G
	//original apb_read(0x30, apb_rddata);
	ddr_mmio_rd32(0x30 + CADENCE_PHYD_APB, apb_rddata);
	//apb_rddata[0]     = 0;                    //TOP_REG_SSC_SW_UP
	//original apb_rddata[1]     = 0;                    //TOP_REG_SSC_EN_SSC
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 1, 1); //
	//original apb_rddata[3:2]   = 2'b00;                //TOP_REG_SSC_SSC_MODE
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 3, 2); //TOP_REG_SSC_SSC_MODE
	//original apb_rddata[4]     = 0;                    //TOP_REG_SSC_SEL
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 4, 4); //
	//original apb_rddata[8]     = mem_freq_2133;       //TOP_REG_CLKDIV_DIV6EN
	apb_rddata = modified_bits_by_value(apb_rddata, mem_freq_2133, 8, 8); //
	//original apb_rddata[9]     = 0;                    //TOP_REG_CLKDIV_DIV7EN
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 9, 9); //
	//original apb_rddata[10]    = mem_freq_1600;       //TOP_REG_CLKDIV_DIV8EN
	apb_rddata = modified_bits_by_value(apb_rddata, mem_freq_1600, 10, 10); //
	//original apb_rddata[11]    = mem_freq_1066;       //TOP_REG_CLKDIV_DIV12EN
	apb_rddata = modified_bits_by_value(apb_rddata, mem_freq_1066, 11, 11); //
	//original apb_rddata[13:12] = clkdiv_clksel;        //TOP_REG_CLKDIV_CLKSEL
	apb_rddata = modified_bits_by_value(apb_rddata, clkdiv_clksel, 13, 12); //TOP_REG_CLKDIV_CLKSEL
	////original apb_write(0x30, apb_rddata);
	mmio_wr32(0x30 + CADENCE_PHYD_APB, apb_rddata);
	uartlog("SSC_BYPASS_DIV40\n");
#else
	//==============================================================
	//SSC_EN =0
	//==============================================================
	uartlog("SSC_EN =0\n");
	//original apb_read(0x34, apb_rddata);
	ddr_mmio_rd32(0x34 + CADENCE_PHYD_APB, apb_rddata);
	//original apb_rddata[31:0] = reg_set[31:0];         //TOP_REG_SSC_SET
	apb_rddata = modified_bits_by_value(apb_rddata, reg_set, 31, 0); //TOP_REG_SSC_SET
	////original apb_write(0x34, apb_rddata);
	mmio_wr32(0x34 + CADENCE_PHYD_APB, apb_rddata);
	//original apb_read(0x38, apb_rddata);
	ddr_mmio_rd32(0x38 + CADENCE_PHYD_APB, apb_rddata);
	//original apb_rddata[31:0] = 0;                 //TOP_REG_SSC_SPAN
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 31, 0); //TOP_REG_SSC_SPAN
	////original apb_write(0x38, apb_rddata);
	mmio_wr32(0x38 + CADENCE_PHYD_APB, apb_rddata);
	//original apb_read(0x3C, apb_rddata);
	ddr_mmio_rd32(0x3C + CADENCE_PHYD_APB, apb_rddata);
	//original apb_rddata[31:0] = reg_step[23:0];        //TOP_REG_SSC_STEP
	apb_rddata = modified_bits_by_value(apb_rddata, get_bits_from_value(reg_step, 23, 0), 31, 0); //TOP_REG_SSC_STEP
	////original apb_write(0x3C, apb_rddata);
	mmio_wr32(0x3C + CADENCE_PHYD_APB, apb_rddata);
	//original apb_read(0x30, apb_rddata);
	ddr_mmio_rd32(0x30 + CADENCE_PHYD_APB, apb_rddata);
	//original apb_rddata[0]     = ~apb_rddata[0];       //TOP_REG_SSC_SW_UP
	apb_rddata = modified_bits_by_value(apb_rddata, ~(get_bits_from_value(apb_rddata, 0, 0)), 0, 0); //
	//original apb_rddata[1]     = 0;                    //TOP_REG_SSC_EN_SSC
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 1, 1); //
	//original apb_rddata[3:2]   = 2'b00;                //TOP_REG_SSC_SSC_MODE
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 3, 2); //TOP_REG_SSC_SSC_MODE
	//original apb_rddata[4]     = 1;                    //TOP_REG_SSC_SEL
	apb_rddata = modified_bits_by_value(apb_rddata, 1, 4, 4); //
	////original apb_write(0x30, apb_rddata);
	mmio_wr32(0x30 + CADENCE_PHYD_APB, apb_rddata);
	uartlog("SSC_OFF\n");
#endif //SSC_BYPASS
}

void ddr_phy_sync_dfi_timing(void)
{
	// //KC test  sync_dfi_timing
	uartlog("ddr_phy_sync_dfi_timing\n");
	//sync dfi timing synp<->PI
	mmio_wr32(cfg_base + 0x00000320, 0x00000000);
	//read param_pi_tdfi_ctrl_delay_f2[3:0] <= `PI_SD int_regin[11:8];
	////original        REGREAD(PI_BASE_ADDR+139, read_temp);
	ddr_mmio_rd32(4 * (PI_BASE_ADDR + 139) + CADENCE_PHYD, read_temp);
	//update cfgs_ff_dfi_t_ctrl_delay DFITMG0[28:24] @ 0x190
	ddr_mmio_rd32(cfg_base + 0x00000190, read_data);
	//original        read_data[28:24] = {1'b0, read_temp[11:8]};
	read_temp = get_bits_from_value(read_temp, 11, 8);
	read_data = modified_bits_by_value(read_data, read_temp, 28, 24); //
	mmio_wr32(cfg_base + 0x00000190, read_data);
	//read param_pi_tdfi_phy_wrdata_f2[2:0] <= `PI_SD int_regin[26:24];
	////original        REGREAD(PI_BASE_ADDR+153, read_temp);
	ddr_mmio_rd32(4 * (PI_BASE_ADDR + 153) + CADENCE_PHYD, read_temp);
	//update cfgs_ff_dfi_tphy_wrdata DFITMG0[13:8] @ 0x190
	ddr_mmio_rd32(cfg_base + 0x00000190, read_data);
	//original        read_data[13:8] = {3'b000, read_temp[26:24]};
	read_temp = get_bits_from_value(read_temp, 26, 24);
	read_data = modified_bits_by_value(read_data, read_temp, 13, 8); //
	mmio_wr32(cfg_base + 0x00000190, read_data);
	//read param_pi_wrlat_adj_f2[7:0] <= `PI_SD int_regin[7:0];
	////original        REGREAD(PI_BASE_ADDR+153, read_temp);
	ddr_mmio_rd32(4 * (PI_BASE_ADDR + 153) + CADENCE_PHYD, read_temp);
	//update cfgs_ff_dfi_tphy_wrlat DFITMG0[5:0] @ 0x190
	ddr_mmio_rd32(cfg_base + 0x00000190, read_data);
#ifdef DDR4
	//original        read_data[5:0] = read_temp[5:0] -1;  //tdfi_phy_wrlat = wrlat_adj_fN + reg_dimm_enable - 1 @page. 549
	read_temp = get_bits_from_value(read_temp, 5, 0);
	read_data = modified_bits_by_value(read_data, read_temp - 1, 5, 0); //tdfi_phy_wrlat = wrlat_adj_fN + reg_dimm_enable - 1 @page. 549
#endif
#ifdef DDR3
	//original        read_data[5:0] = read_temp[5:0] -1;
	//tdfi_phy_wrlat = wrlat_adj_fN + reg_dimm_enable - 1 @page. 549
	read_data = modified_bits_by_value(read_data, read_temp - 1, 5, 0); //tdfi_phy_wrlat = wrlat_adj_fN + reg_dimm_enable - 1 @page. 549
#endif
#ifdef LP4
	//original        read_data[5:0] = read_temp[5:0];     //Note: For LPDDR3 memory systems (dram_class = 0b0111) or LPDDR4
	read_data = modified_bits_by_value(read_data, read_temp, 5, 0); //Note: For LPDDR3 memory systems (dram_class = 0b0111) or LPDDR4
	//memory systems (dram_class = 0b1011),
	//the wrlat_adj_fN value is incremented
	//inside the controller by 1, and therefore
	//the observed value for PHY write latency
	//will be 1 greater than this equation.
#endif
	mmio_wr32(cfg_base + 0x00000190, read_data);
	//read param_pi_rdlat_adj_f2[7:0] <= `PI_SD int_regin[15:8];
	////original        REGREAD(PI_BASE_ADDR+152, read_temp);
	ddr_mmio_rd32(4 * (PI_BASE_ADDR + 152) + CADENCE_PHYD, read_temp);
	//update cfgs_ff_dfi_t_rddata_en DFITMG0[22:16] @ 0x190
	//original ddr_mmio_rd32(cfg_base+0x00000190, read_data);//original       read_data[22:16] = read_temp[14:8] -1;  //tdfi_rddata_en = rdlat_adj_fN + reg_dimm_enable - 1 @page. 557
	read_data = modified_bits_by_value(read_data, get_bits_from_value(read_temp, 14, 8) - 1, 22, 16); //tdfi_rddata_en = rdlat_adj_fN + reg_dimm_enable - 1 @page. 557
	read_temp = get_bits_from_value(read_temp, 14, 8);
	read_data = modified_bits_by_value(read_data, read_temp - 1, 22, 16); //tdfi_rddata_en = rdlat_adj_fN + reg_dimm_enable - 1 @page. 557
	mmio_wr32(cfg_base + 0x00000190, read_data);
}

void ddr_phy_power_on_seq1(void)
{
	//power_seq_1
	// synp setting
	// phy is ready for initial dfi_init_start request
	// set umctl2 to tigger dfi_init_start
	uartlog("ddr_phy_power_on_seq1\n");
	//RESETZ/CKE PD=0
	//original        apb_read(0x40, apb_rddata);
	ddr_mmio_rd32(0x40 + CADENCE_PHYD_APB, apb_rddata);
	//original        apb_rddata[24] = 0;
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 24, 24); //
	//original        apb_rddata[25] = 0;
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 25, 25); //
	//original        apb_rddata[30] = 0;
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 30, 30); //
	////original        apb_write(0x40, apb_rddata);
	mmio_wr32(0x40 + CADENCE_PHYD_APB, apb_rddata);
	//CA PD=0
	//All PHYA CA PD=0
	//original        apb_read(0x40, apb_rddata);
	ddr_mmio_rd32(0x40 + CADENCE_PHYD_APB, apb_rddata);
	//original        apb_rddata[31:0] = 0;
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 31, 0); //
	////original        apb_write(0x40, apb_rddata);
	mmio_wr32(0x40 + CADENCE_PHYD_APB, apb_rddata);
	//TOP_REG_TX_SEL_GPIO = 1 (DQ)
	//original        apb_read(0x1c, apb_rddata);
	ddr_mmio_rd32(0x1c + CADENCE_PHYD_APB, apb_rddata);
	//original        apb_rddata[7] = 1;
	apb_rddata = modified_bits_by_value(apb_rddata, 1, 7, 7); //
	////original        apb_write(0x1c, apb_rddata);
	mmio_wr32(0x1c + CADENCE_PHYD_APB, apb_rddata);
	//DQ PD=0
	//force memcd_test.TOP_REG_TX_BYTE0_PD            = 1'b0 ;
	//force memcd_test.TOP_REG_TX_BYTE1_PD            = 1'b0 ;
	//force memcd_test.TOP_REG_TX_BYTE2_PD            = 1'b0 ;
	//force memcd_test.TOP_REG_TX_BYTE3_PD            = 1'b0 ;
	//force TOP_REG_DDRPLL_PD                         = 1'b0;
	//original        apb_read(0x00, apb_rddata);
	ddr_mmio_rd32(0x00 + CADENCE_PHYD_APB, apb_rddata);
	//original        apb_rddata[0]   = 0;    //TOP_REG_TX_BYTE0_PD
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 0, 0); //
	//original        apb_rddata[1]   = 0;    //TOP_REG_TX_BYTE1_PD
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 1, 1); //
	//original        apb_rddata[2]   = 0;    //TOP_REG_TX_BYTE2_PD
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 2, 2); //
	//original        apb_rddata[3]   = 0;    //TOP_REG_TX_BYTE3_PD
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 3, 3); //
	//original        apb_rddata[4]   = 0;    //TOP_REG_TX_CA_PD
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 4, 4); //
	////original        apb_write(0x00, apb_rddata);
	mmio_wr32(0x00 + CADENCE_PHYD_APB, apb_rddata);
	//TOP_REG_TX_SEL_GPIO = 0 (DQ)
	//original        apb_read(0x1c, apb_rddata);
	ddr_mmio_rd32(0x1c + CADENCE_PHYD_APB, apb_rddata);
	//original        apb_rddata[7] = 0;
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 7, 7); //
	////original        apb_write(0x1c, apb_rddata);
	mmio_wr32(0x1c + CADENCE_PHYD_APB, apb_rddata);
	// power_seq_1
}

void ddr_phy_power_on_seq2(void)
{
	uartlog("ddr_phy_power_on_seq2\n");
	//Change PLL frequency
	chg_pll_freq();
	//OEN
	//param_phyd_sel_cke_oenz        <= `PI_SD int_regin[0];
	ddr_mmio_rd32(4 * (132 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = modified_bits_by_value(rddata, 0, 0, 0);
	mmio_wr32(4 * (132 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	//param_phyd_tx_ca_oenz          <= `PI_SD int_regin[10];
	//param_phyd_tx_ca_clk0_oenz     <= `PI_SD int_regin[11];
	//param_phyd_tx_ca_clk1_oenz     <= `PI_SD int_regin[12];
	////original      REGREAD(142 + PHY_BASE_ADDR, rddata);
	ddr_mmio_rd32(4 * (142 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = modified_bits_by_value(rddata, 0, 10, 10);
	rddata = modified_bits_by_value(rddata, 0, 11, 11);
	rddata = modified_bits_by_value(rddata, 0, 12, 12);
	mmio_wr32(4 * (142 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	//Do DLLCAL if necessary
	dll_cal();
	//      ddr_zqcal_isr8;
	ddr_zqcal_hw_isr8();
	#if 0 //TODO: Wait for golden setting from ATE
	//zq calculate variation
	zq_cal_var();
	#endif
	//CA PD =0
	//All PHYA CA PD=0
	//original      apb_read(0x40, apb_rddata);
	ddr_mmio_rd32(0x40 + CADENCE_PHYD_APB, apb_rddata);
	//original      apb_rddata[30:0] = 0;
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 30, 0); //
	//TOP_REG_TX_ZQ_PD = 1
	//apb_rddata[31] = 0;
	////original      apb_write(0x40, apb_rddata);
	mmio_wr32(0x40 + CADENCE_PHYD_APB, apb_rddata);
	//force memcd_test.TOP_REG_TX_BYTE0_PD            = 1'b0 ;
	//force memcd_test.TOP_REG_TX_BYTE1_PD            = 1'b0 ;
	//force memcd_test.TOP_REG_TX_BYTE2_PD            = 1'b0 ;
	//force memcd_test.TOP_REG_TX_BYTE3_PD            = 1'b0 ;
	//force TOP_REG_DDRPLL_PD                         = 1'b0;
	//BYTE PD =0
	//original      apb_read(0x00, apb_rddata);
	ddr_mmio_rd32(0x00 + CADENCE_PHYD_APB, apb_rddata);
	//original      apb_rddata[0]   = 0;    //TOP_REG_TX_BYTE0_PD
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 0, 0); //
	//original      apb_rddata[1]   = 0;    //TOP_REG_TX_BYTE1_PD
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 1, 1); //
	//original      apb_rddata[2]   = 0;    //TOP_REG_TX_BYTE2_PD
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 2, 2); //
	//original      apb_rddata[3]   = 0;    //TOP_REG_TX_BYTE3_PD
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 3, 3); //
	//original      apb_rddata[4]   = 0;    //TOP_REG_TX_CA_PD
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 4, 4); //
	//apb_rddata[5]   = 0;    //TOP_REG_DDRPLL_PD
	////original      apb_write(0x00, apb_rddata);
	mmio_wr32(0x00 + CADENCE_PHYD_APB, apb_rddata);
	//power_on_2
}

void ddr_phy_power_on_seq3(void)
{
	//power on 3
	uartlog("ddr_phy_power_on_seq3\n");
	//RESETYZ/CKE OENZ
	//param_phyd_sel_cke_oenz        <= `PI_SD int_regin[0];
	////original        REGREAD(132 + PHY_BASE_ADDR, rddata);
	ddr_mmio_rd32(4 * (132 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = modified_bits_by_value(rddata, 0, 0, 0); //
	mmio_wr32(4 * (132 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	//CA/CLK OENZ
	//param_phyd_tx_ca_oenz          <= `PI_SD int_regin[10];
	//param_phyd_tx_ca_clk0_oenz     <= `PI_SD int_regin[11];
	//param_phyd_tx_ca_clk1_oenz     <= `PI_SD int_regin[12];
	////original        REGREAD(142 + PHY_BASE_ADDR, rddata);
	ddr_mmio_rd32(4 * (142 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = modified_bits_by_value(rddata, 0, 10, 10); //
	rddata = modified_bits_by_value(rddata, 0, 11, 11); //
	rddata = modified_bits_by_value(rddata, 0, 12, 12); //
	//original        REGWR  (142 + PHY_BASE_ADDR, rddata, 0);
	mmio_wr32(4 * (142 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	//clock gated for power save
	//param_phya_reg_tx_byte0_en_extend_oenz_gated_dline <= `PI_SD int_regin[0];
	//param_phya_reg_tx_byte1_en_extend_oenz_gated_dline <= `PI_SD int_regin[1];
	//param_phya_reg_tx_byte2_en_extend_oenz_gated_dline <= `PI_SD int_regin[2];
	//param_phya_reg_tx_byte3_en_extend_oenz_gated_dline <= `PI_SD int_regin[3];
	ddr_mmio_rd32(4 * (88 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	//original        rddata[3:0] = 0b1111;
	rddata = modified_bits_by_value(rddata, 15, 3, 0); //
	mmio_wr32(4 * (88 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
}

void wait_for_dfi_init_complete(void)
{
	uartlog("wait_for_dfi_init_complete\n");
	// synp setting
	while (1) {
		ddr_mmio_rd32(cfg_base+0x000001bc, read_data);
			//} while ((read_data & 0x00000001) != 1);
			if (get_bits_from_value(read_data, 0, 0) == 1)
				break;
		}
	mmio_wr32(cfg_base + 0x00000320, 0x00000000);
	ddr_mmio_rd32(cfg_base + 0x000001b0, read_data);
	read_data = modified_bits_by_value(rddata, 5, 5, 0); //
	mmio_wr32(cfg_base + 0x000001b0, read_data);
	mmio_wr32(cfg_base + 0x00000320, 0x00000001);
}

void ctrlupd_short(void)
{
	// ctrlupd short
	uartlog("ctrlupd_short\n");
	// for gate track
	// dis_auto_ctrlupd [31] =0, dis_auto_ctrlupd_srx [30] =0, ctrlupd_pre_srx [29] =1 @ 0x1a0
	ddr_mmio_rd32(cfg_base + 0x000001a0, read_data);
	//original        read_data[31:29] = 3'b001;
	read_data = modified_bits_by_value(read_data, 1, 31, 29); //
	mmio_wr32(cfg_base + 0x000001a0, read_data);
	// dfi_t_ctrlupd_interval_min_x1024 [23:16] = 4, dfi_t_ctrlupd_interval_max_x1024 [7:0] = 8 @ 0x1a4
	ddr_mmio_rd32(cfg_base + 0x000001a4, read_data);
	//original        read_data[23:16] = 0x04;
	read_data = modified_bits_by_value(read_data, 4, 23, 16); //
	//original        read_data[7:0] = 0x08;
	read_data = modified_bits_by_value(read_data, 8, 7, 0); //
	mmio_wr32(cfg_base + 0x000001a4, read_data);
}

void wait_for_pi_training_done(void)
{
	uartlog("wait_for_pi_training_done\n");
	//param_pi_int_status <= `PI_SD {`PI_NUM_INT_SOURCES+1{1'b0}}
	ddr_mmio_rd32(4 * (83 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	while (get_bits_from_value(rddata, 0, 0) == 0) {
		//Read PHY interrupt status
		ddr_mmio_rd32(4 * (1095 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		if (rddata != 0) {
			isr_process();
		}
		ddr_mmio_rd32(4 * (83 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	}
	mmio_wr32(4 * (84 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	//    ////param_pi_int_status <= `PI_SD {`PI_NUM_INT_SOURCES+1{1'b0}}
	//    ddr_mmio_rd32(4 * (83 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	//    while (get_bits_from_value(rddata,0, 0) == 0x0) {
	//        ddr_mmio_rd32(4 * (83 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	//    }
	//    //original           REGWR  (84 + PI_BASE_ADDR, rddata, 0);
	//    mmio_wr32(4 * (84 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
}

void polling_dfi_init_start(void)
{
	uartlog("polling_dfi_init_start\n");
	while (1) {
		ddr_mmio_rd32(4 * (1095 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
		if (get_bits_from_value(axi_rddata, 8, 8) == 1)
			break;
	}
}

void set_dfi_init_complete(void)
{
	uartlog("set_dfi_init_complete\n");
	ddr_mmio_rd32(4 * (179 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
	//axi_rddata[8] = 1;
	axi_rddata = modified_bits_by_value(axi_rddata, 1, 8, 8);
	//original          REGWR  (179 + PHY_BASE_ADDR, axi_rddata, 0);
	mmio_wr32(4 * (179 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);

	//param_phyd_clkctrl_init_complete   <= int_regin[24];
	////original       REGREAD(133 + PHY_BASE_ADDR, axi_rddata);
	ddr_mmio_rd32(4 * (133 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
	//axi_rddata[24] = 1;
	axi_rddata = modified_bits_by_value(axi_rddata, 1, 24, 24);
	//original       REGWR  (133 + PHY_BASE_ADDR, axi_rddata, 0);
	mmio_wr32(4 * (133 + PHY_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
}

void polling_synp_normal_mode(void)
{
	//synp ctrl operating_mode
	while (1) {
		ddr_mmio_rd32(cfg_base + 0x00000004, read_data);
		if (get_bits_from_value(read_data, 2, 0) == 1)
			break;
	}
}

void set_pi_start(void)
{
	uartlog("set_pi_start\n");
	ddr_mmio_rd32(4 * (0 + PI_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
	axi_rddata = modified_bits_by_value(axi_rddata, 1, 0, 0);
	mmio_wr32(4 * (0 + PI_BASE_ADDR) + CADENCE_PHYD, axi_rddata);
}

void pi_reg_adj(void)
{
	uint32_t dram_class;
	//------------------------------
	//  read dram class
	//------------------------------
	//param_pi_dram_class[3:0] <= `PI_SD int_regin[11:8];
	ddr_mmio_rd32(4 * (0 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	//dram_class = rddata[11:8];  //DDR3: 4'b0110, DDR4: 4'b1010, LPDDR3: 4'b0111, LPDDR4: 4'b1011
	dram_class = get_bits_from_value(rddata, 11, 8);
	uartlog("pi_reg_adj\n");
	//------------------------------
	//  CS map for one Rank
	//------------------------------
	//param_pi_cs_map[3:0] <= `PI_SD int_regin[3:0];
	ddr_mmio_rd32(4 * (11 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
#ifdef X16_MODE
	rddata = modified_bits_by_value(rddata, 1, 3, 0);
#else
	rddata = modified_bits_by_value(rddata, 5, 3, 0);
#endif
	mmio_wr32(4 * (11 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	//------------------------------
	//  CA Training times
	//------------------------------
	//change from 2 to 1
	//param_pi_init_work_freq[4:0] <= `PI_SD int_regin[4:0];
	ddr_mmio_rd32(4 * (10 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = modified_bits_by_value(rddata, 2, 4, 0);
	mmio_wr32(4 * (10 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	//param_pi_freq_map[7:0] <= `PI_SD int_regin[7:0];
	ddr_mmio_rd32(4 * (9 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = modified_bits_by_value(rddata, 5, 4, 0);
	mmio_wr32(4 * (9 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	//------------------------------
	//  CALVL- vref initial start }
	//------------------------------
	//param_pi_calvl_vref_initial_start_point_f2[6:0] <= `PI_SD int_regin[6:0];
	//param_pi_calvl_vref_initial_stop_point_f2[6:0] <= `PI_SD int_regin[14:8];
	ddr_mmio_rd32(4 * (167 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	// rddata = modified_bits_by_value(rddata, 0x1a, 6, 0);
	// rddata = modified_bits_by_value(rddata, 0x1e, 14, 8);
	// rddata = modified_bits_by_value(rddata, 0x40, 6, 0);
	// rddata = modified_bits_by_value(rddata, 0x72, 14, 8);
	rddata = modified_bits_by_value(rddata, 0x6E, 6, 0);
	rddata = modified_bits_by_value(rddata, 0x72, 14, 8);
	mmio_wr32(4 * (167 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	//------------------------------
	//  CALVL- No CA Temination read (No MRR before CA training..)
	//------------------------------
	//param_pi_no_catr_read[0:0] <= `PI_SD int_regin[0:0]
	ddr_mmio_rd32(4 * (121 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = modified_bits_by_value(rddata, 1, 0, 0);
	mmio_wr32(4 * (121 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	//------------------------------
	//  CALVL- 1: cha/chb different cs @calvl
	//------------------------------
	//param_phyd_calvl_cs_diff       <= `PI_SD int_regin[24];
	ddr_mmio_rd32(4 * (166 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = modified_bits_by_value(rddata, 1, 24, 24);
	mmio_wr32(4 * (166 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	//------------------------------
	//  CALVL- Only CS0/CS2
	//------------------------------
	//param_pi_calvl_cs_map[3:0] <= `PI_SD int_regin[3:0];
	ddr_mmio_rd32(4 * (57 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
#ifdef X16_MODE
	rddata = modified_bits_by_value(rddata, 1, 3, 0);
#else
	rddata = modified_bits_by_value(rddata, 5, 3, 0);
#endif
	mmio_wr32(4 * (57 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	//------------------------------
	ddr_mmio_rd32(4 * (165 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = modified_bits_by_value(rddata, 0x2, 5, 0);
	rddata = modified_bits_by_value(rddata, 0x70, 14, 8);
	rddata = modified_bits_by_value(rddata, 0x4, 21, 16);
	rddata = modified_bits_by_value(rddata, 0x50, 30, 24);
	mmio_wr32(4 * (165 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	ddr_mmio_rd32(4 * (168 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = modified_bits_by_value(rddata, 0x3, 2, 0);
	rddata = modified_bits_by_value(rddata, 0x6, 11, 8);
	rddata = modified_bits_by_value(rddata, 0x2, 15, 12);
	mmio_wr32(4 * (168 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	ddr_mmio_rd32(4 * (166 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = modified_bits_by_value(rddata, 0x2a57f, 23, 0);
	mmio_wr32(4 * (166 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	ddr_mmio_rd32(4 * (167 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = modified_bits_by_value(rddata, 0xfd5a80, 23, 0);
	mmio_wr32(4 * (167 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	ddr_mmio_rd32(4 * (157 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = modified_bits_by_value(rddata, 0x0, 1, 0);
	rddata = modified_bits_by_value(rddata, 0x0, 17, 16);
	mmio_wr32(4 * (157 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
#ifdef LP4
	//param_pi_calvl_en_f2[1:0] <= `PI_SD int_regin[1:0];
	ddr_mmio_rd32(4 * (158 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = modified_bits_by_value(rddata, 0x1, 1, 0);
	mmio_wr32(4 * (158 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
#else
	//param_pi_calvl_en_f2[1:0] <= `PI_SD int_regin[1:0];
	ddr_mmio_rd32(4 * (158 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = modified_bits_by_value(rddata, 0x0, 1, 0);
	mmio_wr32(4 * (158 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
#endif
	ddr_mmio_rd32(4 * (10 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = modified_bits_by_value(rddata, 0x2, 4, 0);
	mmio_wr32(4 * (10 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	ddr_mmio_rd32(4 * (9 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = modified_bits_by_value(rddata, 0x5, 4, 0);
	mmio_wr32(4 * (9 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	ddr_mmio_rd32(4 * (141 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = modified_bits_by_value(rddata, 0x64, 25, 16);
	mmio_wr32(4 * (141 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	ddr_mmio_rd32(4 * (27 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
#ifdef X16_MODE
	rddata = modified_bits_by_value(rddata, 0x1, 3, 0);
#else
	rddata = modified_bits_by_value(rddata, 0x5, 3, 0);
#endif
	mmio_wr32(4 * (27 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	ddr_mmio_rd32(4 * (139 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = modified_bits_by_value(rddata, 0x0, 17, 16);
	rddata = modified_bits_by_value(rddata, 0x0, 25, 24);
	mmio_wr32(4 * (139 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	ddr_mmio_rd32(4 * (140 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = modified_bits_by_value(rddata, 0x0, 1, 0);
	mmio_wr32(4 * (140 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	ddr_mmio_rd32(4 * (146 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = modified_bits_by_value(rddata, 0x0, 17, 16);
	rddata = modified_bits_by_value(rddata, 0x0, 25, 24);
	mmio_wr32(4 * (146 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	ddr_mmio_rd32(4 * (147 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = modified_bits_by_value(rddata, 0x0, 1, 0);
	rddata = modified_bits_by_value(rddata, 0x0, 9, 8);
	rddata = modified_bits_by_value(rddata, 0x0, 17, 16);
	rddata = modified_bits_by_value(rddata, 0x0, 25, 24);
	mmio_wr32(4 * (147 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	ddr_mmio_rd32(4 * (46 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
#ifdef X16_MODE
	rddata = modified_bits_by_value(rddata, 0x1, 19, 16);
	rddata = modified_bits_by_value(rddata, 0x1, 27, 24);
#else
	rddata = modified_bits_by_value(rddata, 0x5, 19, 16);
	rddata = modified_bits_by_value(rddata, 0x5, 27, 24);
#endif
	mmio_wr32(4 * (46 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	ddr_mmio_rd32(4 * (47 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = modified_bits_by_value(rddata, 0x28, 9, 0);
	mmio_wr32(4 * (47 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	ddr_mmio_rd32(4 * (180 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = modified_bits_by_value(rddata, 0x0, 9, 8);
	mmio_wr32(4 * (180 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	ddr_mmio_rd32(4 * (183 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = modified_bits_by_value(rddata, 0x0, 9, 8);
	mmio_wr32(4 * (183 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	ddr_mmio_rd32(4 * (186 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = modified_bits_by_value(rddata, 0x0, 9, 8);
	mmio_wr32(4 * (186 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	ddr_mmio_rd32(4 * (76 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = modified_bits_by_value(rddata, 0x1, 16, 16);
	mmio_wr32(4 * (76 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	ddr_mmio_rd32(4 * (67 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
#ifdef X16_MODE
	rddata = modified_bits_by_value(rddata, 0x1, 11, 8);
#else
	rddata = modified_bits_by_value(rddata, 0x5, 11, 8);
#endif
	mmio_wr32(4 * (67 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	ddr_mmio_rd32(4 * (11 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
#ifdef X16_MODE
	rddata = modified_bits_by_value(rddata, 0x1, 3, 0);
#else
	rddata = modified_bits_by_value(rddata, 0x5, 3, 0);
#endif
	mmio_wr32(4 * (11 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	if (dram_class == 0xb) {
		//param_pi_wdqlvl_vref_initial_start_point_f2[6:0] <= `PI_SD int_regin[22:16];
		//param_pi_wdqlvl_vref_initial_stop_point_f2[6:0] <= `PI_SD int_regin[30:24];
		ddr_mmio_rd32(4 * (185 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
		//rddata[22:16] = 7'b0011010;
		//rddata[30:24] = 7'b0011011;
		rddata = modified_bits_by_value(rddata, 0x1a, 22, 16);
		rddata = modified_bits_by_value(rddata, 0x1e, 30, 24);
		mmio_wr32(4 * (185 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
		//param_pi_wdqlvl_vref_initial_stepsize[4:0] <= `PI_SD int_regin[20:16];
		//param_pi_wdqlvl_vref_normal_stepsize[4:0] <= `PI_SD int_regin[28:24];
		ddr_mmio_rd32(4 * (67 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
		//rddata[20:16] = 5'b00001;
		//rddata[28:24] = 5'b00001;
		rddata = modified_bits_by_value(rddata, 0x1, 20, 16);
		rddata = modified_bits_by_value(rddata, 0x1, 28, 24);
		mmio_wr32(4 * (67 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
		//param_pi_wdqlvl_vref_delta_f2[3:0] <= `PI_SD int_regin[3:0];
		//ddr_mmio_rd32(4 * (186 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
		//rddata=modified_bits_by_value(rddata, 0x1, 3, 0);
		//mmio_wr32(4 * (186 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
		//#endif
	}
	if (dram_class == 0xb) {
		//param_pi_wdqlvl_need_save_restore[1:0] <= `PI_SD int_regin[25:24];
		ddr_mmio_rd32(4 * (72 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
		rddata = modified_bits_by_value(rddata, 0x0, 25, 24);
		mmio_wr32(4 * (72 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
		//#else
	} else {
		//------------------------------
		//  wdqLVL- save/restore (lpddr3/ddr4)
		//------------------------------
		//param_pi_wdqlvl_need_save_restore[1:0] <= `PI_SD int_regin[25:24];
		ddr_mmio_rd32(4 * (72 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
		rddata = modified_bits_by_value(rddata, 0x3, 25, 24);
		mmio_wr32(4 * (72 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
		//param_pi_wdqlvl_dram_lvl_start_addr
		ddr_mmio_rd32(4 * (73 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
		rddata = modified_bits_by_value(rddata, 0x1, 31, 0);
		mmio_wr32(4 * (73 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
		ddr_mmio_rd32(4 * (74 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
		rddata = modified_bits_by_value(rddata, 0x0, 3, 0);
		mmio_wr32(4 * (74 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
		//#endif
	}
	ddr_mmio_rd32(4 * (66 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = modified_bits_by_value(rddata, 0x1, 8, 8);
	mmio_wr32(4 * (66 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	ddr_mmio_rd32(4 * (184 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = modified_bits_by_value(rddata, 0x34, 25, 16);
	mmio_wr32(4 * (184 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	ddr_mmio_rd32(4 * (185 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = modified_bits_by_value(rddata, 0x63, 9, 0);
	mmio_wr32(4 * (185 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	ddr_mmio_rd32(4 * (178 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = modified_bits_by_value(rddata, 0x0, 1, 0);
	mmio_wr32(4 * (178 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
}

void pi_wrlvl_req(void)
{
	rddata = mmio_rd32(cfg_base + 0x30);
	rddata = modified_bits_by_value(rddata, 0, 1, 1); //PWRCTL.powerdown_en
	rddata = modified_bits_by_value(rddata, 0, 0, 0); //PWRCTL.selfref_en
	mmio_wr32(cfg_base + 0x30, rddata);
	uartlog("pi_wrlvl_req\n");
	// ddr_debug_wr32(0x2e);
	// ddr_debug_num_write();
	rddata = mmio_rd32(4 * (24 + PI_BASE_ADDR) + CADENCE_PHYD);
	rddata = modified_bits_by_value(rddata, 1, 0, 0);
	mmio_wr32(4 * (24 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = mmio_rd32(4 * (83 + PI_BASE_ADDR) + CADENCE_PHYD);
	while (get_bits_from_value(rddata, 23, 23) == 0)
		rddata = mmio_rd32(4 * (83 + PI_BASE_ADDR) + CADENCE_PHYD);
	mmio_wr32(4 * (84 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	pi_wrlvl_status();
	// rddata=mmio_rd32(cfg_base + 0x30);
	// rddata=modified_bits_by_value(rddata, 1, 1, 1); ////PWRCTL.powerdown_en
	// rddata=modified_bits_by_value(rddata, 1, 0, 0); ////PWRCTL.selfref_en
	// mmio_wr32(cfg_base + 0x30, rddata);
}

void pi_rdlvl_gate_req(void)
{
	// disable mask track
	rddata = mmio_rd32(4*(194 + PHY_BASE_ADDR)+CADENCE_PHYD);
	rddata = modified_bits_by_value(rddata, 0xf, 19, 16);
	mmio_wr32(4*(194 + PHY_BASE_ADDR)+CADENCE_PHYD,  rddata);

	rddata = mmio_rd32(cfg_base + 0x30);
	rddata = modified_bits_by_value(rddata, 0, 0, 1); // PWRCTL.powerdown_en
	rddata = modified_bits_by_value(rddata, 0, 0, 0); // PWRCTL.selfref_en
	mmio_wr32(cfg_base + 0x30, rddata);
	uartlog("pi_rdlvl_gate_req\n");
	// ddr_debug_wr32(0x2f);
	// ddr_debug_num_write();
	rddata = mmio_rd32(4*(33 + PI_BASE_ADDR)+CADENCE_PHYD);
	rddata = modified_bits_by_value(rddata, 1, 24, 24);
	mmio_wr32(4*(33 + PI_BASE_ADDR)+CADENCE_PHYD,  rddata);
	rddata = mmio_rd32(4*(83 + PI_BASE_ADDR)+CADENCE_PHYD);
	while (get_bits_from_value(rddata, 21, 21) == 0)
		rddata = mmio_rd32(4*(83 + PI_BASE_ADDR)+CADENCE_PHYD);
	mmio_wr32(4*(84 + PI_BASE_ADDR)+CADENCE_PHYD,  rddata);
	pi_rdlvl_gate_status();
	// ddata = mmio_rd32(cfg_base + 0x30);
	// ddata = modified_bits_by_value(rddata, 0, 0, 1);  // PWRCTL.powerdown_en
	// ddata = modified_bits_by_value(rddata, 0, 0, 0);  // PWRCTL.selfref_en
	// mio_wr32(cfg_base + 0x30, rddata);

	// enable mask track
	rddata = mmio_rd32(4*(194 + PHY_BASE_ADDR)+CADENCE_PHYD);
	rddata = modified_bits_by_value(rddata, 0x0, 19, 16);
	mmio_wr32(4*(194 + PHY_BASE_ADDR)+CADENCE_PHYD,  rddata);
}

void pi_rdlvl_req(void)
{
	rddata = mmio_rd32(cfg_base + 0x30);
	rddata = modified_bits_by_value(rddata, 0, 1, 1); // PWRCTL.powerdown_en
	rddata = modified_bits_by_value(rddata, 0, 0, 0); // PWRCTL.selfref_en
	mmio_wr32(cfg_base + 0x30, rddata);

	uartlog("-30- pi_rdlvl_req\n");
	uartlog("pi_rdlvl_req\n");

	rddata = mmio_rd32(4 * (33 + PI_BASE_ADDR) + CADENCE_PHYD);
	rddata = modified_bits_by_value(rddata, 1, 16, 16);
	mmio_wr32(4 * (33 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);

	uartlog("wait retraining finish\n");
	rddata = mmio_rd32(4 * (83 + PI_BASE_ADDR) + CADENCE_PHYD);
	uartlog("Read param_pi_int_status %x\n", rddata);

	while (get_bits_from_value(rddata, 22, 22) == 0) {
		rddata = mmio_rd32(4 * (83 + PI_BASE_ADDR) + CADENCE_PHYD);
		uartlog("Read param_pi_int_status %x\n", rddata);
	}

	uartlog("Ack retraining\n");
	mmio_wr32(4 * (84 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	uartlog("write param_pi_int_ack %x\n", rddata);

	pi_rdlvl_status();

	rddata = mmio_rd32(cfg_base + 0x30);
	rddata = modified_bits_by_value(rddata, 0, 1, 1); // PWRCTL.powerdown_en
	rddata = modified_bits_by_value(rddata, 0, 0, 0); // PWRCTL.selfref_en
	mmio_wr32(cfg_base + 0x30, rddata);
}

void pi_wdqlvl_req(void)
{
	rddata = mmio_rd32(cfg_base + 0x30);
	rddata = modified_bits_by_value(rddata, 0, 1, 1); // PWRCTL.powerdown_en
	rddata = modified_bits_by_value(rddata, 0, 0, 0); // PWRCTL.selfref_en
	mmio_wr32(cfg_base + 0x30, rddata);
	uartlog("pi_wdqlvl_req\n");
	// ddr_debug_wr32(0x31);
	// ddr_debug_num_write();
	rddata = mmio_rd32(4 * (68 + PI_BASE_ADDR) + CADENCE_PHYD);
	rddata = modified_bits_by_value(rddata, 1, 8, 8);
	mmio_wr32(4 * (68 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = mmio_rd32(4 * (83 + PI_BASE_ADDR) + CADENCE_PHYD);
	while (get_bits_from_value(rddata, 25, 25) == 0)
		rddata = mmio_rd32(4 * (83 + PI_BASE_ADDR) + CADENCE_PHYD);
	mmio_wr32(4 * (84 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	pi_wdqlvl_status();
	// rddata=mmio_rd32(cfg_base + 0x30);
	// rddata=modified_bits_by_value(rddata, 1, 1, 1);  // PWRCTL.powerdown_en
	// rddata=modified_bits_by_value(rddata, 1, 0, 0);  // PWRCTL.selfref_en
	// mmio_wr32(cfg_base + 0x30, rddata);
}

void pi_wrlvl_status(void)
{
	uartlog("pi_wrlvl_status\n");
#if 1
	// ddr_debug_wr32(0x32);
	// ddr_debug_num_write();
	rddata = mmio_rd32(4 * (1089 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte0_hard0=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("byte0_hard1=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1090 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte1_hard0=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("byte1_hard1=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1091 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte2_hard0=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("byte2_hard1=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1092 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte3_hard0=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("byte3_hard1=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1093 + PHY_BASE_ADDR) + CADENCE_PHYD);
	rddata = mmio_rd32(4 * (1094 + PHY_BASE_ADDR) + CADENCE_PHYD);

	rddata = mmio_rd32(4 * (116 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte0 tx dqs shift=%x\n", get_bits_from_value(rddata, 11, 6));
	rddata = mmio_rd32(4 * (119 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte1 tx dqs shift=%x\n", get_bits_from_value(rddata, 11, 6));
	rddata = mmio_rd32(4 * (122 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte2 tx dqs shift=%x\n", get_bits_from_value(rddata, 11, 6));
	rddata = mmio_rd32(4 * (125 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte3 tx dqs shift=%x\n", get_bits_from_value(rddata, 11, 6));

	rddata = mmio_rd32(4 * (155 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte0 tx dqs delay code=%x\n", get_bits_from_value(rddata, 31, 16));
	rddata = mmio_rd32(4 * (158 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte1 tx dqs delay code=%x\n", get_bits_from_value(rddata, 31, 16));
	rddata = mmio_rd32(4 * (161 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte2 tx dqs delay code=%x\n", get_bits_from_value(rddata, 31, 16));
	rddata = mmio_rd32(4 * (164 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte3 tx dqs delay code=%x\n", get_bits_from_value(rddata, 31, 16));
#endif
}

void pi_rdlvl_gate_status(void)
{
	uartlog("pi_rdlvl_gate_status\n");
#if 1
	// ddr_debug_wr32(0x33);
	// ddr_debug_num_write();
	rddata = mmio_rd32(4 * (1097 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte0_hard0=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("byte0_hard1=%x\n", get_bits_from_value(rddata, 28, 16));

	rddata = mmio_rd32(4 * (1098 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte1_hard0=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("byte1_hard1=%x\n", get_bits_from_value(rddata, 28, 16));

	rddata = mmio_rd32(4 * (1099 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte2_hard0=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("byte2_hard1=%x\n", get_bits_from_value(rddata, 28, 16));

	rddata = mmio_rd32(4 * (1100 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte3_hard0=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("byte3_hard1=%x\n", get_bits_from_value(rddata, 28, 16));

	rddata = mmio_rd32(4 * (1101 + PHY_BASE_ADDR) + CADENCE_PHYD);
	rddata = mmio_rd32(4 * (1102 + PHY_BASE_ADDR) + CADENCE_PHYD); // ##2194

	// `test_stream = "byte0 mask shift";
	// ##2194
	rddata = mmio_rd32(4 * (101 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte0 mask shift=%x\n", get_bits_from_value(rddata, 21, 16));
	rddata = mmio_rd32(4 * (102 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte1 mask shift=%x\n", get_bits_from_value(rddata, 21, 16));
	rddata = mmio_rd32(4 * (103 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte2 mask shift=%x\n", get_bits_from_value(rddata, 21, 16));
	rddata = mmio_rd32(4 * (104 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte3 mask shift=%x\n", get_bits_from_value(rddata, 21, 16));
	// `test_stream = "mask delay";
	// ##22B8

	rddata = mmio_rd32(4 * (174 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte10 mask delay code=%x\n", rddata);
	rddata = mmio_rd32(4 * (175 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte32 mask delay code=%x\n", rddata);
#endif
}

void pi_rdlvl_status(void)
{
	uartlog("-34- pi_rdlvl_status\n");
#if 1
	rddata = mmio_rd32(4 * (1110 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte0_dq0_rise_le=%x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("pirdlvl_byte0_dq1_rise_le=%x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("pirdlvl_byte0_dq2_rise_le=%x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("pirdlvl_byte0_dq3_rise_le=%x\n", get_bits_from_value(rddata, 31, 24));
	rddata = mmio_rd32(4 * (1111 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte0_dq4_rise_le=%x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("pirdlvl_byte0_dq5_rise_le=%x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("pirdlvl_byte0_dq6_rise_le=%x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("pirdlvl_byte0_dq7_rise_le=%x\n", get_bits_from_value(rddata, 31, 24));
	rddata = mmio_rd32(4 * (1112 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte0_dq8_rise_le=%x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("pirdlvl_byte0_dq0_rise_te=%x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("pirdlvl_byte0_dq1_rise_te=%x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("pirdlvl_byte0_dq2_rise_te=%x\n", get_bits_from_value(rddata, 31, 24));
	rddata = mmio_rd32(4 * (1113 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte0_dq3_rise_te=%x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("pirdlvl_byte0_dq4_rise_te=%x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("pirdlvl_byte0_dq5_rise_te=%x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("pirdlvl_byte0_dq6_rise_te=%x\n", get_bits_from_value(rddata, 31, 24));
	rddata = mmio_rd32(4 * (1114 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte0_dq7_rise_te=%x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("pirdlvl_byte0_dq8_rise_te=%x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("pirdlvl_byte0_dq0_fall_le=%x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("pirdlvl_byte0_dq1_fall_le=%x\n", get_bits_from_value(rddata, 31, 24));
	rddata = mmio_rd32(4 * (1115 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte0_dq2_fall_le=%x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("pirdlvl_byte0_dq3_fall_le=%x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("pirdlvl_byte0_dq4_fall_le=%x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("pirdlvl_byte0_dq5_fall_le=%x\n", get_bits_from_value(rddata, 31, 24));
	rddata = mmio_rd32(4 * (1116 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte0_dq6_fall_le=%x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("pirdlvl_byte0_dq7_fall_le=%x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("pirdlvl_byte0_dq8_fall_le=%x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("pirdlvl_byte0_dq0_fall_te=%x\n", get_bits_from_value(rddata, 31, 24));
	rddata = mmio_rd32(4 * (1117 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte0_dq1_fall_te=%x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("pirdlvl_byte0_dq2_fall_te=%x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("pirdlvl_byte0_dq3_fall_te=%x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("pirdlvl_byte0_dq4_fall_te=%x\n", get_bits_from_value(rddata, 31, 24));
	rddata = mmio_rd32(4 * (1118 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte0_dq5_fall_te=%x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("pirdlvl_byte0_dq6_fall_te=%x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("pirdlvl_byte0_dq7_fall_te=%x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("pirdlvl_byte0_dq8_fall_te=%x\n", get_bits_from_value(rddata, 31, 24));
	rddata = mmio_rd32(4 * (1119 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte0_status0=%x\n", rddata);
	rddata = mmio_rd32(4 * (1120 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte0_status1=%x\n", rddata);

	rddata = mmio_rd32(4 * (1121 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte1_dq0_rise_le=%x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("pirdlvl_byte1_dq1_rise_le=%x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("pirdlvl_byte1_dq2_rise_le=%x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("pirdlvl_byte1_dq3_rise_le=%x\n", get_bits_from_value(rddata, 31, 24));
	rddata = mmio_rd32(4 * (1122 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte1_dq4_rise_le=%x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("pirdlvl_byte1_dq5_rise_le=%x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("pirdlvl_byte1_dq6_rise_le=%x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("pirdlvl_byte1_dq7_rise_le=%x\n", get_bits_from_value(rddata, 31, 24));
	rddata = mmio_rd32(4 * (1123 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte1_dq8_rise_le=%x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("pirdlvl_byte1_dq0_rise_te=%x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("pirdlvl_byte1_dq1_rise_te=%x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("pirdlvl_byte1_dq2_rise_te=%x\n", get_bits_from_value(rddata, 31, 24));
	rddata = mmio_rd32(4 * (1124 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte1_dq3_rise_te=%x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("pirdlvl_byte1_dq4_rise_te=%x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("pirdlvl_byte1_dq5_rise_te=%x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("pirdlvl_byte1_dq6_rise_te=%x\n", get_bits_from_value(rddata, 31, 24));
	rddata = mmio_rd32(4 * (1125 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte1_dq7_rise_te=%x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("pirdlvl_byte1_dq8_rise_te=%x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("pirdlvl_byte1_dq0_fall_le=%x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("pirdlvl_byte1_dq1_fall_le=%x\n", get_bits_from_value(rddata, 31, 24));
	rddata = mmio_rd32(4 * (1126 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte1_dq2_fall_le=%x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("pirdlvl_byte1_dq3_fall_le=%x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("pirdlvl_byte1_dq4_fall_le=%x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("pirdlvl_byte1_dq5_fall_le=%x\n", get_bits_from_value(rddata, 31, 24));
	rddata = mmio_rd32(4 * (1127 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte1_dq6_fall_le=%x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("pirdlvl_byte1_dq7_fall_le=%x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("pirdlvl_byte1_dq8_fall_le=%x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("pirdlvl_byte1_dq0_fall_te=%x\n", get_bits_from_value(rddata, 31, 24));
	rddata = mmio_rd32(4 * (1128 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte1_dq1_fall_te=%x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("pirdlvl_byte1_dq2_fall_te=%x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("pirdlvl_byte1_dq3_fall_te=%x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("pirdlvl_byte1_dq4_fall_te=%x\n", get_bits_from_value(rddata, 31, 24));
	rddata = mmio_rd32(4 * (1129 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte1_dq5_fall_te=%x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("pirdlvl_byte1_dq6_fall_te=%x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("pirdlvl_byte1_dq7_fall_te=%x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("pirdlvl_byte1_dq8_fall_te=%x\n", get_bits_from_value(rddata, 31, 24));
	rddata = mmio_rd32(4 * (1130 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte1_status0=%x\n", rddata);
	rddata = mmio_rd32(4 * (1131 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte1_status1=%x\n", rddata);

	rddata = mmio_rd32(4 * (1132 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte2_dq0_rise_le=%x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("pirdlvl_byte2_dq1_rise_le=%x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("pirdlvl_byte2_dq2_rise_le=%x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("pirdlvl_byte2_dq3_rise_le=%x\n", get_bits_from_value(rddata, 31, 24));
	rddata = mmio_rd32(4 * (1133 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte2_dq4_rise_le=%x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("pirdlvl_byte2_dq5_rise_le=%x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("pirdlvl_byte2_dq6_rise_le=%x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("pirdlvl_byte2_dq7_rise_le=%x\n", get_bits_from_value(rddata, 31, 24));
	rddata = mmio_rd32(4 * (1134 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte2_dq8_rise_le=%x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("pirdlvl_byte2_dq0_rise_te=%x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("pirdlvl_byte2_dq1_rise_te=%x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("pirdlvl_byte2_dq2_rise_te=%x\n", get_bits_from_value(rddata, 31, 24));
	rddata = mmio_rd32(4 * (1135 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte2_dq3_rise_te=%x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("pirdlvl_byte2_dq4_rise_te=%x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("pirdlvl_byte2_dq5_rise_te=%x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("pirdlvl_byte2_dq6_rise_te=%x\n", get_bits_from_value(rddata, 31, 24));
	rddata = mmio_rd32(4 * (1136 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte2_dq7_rise_te=%x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("pirdlvl_byte2_dq8_rise_te=%x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("pirdlvl_byte2_dq0_fall_le=%x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("pirdlvl_byte2_dq1_fall_le=%x\n", get_bits_from_value(rddata, 31, 24));
	rddata = mmio_rd32(4 * (1137 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte2_dq2_fall_le=%x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("pirdlvl_byte2_dq3_fall_le=%x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("pirdlvl_byte2_dq4_fall_le=%x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("pirdlvl_byte2_dq5_fall_le=%x\n", get_bits_from_value(rddata, 31, 24));
	rddata = mmio_rd32(4 * (1138 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte2_dq6_fall_le=%x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("pirdlvl_byte2_dq7_fall_le=%x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("pirdlvl_byte2_dq8_fall_le=%x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("pirdlvl_byte2_dq0_fall_te=%x\n", get_bits_from_value(rddata, 31, 24));
	rddata = mmio_rd32(4 * (1139 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte2_dq1_fall_te=%x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("pirdlvl_byte2_dq2_fall_te=%x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("pirdlvl_byte2_dq3_fall_te=%x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("pirdlvl_byte2_dq4_fall_te=%x\n", get_bits_from_value(rddata, 31, 24));
	rddata = mmio_rd32(4 * (1140 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte2_dq5_fall_te=%x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("pirdlvl_byte2_dq6_fall_te=%x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("pirdlvl_byte2_dq7_fall_te=%x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("pirdlvl_byte2_dq8_fall_te=%x\n", get_bits_from_value(rddata, 31, 24));
	rddata = mmio_rd32(4 * (1141 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte2_status0=%x\n", rddata);
	rddata = mmio_rd32(4 * (1142 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte2_status1=%x\n", rddata);

	rddata = mmio_rd32(4 * (1143 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte3_dq0_rise_le=%x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("pirdlvl_byte3_dq1_rise_le=%x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("pirdlvl_byte3_dq2_rise_le=%x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("pirdlvl_byte3_dq3_rise_le=%x\n", get_bits_from_value(rddata, 31, 24));
	rddata = mmio_rd32(4 * (1144 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte3_dq4_rise_le=%x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("pirdlvl_byte3_dq5_rise_le=%x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("pirdlvl_byte3_dq6_rise_le=%x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("pirdlvl_byte3_dq7_rise_le=%x\n", get_bits_from_value(rddata, 31, 24));
	rddata = mmio_rd32(4 * (1145 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte3_dq8_rise_le=%x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("pirdlvl_byte3_dq0_rise_te=%x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("pirdlvl_byte3_dq1_rise_te=%x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("pirdlvl_byte3_dq2_rise_te=%x\n", get_bits_from_value(rddata, 31, 24));
	rddata = mmio_rd32(4 * (1146 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte3_dq3_rise_te=%x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("pirdlvl_byte3_dq4_rise_te=%x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("pirdlvl_byte3_dq5_rise_te=%x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("pirdlvl_byte3_dq6_rise_te=%x\n", get_bits_from_value(rddata, 31, 24));
	rddata = mmio_rd32(4 * (1147 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte3_dq7_rise_te=%x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("pirdlvl_byte3_dq8_rise_te=%x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("pirdlvl_byte3_dq0_fall_le=%x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("pirdlvl_byte3_dq1_fall_le=%x\n", get_bits_from_value(rddata, 31, 24));
	rddata = mmio_rd32(4 * (1148 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte3_dq2_fall_le=%x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("pirdlvl_byte3_dq3_fall_le=%x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("pirdlvl_byte3_dq4_fall_le=%x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("pirdlvl_byte3_dq5_fall_le=%x\n", get_bits_from_value(rddata, 31, 24));
	rddata = mmio_rd32(4 * (1149 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte3_dq6_fall_le=%x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("pirdlvl_byte3_dq7_fall_le=%x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("pirdlvl_byte3_dq8_fall_le=%x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("pirdlvl_byte3_dq0_fall_te=%x\n", get_bits_from_value(rddata, 31, 24));
	rddata = mmio_rd32(4 * (1150 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte3_dq1_fall_te=%x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("pirdlvl_byte3_dq2_fall_te=%x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("pirdlvl_byte3_dq3_fall_te=%x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("pirdlvl_byte3_dq4_fall_te=%x\n", get_bits_from_value(rddata, 31, 24));
	rddata = mmio_rd32(4 * (1151 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte3_dq5_fall_te=%x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("pirdlvl_byte3_dq6_fall_te=%x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("pirdlvl_byte3_dq7_fall_te=%x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("pirdlvl_byte3_dq8_fall_te=%x\n", get_bits_from_value(rddata, 31, 24));
	rddata = mmio_rd32(4 * (1152 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte3_status0=%x\n", rddata);
	rddata = mmio_rd32(4 * (1153 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("pirdlvl_byte3_status1=%x\n", rddata);

	// deskew
	rddata = mmio_rd32(4 * (202 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte0_deskewdq3210=%x\n", rddata);
	rddata = mmio_rd32(4 * (203 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte0_deskewdq7654=%x\n", rddata);
	rddata = mmio_rd32(4 * (204 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte0_deskewdq8=%x\n", rddata);

	rddata = mmio_rd32(4 * (205 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte1_deskewdq3210=%x\n", rddata);
	rddata = mmio_rd32(4 * (206 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte1_deskewdq7654=%x\n", rddata);
	rddata = mmio_rd32(4 * (207 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte1_deskewdq8=%x\n", rddata);

	rddata = mmio_rd32(4 * (208 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte2_deskewdq3210=%x\n", rddata);
	rddata = mmio_rd32(4 * (209 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte2_deskewdq7654=%x\n", rddata);
	rddata = mmio_rd32(4 * (210 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte2_deskewdq8=%x\n", rddata);

	rddata = mmio_rd32(4 * (211 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte3_deskewdq3210=%x\n", rddata);
	rddata = mmio_rd32(4 * (212 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte3_deskewdq7654=%x\n", rddata);
	rddata = mmio_rd32(4 * (213 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte3_deskewdq8=%x\n", rddata);

	// rdq delay code
	rddata = mmio_rd32(4 * (170 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte0_rdqspos/neg=%x\n", rddata);
	rddata = mmio_rd32(4 * (171 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte0_rdqspos/neg=%x\n", rddata);
	rddata = mmio_rd32(4 * (172 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte0_rdqspos/neg=%x\n", rddata);
	rddata = mmio_rd32(4 * (173 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte0_rdqspos/neg=%x\n", rddata);
#endif
}

void pi_wdqlvl_status(void)
{
	uartlog("-35- pi_wdqlvl_status\n");

#if 1
	rddata = mmio_rd32(4 * (1154 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte0_dq0_le=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("piwdqlvl_byte0_dq1_le=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1155 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte0_dq2_le=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("piwdqlvl_byte0_dq3_le=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1156 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte0_dq4_le=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("piwdqlvl_byte0_dq5_le=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1157 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte0_dq6_le=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("piwdqlvl_byte0_dq7_le=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1158 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte0_dq8_le=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("piwdqlvl_byte0_dq0_te=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1159 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte0_dq1_te=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("piwdqlvl_byte0_dq2_te=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1160 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte0_dq3_te=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("piwdqlvl_byte0_dq4_te=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1161 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte0_dq5_te=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("piwdqlvl_byte0_dq6_te=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1162 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte0_dq7_te=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("piwdqlvl_byte0_dq8_te=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1163 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte0_status0=%x\n", rddata);
	rddata = mmio_rd32(4 * (1164 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte0_status1=%x\n", rddata);

	rddata = mmio_rd32(4 * (1165 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte1_dq0_le=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("piwdqlvl_byte1_dq1_le=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1166 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte1_dq2_le=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("piwdqlvl_byte1_dq3_le=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1167 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte1_dq4_le=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("piwdqlvl_byte1_dq5_le=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1168 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte1_dq6_le=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("piwdqlvl_byte1_dq7_le=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1169 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte1_dq8_le=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("piwdqlvl_byte1_dq0_te=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1170 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte1_dq1_te=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("piwdqlvl_byte1_dq2_te=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1171 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte1_dq3_te=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("piwdqlvl_byte1_dq4_te=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1172 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte1_dq5_te=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("piwdqlvl_byte1_dq6_te=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1173 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte1_dq7_te=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("piwdqlvl_byte1_dq8_te=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1174 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte1_status0=%x\n", rddata);
	rddata = mmio_rd32(4 * (1175 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte1_status1=%x\n", rddata);

	rddata = mmio_rd32(4 * (1176 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte2_dq0_le=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("piwdqlvl_byte2_dq1_le=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1177 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte2_dq2_le=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("piwdqlvl_byte2_dq3_le=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1178 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte2_dq4_le=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("piwdqlvl_byte2_dq5_le=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1179 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte2_dq6_le=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("piwdqlvl_byte2_dq7_le=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1180 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte2_dq8_le=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("piwdqlvl_byte2_dq0_te=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1181 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte2_dq1_te=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("piwdqlvl_byte2_dq2_te=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1182 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte2_dq3_te=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("piwdqlvl_byte2_dq4_te=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1183 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte2_dq5_te=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("piwdqlvl_byte2_dq6_te=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1184 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte2_dq7_te=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("piwdqlvl_byte2_dq8_te=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1185 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte2_status0=%x\n", rddata);
	rddata = mmio_rd32(4 * (1186 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte2_status1=%x\n", rddata);

	rddata = mmio_rd32(4 * (1189 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte3_dq0_le=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("piwdqlvl_byte3_dq1_le=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1190 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte3_dq2_le=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("piwdqlvl_byte3_dq3_le=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1191 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte3_dq4_le=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("piwdqlvl_byte3_dq5_le=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1192 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte3_dq6_le=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("piwdqlvl_byte3_dq7_le=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1193 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte3_dq8_le=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("piwdqlvl_byte3_dq0_te=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1194 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte3_dq1_te=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("piwdqlvl_byte3_dq2_te=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1195 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte3_dq3_te=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("piwdqlvl_byte3_dq4_te=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1196 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte3_dq5_te=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("piwdqlvl_byte3_dq6_te=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1197 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte3_dq7_te=%x\n", get_bits_from_value(rddata, 12, 0));
	uartlog("piwdqlvl_byte3_dq8_te=%x\n", get_bits_from_value(rddata, 28, 16));
	rddata = mmio_rd32(4 * (1198 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte3_status0=%x\n", rddata);
	rddata = mmio_rd32(4 * (1199 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("piwdqlvl_byte3_status1=%x\n", rddata);

	// wdq shift
	rddata = mmio_rd32(4 * (114 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte0_wdq3210_shift=%x\n", rddata);
	rddata = mmio_rd32(4 * (115 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte0_wdq7654_shift=%x\n", rddata);
	rddata = mmio_rd32(4 * (116 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte0_wdq8_shift=%x\n", rddata);

	rddata = mmio_rd32(4 * (117 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte1_wdq3210_shift=%x\n", rddata);
	rddata = mmio_rd32(4 * (118 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte1_wdq7654_shift=%x\n", rddata);
	rddata = mmio_rd32(4 * (119 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte1_wdq8_shift=%x\n", rddata);

	rddata = mmio_rd32(4 * (120 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte2_wdq3210_shift=%x\n", rddata);
	rddata = mmio_rd32(4 * (121 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte2_wdq7654_shift=%x\n", rddata);
	rddata = mmio_rd32(4 * (122 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte2_wdq8_shift=%x\n", rddata);

	rddata = mmio_rd32(4 * (123 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte3_wdq3210_shift=%x\n", rddata);
	rddata = mmio_rd32(4 * (124 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte3_wdq7654_shift=%x\n", rddata);
	rddata = mmio_rd32(4 * (125 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte3_wdq8_shift=%x\n", rddata);

	// wdq delay code
	rddata = mmio_rd32(4 * (153 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte0_wdq3210_code=%x\n", rddata);
	rddata = mmio_rd32(4 * (154 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte0_wdq7654_code=%x\n", rddata);
	rddata = mmio_rd32(4 * (155 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte0_wdqs/wdq8_code=%x\n", rddata);

	rddata = mmio_rd32(4 * (156 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte1_wdq3210_code=%x\n", rddata);
	rddata = mmio_rd32(4 * (157 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte1_wdq7654_code=%x\n", rddata);
	rddata = mmio_rd32(4 * (158 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte1_wdqs/wdq8_code=%x\n", rddata);
	rddata = mmio_rd32(4 * (159 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte2_wdq3210_code=%x\n", rddata);
	rddata = mmio_rd32(4 * (160 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte2_wdq7654_code=%x\n", rddata);
	rddata = mmio_rd32(4 * (161 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte2_wdqs/wdq8_code=%x\n", rddata);
	rddata = mmio_rd32(4 * (162 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte3_wdq3210_code=%x\n", rddata);
	rddata = mmio_rd32(4 * (163 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte3_wdq7654_code=%x\n", rddata);
	rddata = mmio_rd32(4 * (164 + PHY_BASE_ADDR) + CADENCE_PHYD);
	uartlog("byte3_wdqs/wdq8_code=%x\n", rddata);
#endif
}

void dll_cal_status(void)
{
	// uint32_t err_cnt;
	// uint32_t rx_dll_code;
	// uint32_t tx_dll_code;

	uartlog("dll_cal_status\n");
	ddr_mmio_rd32(4 * (1063 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	uartlog("param_phyd_to_reg_rx_dll_code0= %x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("param_phyd_to_reg_rx_dll_code1= %x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("param_phyd_to_reg_rx_dll_code2= %x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("param_phyd_to_reg_rx_dll_code3= %x\n", get_bits_from_value(rddata, 31, 24));
	ddr_mmio_rd32(4 * (1064 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	uartlog("param_phyd_to_reg_rx_dll_max= %x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("param_phyd_to_reg_rx_dll_min= %x\n", get_bits_from_value(rddata, 15, 8));
	ddr_mmio_rd32(4 * (1065 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	uartlog("param_phyd_to_reg_tx_dll_code0= %x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("param_phyd_to_reg_tx_dll_code1= %x\n", get_bits_from_value(rddata, 15, 8));
	uartlog("param_phyd_to_reg_tx_dll_code2= %x\n", get_bits_from_value(rddata, 23, 16));
	uartlog("param_phyd_to_reg_tx_dll_code3= %x\n", get_bits_from_value(rddata, 31, 24));
	ddr_mmio_rd32(4 * (1066 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	uartlog("param_phyd_to_reg_tx_dll_max= %x\n", get_bits_from_value(rddata, 7, 0));
	uartlog("param_phyd_to_reg_tx_dll_min= %x\n", get_bits_from_value(rddata, 15, 8));
#if 0
	err_cnt = 0;
	ddr_mmio_rd32(4 * (1062 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	rx_dll_code = get_bits_from_value(rddata, 8, 1);
	tx_dll_code = get_bits_from_value(rddata, 17, 10);
#ifdef _mem_freq_2133
	if (!((rx_dll_code > 0x26) && (rx_dll_code < 0x2b))) {
		uartlog("Info!! rx_dll_code dly_sel result fail, not 0x26~0x2b --%x--\n", rx_dll_code);
		err_cnt = err_cnt + 1;
	}
	if (!((tx_dll_code > 0x26) && (tx_dll_code < 0x2b))) {
		uartlog("Info!! tx_dll_code dly_sel result fail, not 0x26~0x2b --%x--\n", tx_dll_code);
		err_cnt = err_cnt + 1;
	}
#endif
#ifdef _mem_freq_3200
	if (!((rx_dll_code > 0x15) && (rx_dll_code < 0x22))) {
		uartlog("Info!! rx_dll_code dly_sel result fail, not 0x15~0x22 --%x--\n", rx_dll_code);
		err_cnt = err_cnt + 1;
	}
	if (!((tx_dll_code > 0x15) && (tx_dll_code < 0x22))) {
		uartlog("Info!! tx_dll_code dly_sel result fail, not 0x15~0x22 --%x--\n", tx_dll_code);
		err_cnt = err_cnt + 1;
	}
#endif
#ifdef _mem_freq_1866
	if (!((rx_dll_code > 0x2b) && (rx_dll_code < 0x30))) {
		uartlog("Info!! rx_dll_code dly_sel result fail, not 0x2b~0x30 --%x--\n", rx_dll_code);
		err_cnt = err_cnt + 1;
	}
	if (!((tx_dll_code > 0x2b) && (tx_dll_code < 0x30))) {
		uartlog("Info!! tx_dll_code dly_sel result fail, not 0x2b~0x30 --%x--\n", tx_dll_code);
		err_cnt = err_cnt + 1;
	}
#endif
#ifdef _mem_freq_2400
	if (!((rx_dll_code > 0x21) && (rx_dll_code < 0x25))) {
		uartlog("Info!! rx_dll_code dly_sel result fail, not 0x21~0x25 --%x--\n", rx_dll_code);
		err_cnt = err_cnt + 1;
	}
	if (!((tx_dll_code > 0x21) && (tx_dll_code < 0x25))) {
		uartlog("Info!! tx_dll_code dly_sel result fail, not 0x21~0x25 --%x--\n", tx_dll_code);
		err_cnt = err_cnt + 1;
	}
#endif
#ifdef _mem_freq_2666
	if (!((rx_dll_code > 0x1e) && (rx_dll_code < 0x22))) {
		uartlog("Info!! rx_dll_code dly_sel result fail, not 0x1e~0x22 --%x--\n", rx_dll_code);
		err_cnt = err_cnt + 1;
	}
	if (!((tx_dll_code > 0x1e) && (tx_dll_code < 0x22))) {
		uartlog("Info!! tx_dll_code dly_sel result fail, not 0x1e~0x22 --%x--\n", tx_dll_code);
		err_cnt = err_cnt + 1;
	}
#endif
	//***************************************************************
	//      FINISHED!
	//***************************************************************
	if (err_cnt != 0x0) {
		// uartlog("*****************************************\n");
		uartlog("DLL_CAL ERROR!!! err_cnt=%x\n", err_cnt);
		// uartlog("*****************************************\n");
	} else {
		// uartlog("*****************************************\n");
		uartlog("DLL_CAL PASS!!!\n");
		// uartlog("*****************************************\n");
	}
#endif
}

void zqcal_status(void)
{
	uint32_t zq_drvn;
	uint32_t zq_drvp;

	uartlog("zqcal_status\n");
	ddr_mmio_rd32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	zq_drvn = get_bits_from_value(rddata, 12, 8);
	zq_drvp = get_bits_from_value(rddata, 20, 16);
	if ((zq_drvn >= 0x0f) && (zq_drvn <= 0x11))
		uartlog("ZQ Complete ... param_phya_reg_tx_zq_drvn=%x\n", zq_drvn);
	else
		uartlog("ZQ Complete ... ERROR! param_phya_reg_tx_zq_drvn != 5'b10000 +- 1, read:value: %x\n", zq_drvn);

	if ((zq_drvp >= 0x0f) && (zq_drvp <= 0x11))
		uartlog("ZQ Complete ... param_phya_reg_tx_zq_drvp=%x\n", zq_drvp);
	else
		uartlog("ZQ Complete ... ERROR! param_phya_reg_tx_zq_drvp != 5'b10000 +- 1, read value: %x\n", zq_drvp);
}

void setting_check(void)
{
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wunused-but-set-variable" //for ignore phy_reg_version warning when no uartlog
	uint32_t pi_rdlat;
	uint32_t pi_wrlat;
	uint32_t synp_rdlat;
	uint32_t synp_wrlat;
	uint32_t phy_reg_version;
	uint32_t synp_wrlat_odd;
	uint32_t synp_rdlat_odd;

	uartlog("setting_check\n");
	ddr_mmio_rd32(4 * (131 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	pi_wrlat = get_bits_from_value(rddata, 22, 16);
	ddr_mmio_rd32(4 * (132 + PI_BASE_ADDR) + CADENCE_PHYD, rddata);
	pi_rdlat = get_bits_from_value(rddata, 22, 16) / 2;
	ddr_mmio_rd32(4 * (1058 + PHY_BASE_ADDR) + CADENCE_PHYD, phy_reg_version);
	ddr_mmio_rd32(cfg_base + 0x108, rddata);
	synp_wrlat = get_bits_from_value(rddata, 29, 24) * 2;
	synp_rdlat = get_bits_from_value(rddata, 21, 16) * 2;
	synp_wrlat_odd = get_bits_from_value(rddata, 24, 24);
	synp_rdlat_odd = get_bits_from_value(rddata, 16, 16);
	uartlog("phy_reg_version=%x, pi_wrlat=%x, pi_rdlat=%x, synp_wrlat=%x, synp_rdlat=%x\n", phy_reg_version, pi_wrlat, pi_rdlat, synp_wrlat, synp_rdlat);
	if ((pi_wrlat + synp_wrlat_odd) == synp_wrlat)
		uartlog("wrlat equal\n");
	else
		uartlog("wrlat Info !!! pi_wrlat + synp_wrlat_odd=%x\n", (pi_wrlat + synp_wrlat_odd));

	if ((pi_rdlat + synp_rdlat_odd) == synp_rdlat)
		uartlog("rdlat equal\n");
	else
		uartlog("rdlat Info !!! pi_rdlat + synp_rdlat_odd=%x\n", (pi_rdlat + synp_rdlat_odd));

	#pragma GCC diagnostic pop
}

void change_to_calvl_freq(void)
{
	uartlog("change_to_calvl_freq\n");
	//TOP_REG_EN_PLL_SPEED_CHG =1, TOP_REG_NEXT_PLL_SPEED = 2'b00, TOP_REG_CUR_PLL_SPEED=2'b10
	//apb_read(0x4c, apb_rddata);
	ddr_mmio_rd32(0x4c + CADENCE_PHYD_APB, apb_rddata);
	apb_rddata = modified_bits_by_value(apb_rddata, 1, 0, 0); //TOP_REG_EN_PLL_SPEED_CHG
	//apb_rddata[5:4]   = 2'b10;    //TOP_REG_CUR_PLL_SPEED
	apb_rddata = modified_bits_by_value(apb_rddata, 2, 5, 4); //TOP_REG_EN_PLL_SPEED_CHG
	//apb_rddata[9:8]   = 2'b00;    //TOP_REG_NEXT_PLL_SPEED
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 9, 8); //TOP_REG_EN_PLL_SPEED_CHG
	////original         apb_write(0x4c, apb_rddata);
	mmio_wr32(0x4c + CADENCE_PHYD_APB, apb_rddata);
}

void zq_cal_var(void)
{
	uint32_t tx_zq_golden_drvn;
	uint32_t tx_zq_golden_drvp;
	uint32_t Kn_dqsn_orig_byte0, Kn_dqsn_orig_byte1, Kn_dqsn_orig_byte2, Kn_dqsn_orig_byte3;
	uint32_t Kp_dqsn_orig_byte0, Kp_dqsn_orig_byte1, Kp_dqsn_orig_byte2, Kp_dqsn_orig_byte3;
	uint32_t Kn_dqsp_orig_byte0, Kn_dqsp_orig_byte1, Kn_dqsp_orig_byte2, Kn_dqsp_orig_byte3;
	uint32_t Kp_dqsp_orig_byte0, Kp_dqsp_orig_byte1, Kp_dqsp_orig_byte2, Kp_dqsp_orig_byte3;
	uint32_t Kn_dq_orig_byte0, Kn_dq_orig_byte1, Kn_dq_orig_byte2, Kn_dq_orig_byte3;
	uint32_t Kp_dq_orig_byte0, Kp_dq_orig_byte1, Kp_dq_orig_byte2, Kp_dq_orig_byte3;
	uint32_t Kn_clkn0_orig, Kn_clkn1_orig;
	uint32_t Kp_clkn0_orig, Kp_clkn1_orig;
	uint32_t Kn_clkp0_orig, Kn_clkp1_orig;
	uint32_t Kp_clkp0_orig, Kp_clkp1_orig;
	uint32_t Kn_ca_orig;
	uint32_t Kp_ca_orig;
	uint32_t Kn_csb_orig;
	uint32_t Kp_csb_orig;
	uint32_t kn60;
	uint32_t kp60;

	uartlog("zq_cal_var\n");
	//param_phya_reg_zqcal_done  <= `PI_SD int_regin[14];
	ddr_mmio_rd32(4 * (85 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	//if (rddata[14] == 0b0) { // initial ZQCAL
	if (get_bits_from_value(rddata, 14, 14) == 0) { // initial ZQCAL
		//update golden_drvn/golden_drvp from FT 500*TT IC or efuse
		//param_phya_reg_tx_zq_golden_drvn
		//param_phya_reg_tx_zq_golden_drvp
		ddr_mmio_rd32(4 * (6 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		tx_zq_golden_drvn = get_bits_from_value(rddata, 4, 0);
		tx_zq_golden_drvp = get_bits_from_value(rddata, 12, 8);
		//param_phya_reg_tx_zq_drvn[4:0] <= `PI_SD int_regin[12:8];
		//param_phya_reg_tx_zq_drvp[4:0] <= `PI_SD int_regin[20:16];
		ddr_mmio_rd32(4 * (68 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		kn60 = get_bits_from_value(rddata, 12, 8);
		kp60 = get_bits_from_value(rddata, 20, 16);
		//param_phya_reg_tx_byte0_drvn_dq[4:0] <= `PI_SD int_regin[8:4];
		//param_phya_reg_tx_byte0_drvp_dq[4:0] <= `PI_SD int_regin[30:26];
		//param_phya_reg_tx_byte0_drvn_dqsp[4:0] <= `PI_SD int_regin[20:16];
		//param_phya_reg_tx_byte0_drvn_dqsn[4:0] <= `PI_SD int_regin[14:10];
		ddr_mmio_rd32(4 * (23 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		Kn_dq_orig_byte0 = get_bits_from_value(rddata, 8, 4);
		Kp_dq_orig_byte0 = get_bits_from_value(rddata, 30, 26);
		Kn_dqsp_orig_byte0 = get_bits_from_value(rddata, 20, 16);
		Kn_dqsn_orig_byte0 = get_bits_from_value(rddata, 14, 10);
		Kn_dq_orig_byte0 = ((kn60 * Kn_dq_orig_byte0 + (kn60 << 3) + (Kn_dq_orig_byte0 << 3) - (tx_zq_golden_drvn << 3)) << 2) / (tx_zq_golden_drvn + 8);
		Kp_dq_orig_byte0 = ((kp60 * Kp_dq_orig_byte0 + (kp60 << 3) + (Kp_dq_orig_byte0 << 3) - (tx_zq_golden_drvp << 3)) << 2) / (tx_zq_golden_drvp + 8);
		Kn_dqsp_orig_byte0 = ((kn60 * Kn_dqsp_orig_byte0 + (kn60 << 3) + (Kn_dqsp_orig_byte0 << 3) - (tx_zq_golden_drvn << 3)) << 2) / (tx_zq_golden_drvn + 8);
		Kn_dqsn_orig_byte0 = ((kn60 * Kn_dqsn_orig_byte0 + (kn60 << 3) + (Kn_dqsn_orig_byte0 << 3) - (tx_zq_golden_drvn << 3)) << 2) / (tx_zq_golden_drvn + 8);
		Kn_dq_orig_byte0 = get_bits_from_value(Kn_dq_orig_byte0, 6, 2) + get_bits_from_value(Kn_dq_orig_byte0, 1, 1);
		Kp_dq_orig_byte0 = get_bits_from_value(Kp_dq_orig_byte0, 6, 2) + get_bits_from_value(Kp_dq_orig_byte0, 1, 1);
		Kn_dqsp_orig_byte0 = get_bits_from_value(Kn_dqsp_orig_byte0, 6, 2) + get_bits_from_value(Kn_dqsp_orig_byte0, 1, 1);
		Kn_dqsn_orig_byte0 = get_bits_from_value(Kn_dqsn_orig_byte0, 6, 2) + get_bits_from_value(Kn_dqsn_orig_byte0, 1, 1);
		rddata = modified_bits_by_value(rddata, Kn_dq_orig_byte0, 8, 4);
		rddata = modified_bits_by_value(rddata, Kp_dq_orig_byte0, 30, 26);
		rddata = modified_bits_by_value(rddata, Kn_dqsp_orig_byte0, 20, 16);
		rddata = modified_bits_by_value(rddata, Kn_dqsn_orig_byte0, 14, 10);
		mmio_wr32(4 * (23 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//param_phya_reg_tx_byte0_drvp_dqsp[4:0] <= `PI_SD int_regin[10:6];
		//param_phya_reg_tx_byte0_drvp_dqsn[4:0] <= `PI_SD int_regin[4:0];
		ddr_mmio_rd32(4 * (24 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		Kp_dqsp_orig_byte0 = get_bits_from_value(rddata, 10, 6);
		Kp_dqsn_orig_byte0 = get_bits_from_value(rddata, 4, 0);
		Kp_dqsp_orig_byte0 = ((kp60 * Kp_dqsp_orig_byte0 + (kp60 << 3) + (Kp_dqsp_orig_byte0 << 3) - (tx_zq_golden_drvp << 3)) << 2) / (tx_zq_golden_drvp + 8);
		Kp_dqsn_orig_byte0 = ((kp60 * Kp_dqsn_orig_byte0 + (kp60 << 3) + (Kp_dqsn_orig_byte0 << 3) - (tx_zq_golden_drvp << 3)) << 2) / (tx_zq_golden_drvp + 8);
		Kp_dqsp_orig_byte0 = get_bits_from_value(Kp_dqsp_orig_byte0, 6, 2) + get_bits_from_value(Kp_dqsp_orig_byte0, 1, 1);
		Kp_dqsn_orig_byte0 = get_bits_from_value(Kp_dqsn_orig_byte0, 6, 2) + get_bits_from_value(Kp_dqsn_orig_byte0, 1, 1);
		rddata = modified_bits_by_value(rddata, Kp_dqsp_orig_byte0, 10, 6);
		rddata = modified_bits_by_value(rddata, Kp_dqsn_orig_byte0, 4, 0);
		mmio_wr32(4 * (24 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//param_phya_reg_tx_byte1_drvn_dq[4:0] <= `PI_SD int_regin[8:4];
		//param_phya_reg_tx_byte1_drvp_dq[4:0] <= `PI_SD int_regin[30:26];
		//param_phya_reg_tx_byte1_drvn_dqsp[4:0] <= `PI_SD int_regin[20:16];
		//param_phya_reg_tx_byte1_drvn_dqsn[4:0] <= `PI_SD int_regin[14:10];
		ddr_mmio_rd32(4 * (30 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		Kn_dq_orig_byte1 = get_bits_from_value(rddata, 8, 4);
		Kp_dq_orig_byte1 = get_bits_from_value(rddata, 30, 26);
		Kn_dqsp_orig_byte1 = get_bits_from_value(rddata, 20, 16);
		Kn_dqsn_orig_byte1 = get_bits_from_value(rddata, 14, 10);
		Kn_dq_orig_byte1 = ((kn60 * Kn_dq_orig_byte1 + (kn60 << 3) + (Kn_dq_orig_byte1 << 3) - (tx_zq_golden_drvn << 3)) << 2) / (tx_zq_golden_drvn + 8);
		Kp_dq_orig_byte1 = ((kp60 * Kp_dq_orig_byte1 + (kp60 << 3) + (Kp_dq_orig_byte1 << 3) - (tx_zq_golden_drvp << 3)) << 2) / (tx_zq_golden_drvp + 8);
		Kn_dqsp_orig_byte1 = ((kn60 * Kn_dqsp_orig_byte1 + (kn60 << 3) + (Kn_dqsp_orig_byte1 << 3) - (tx_zq_golden_drvn << 3)) << 2) / (tx_zq_golden_drvn + 8);
		Kn_dqsn_orig_byte1 = ((kn60 * Kn_dqsn_orig_byte1 + (kn60 << 3) + (Kn_dqsn_orig_byte1 << 3) - (tx_zq_golden_drvn << 3)) << 2) / (tx_zq_golden_drvn + 8);
		Kn_dq_orig_byte1 = get_bits_from_value(Kn_dq_orig_byte1, 6, 2) + get_bits_from_value(Kn_dq_orig_byte1, 1, 1);
		Kp_dq_orig_byte1 = get_bits_from_value(Kp_dq_orig_byte1, 6, 2) + get_bits_from_value(Kp_dq_orig_byte1, 1, 1);
		Kn_dqsp_orig_byte1 = get_bits_from_value(Kn_dqsp_orig_byte1, 6, 2) + get_bits_from_value(Kn_dqsp_orig_byte1, 1, 1);
		Kn_dqsn_orig_byte1 = get_bits_from_value(Kn_dqsn_orig_byte1, 6, 2) + get_bits_from_value(Kn_dqsn_orig_byte1, 1, 1);
		rddata = modified_bits_by_value(rddata, Kn_dq_orig_byte1, 8, 4);
		rddata = modified_bits_by_value(rddata, Kp_dq_orig_byte1, 30, 26);
		rddata = modified_bits_by_value(rddata, Kn_dqsp_orig_byte1, 20, 16);
		rddata = modified_bits_by_value(rddata, Kn_dqsn_orig_byte1, 14, 10);
		mmio_wr32(4 * (30 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//param_phya_reg_tx_byte1_drvp_dqsp[4:0] <= `PI_SD int_regin[10:6];
		//param_phya_reg_tx_byte1_drvp_dqsn[4:0] <= `PI_SD int_regin[4:0];
		ddr_mmio_rd32(4 * (31 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		Kp_dqsp_orig_byte1 = get_bits_from_value(rddata, 10, 6);
		Kp_dqsn_orig_byte1 = get_bits_from_value(rddata, 4, 0);
		Kp_dqsp_orig_byte1 = ((kp60 * Kp_dqsp_orig_byte1 + (kp60 << 3) + (Kp_dqsp_orig_byte1 << 3) - (tx_zq_golden_drvp << 3)) << 2) / (tx_zq_golden_drvp + 8);
		Kp_dqsn_orig_byte1 = ((kp60 * Kp_dqsn_orig_byte1 + (kp60 << 3) + (Kp_dqsn_orig_byte1 << 3) - (tx_zq_golden_drvp << 3)) << 2) / (tx_zq_golden_drvp + 8);
		Kp_dqsp_orig_byte1 = get_bits_from_value(Kp_dqsp_orig_byte1, 6, 2) + get_bits_from_value(Kp_dqsp_orig_byte1, 1, 1);
		Kp_dqsn_orig_byte1 = get_bits_from_value(Kp_dqsn_orig_byte1, 6, 2) + get_bits_from_value(Kp_dqsn_orig_byte1, 1, 1);
		rddata = modified_bits_by_value(rddata, Kp_dqsp_orig_byte1, 10, 6);
		rddata = modified_bits_by_value(rddata, Kp_dqsn_orig_byte1, 4, 0);
		mmio_wr32(4 * (31 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//param_phya_reg_tx_byte2_drvn_dq[4:0] <= `PI_SD int_regin[8:4];
		//param_phya_reg_tx_byte2_drvp_dq[4:0] <= `PI_SD int_regin[30:26];
		//param_phya_reg_tx_byte2_drvn_dqsp[4:0] <= `PI_SD int_regin[20:16];
		//param_phya_reg_tx_byte2_drvn_dqsn[4:0] <= `PI_SD int_regin[14:10];
		ddr_mmio_rd32(4 * (37 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		Kn_dq_orig_byte2 = get_bits_from_value(rddata, 8, 4);
		Kp_dq_orig_byte2 = get_bits_from_value(rddata, 30, 26);
		Kn_dqsp_orig_byte2 = get_bits_from_value(rddata, 20, 16);
		Kn_dqsn_orig_byte2 = get_bits_from_value(rddata, 14, 10);
		Kn_dq_orig_byte2 = ((kn60 * Kn_dq_orig_byte2 + (kn60 << 3) + (Kn_dq_orig_byte2 << 3) - (tx_zq_golden_drvn << 3)) << 2) / (tx_zq_golden_drvn + 8);
		Kp_dq_orig_byte2 = ((kp60 * Kp_dq_orig_byte2 + (kp60 << 3) + (Kp_dq_orig_byte2 << 3) - (tx_zq_golden_drvp << 3)) << 2) / (tx_zq_golden_drvp + 8);
		Kn_dqsp_orig_byte2 = ((kn60 * Kn_dqsp_orig_byte2 + (kn60 << 3) + (Kn_dqsp_orig_byte2 << 3) - (tx_zq_golden_drvn << 3)) << 2) / (tx_zq_golden_drvn + 8);
		Kn_dqsn_orig_byte2 = ((kn60 * Kn_dqsn_orig_byte2 + (kn60 << 3) + (Kn_dqsn_orig_byte2 << 3) - (tx_zq_golden_drvn << 3)) << 2) / (tx_zq_golden_drvn + 8);
		Kn_dq_orig_byte2 = get_bits_from_value(Kn_dq_orig_byte2, 6, 2) + get_bits_from_value(Kn_dq_orig_byte2, 1, 1);
		Kp_dq_orig_byte2 = get_bits_from_value(Kp_dq_orig_byte2, 6, 2) + get_bits_from_value(Kp_dq_orig_byte2, 1, 1);
		Kn_dqsp_orig_byte2 = get_bits_from_value(Kn_dqsp_orig_byte2, 6, 2) + get_bits_from_value(Kn_dqsp_orig_byte2, 1, 1);
		Kn_dqsn_orig_byte2 = get_bits_from_value(Kn_dqsn_orig_byte2, 6, 2) + get_bits_from_value(Kn_dqsn_orig_byte2, 1, 1);
		rddata = modified_bits_by_value(rddata, Kn_dq_orig_byte2, 8, 4);
		rddata = modified_bits_by_value(rddata, Kp_dq_orig_byte2, 30, 26);
		rddata = modified_bits_by_value(rddata, Kn_dqsp_orig_byte2, 20, 16);
		rddata = modified_bits_by_value(rddata, Kn_dqsn_orig_byte2, 14, 10);
		mmio_wr32(4 * (37 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//param_phya_reg_tx_byte2_drvp_dqsp[4:0] <= `PI_SD int_regin[10:6];
		//param_phya_reg_tx_byte2_drvp_dqsn[4:0] <= `PI_SD int_regin[4:0];
		ddr_mmio_rd32(4 * (38 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		Kp_dqsp_orig_byte2 = get_bits_from_value(rddata, 10, 6);
		Kp_dqsn_orig_byte2 = get_bits_from_value(rddata, 4, 0);
		Kp_dqsp_orig_byte2 = ((kp60 * Kp_dqsp_orig_byte2 + (kp60 << 3) + (Kp_dqsp_orig_byte2 << 3) - (tx_zq_golden_drvp << 3)) << 2) / (tx_zq_golden_drvp + 8);
		Kp_dqsn_orig_byte2 = ((kp60 * Kp_dqsn_orig_byte2 + (kp60 << 3) + (Kp_dqsn_orig_byte2 << 3) - (tx_zq_golden_drvp << 3)) << 2) / (tx_zq_golden_drvp + 8);
		Kp_dqsp_orig_byte2 = get_bits_from_value(Kp_dqsp_orig_byte2, 6, 2) + get_bits_from_value(Kp_dqsp_orig_byte2, 1, 1);
		Kp_dqsn_orig_byte2 = get_bits_from_value(Kp_dqsn_orig_byte2, 6, 2) + get_bits_from_value(Kp_dqsn_orig_byte2, 1, 1);
		rddata = modified_bits_by_value(rddata, Kp_dqsp_orig_byte2, 10, 6);
		rddata = modified_bits_by_value(rddata, Kp_dqsn_orig_byte2, 4, 0);
		mmio_wr32(4 * (38 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//param_phya_reg_tx_byte3_drvn_dq[4:0] <= `PI_SD int_regin[8:4];
		//param_phya_reg_tx_byte3_drvp_dq[4:0] <= `PI_SD int_regin[30:26];
		//param_phya_reg_tx_byte3_drvn_dqsp[4:0] <= `PI_SD int_regin[20:16];
		//param_phya_reg_tx_byte3_drvn_dqsn[4:0] <= `PI_SD int_regin[14:10];
		ddr_mmio_rd32(4 * (44 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		Kn_dq_orig_byte3 = get_bits_from_value(rddata, 8, 4);
		Kp_dq_orig_byte3 = get_bits_from_value(rddata, 30, 26);
		Kn_dqsp_orig_byte3 = get_bits_from_value(rddata, 20, 16);
		Kn_dqsn_orig_byte3 = get_bits_from_value(rddata, 14, 10);
		Kn_dq_orig_byte3 = ((kn60 * Kn_dq_orig_byte3 + (kn60 << 3) + (Kn_dq_orig_byte3 << 3) - (tx_zq_golden_drvn << 3)) << 2) / (tx_zq_golden_drvn + 8);
		Kp_dq_orig_byte3 = ((kp60 * Kp_dq_orig_byte3 + (kp60 << 3) + (Kp_dq_orig_byte3 << 3) - (tx_zq_golden_drvp << 3)) << 2) / (tx_zq_golden_drvp + 8);
		Kn_dqsp_orig_byte3 = ((kn60 * Kn_dqsp_orig_byte3 + (kn60 << 3) + (Kn_dqsp_orig_byte3 << 3) - (tx_zq_golden_drvn << 3)) << 2) / (tx_zq_golden_drvn + 8);
		Kn_dqsn_orig_byte3 = ((kn60 * Kn_dqsn_orig_byte3 + (kn60 << 3) + (Kn_dqsn_orig_byte3 << 3) - (tx_zq_golden_drvn << 3)) << 2) / (tx_zq_golden_drvn + 8);
		Kn_dq_orig_byte3 = get_bits_from_value(Kn_dq_orig_byte3, 6, 2) + get_bits_from_value(Kn_dq_orig_byte3, 1, 1);
		Kp_dq_orig_byte3 = get_bits_from_value(Kp_dq_orig_byte3, 6, 2) + get_bits_from_value(Kp_dq_orig_byte3, 1, 1);
		Kn_dqsp_orig_byte3 = get_bits_from_value(Kn_dqsp_orig_byte3, 6, 2) + get_bits_from_value(Kn_dqsp_orig_byte3, 1, 1);
		Kn_dqsn_orig_byte3 = get_bits_from_value(Kn_dqsn_orig_byte3, 6, 2) + get_bits_from_value(Kn_dqsn_orig_byte3, 1, 1);
		rddata = modified_bits_by_value(rddata, Kn_dq_orig_byte3, 8, 4);
		rddata = modified_bits_by_value(rddata, Kp_dq_orig_byte3, 30, 26);
		rddata = modified_bits_by_value(rddata, Kn_dqsp_orig_byte3, 20, 16);
		rddata = modified_bits_by_value(rddata, Kn_dqsn_orig_byte3, 14, 10);
		mmio_wr32(4 * (44 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//param_phya_reg_tx_byte3_drvp_dqsp[4:0] <= `PI_SD int_regin[10:6];
		//param_phya_reg_tx_byte3_drvp_dqsn[4:0] <= `PI_SD int_regin[4:0];
		ddr_mmio_rd32(4 * (45 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		Kp_dqsp_orig_byte3 = get_bits_from_value(rddata, 10, 6);
		Kp_dqsn_orig_byte3 = get_bits_from_value(rddata, 4, 0);
		Kp_dqsp_orig_byte3 = ((kp60 * Kp_dqsp_orig_byte3 + (kp60 << 3) + (Kp_dqsp_orig_byte3 << 3) - (tx_zq_golden_drvp << 3)) << 2) / (tx_zq_golden_drvp + 8);
		Kp_dqsn_orig_byte3 = ((kp60 * Kp_dqsn_orig_byte3 + (kp60 << 3) + (Kp_dqsn_orig_byte3 << 3) - (tx_zq_golden_drvp << 3)) << 2) / (tx_zq_golden_drvp + 8);
		Kp_dqsp_orig_byte3 = get_bits_from_value(Kp_dqsp_orig_byte3, 6, 2) + get_bits_from_value(Kp_dqsp_orig_byte3, 1, 1);
		Kp_dqsn_orig_byte3 = get_bits_from_value(Kp_dqsn_orig_byte3, 6, 2) + get_bits_from_value(Kp_dqsn_orig_byte3, 1, 1);
		rddata = modified_bits_by_value(rddata, Kp_dqsp_orig_byte3, 10, 6);
		rddata = modified_bits_by_value(rddata, Kp_dqsn_orig_byte3, 4, 0);
		mmio_wr32(4 * (45 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//param_phya_reg_tx_ca_drvn_ca[4:0] <= `PI_SD int_regin[4:0];
		//param_phya_reg_tx_ca_drvn_csb[4:0] <= `PI_SD int_regin[10:6];
		//param_phya_reg_tx_ca_drvp_ca[4:0] <= `PI_SD int_regin[20:16];
		//param_phya_reg_tx_ca_drvp_csb[4:0] <= `PI_SD int_regin[27:23];
		ddr_mmio_rd32(4 * (51 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		Kn_ca_orig = get_bits_from_value(rddata, 4, 0);
		Kp_ca_orig = get_bits_from_value(rddata, 20, 16);
		Kn_csb_orig = get_bits_from_value(rddata, 10, 6);
		Kp_csb_orig = get_bits_from_value(rddata, 27, 23);
		Kn_ca_orig = ((kn60 * Kn_ca_orig + (kn60 << 3) + (Kn_ca_orig << 3) - (tx_zq_golden_drvn << 3)) << 2) / (tx_zq_golden_drvn + 8);
		Kp_ca_orig = ((kp60 * Kp_ca_orig + (kp60 << 3) + (Kp_ca_orig << 3) - (tx_zq_golden_drvp << 3)) << 2) / (tx_zq_golden_drvp + 8);
		Kn_csb_orig = ((kn60 * Kn_csb_orig + (kn60 << 3) + (Kn_csb_orig << 3) - (tx_zq_golden_drvn << 3)) << 2) / (tx_zq_golden_drvn + 8);
		Kp_csb_orig = ((kp60 * Kp_csb_orig + (kp60 << 3) + (Kp_csb_orig << 3) - (tx_zq_golden_drvp << 3)) << 2) / (tx_zq_golden_drvp + 8);
		Kn_ca_orig = get_bits_from_value(Kn_ca_orig, 6, 2) + get_bits_from_value(Kn_ca_orig, 1, 1);
		Kp_ca_orig = get_bits_from_value(Kp_ca_orig, 6, 2) + get_bits_from_value(Kp_ca_orig, 1, 1);
		Kn_csb_orig = get_bits_from_value(Kn_csb_orig, 6, 2) + get_bits_from_value(Kn_csb_orig, 1, 1);
		Kp_csb_orig = get_bits_from_value(Kp_csb_orig, 6, 2) + get_bits_from_value(Kp_csb_orig, 1, 1);
		rddata = modified_bits_by_value(rddata, Kn_ca_orig, 4, 0);
		rddata = modified_bits_by_value(rddata, Kp_ca_orig, 20, 16);
		rddata = modified_bits_by_value(rddata, Kn_csb_orig, 10, 6);
		rddata = modified_bits_by_value(rddata, Kp_csb_orig, 27, 23);
		mmio_wr32(4 * (51 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//param_phya_reg_tx_clk_drvn_clkn0[4:0] <= `PI_SD int_regin[4:0];
		//param_phya_reg_tx_clk_drvn_clkn1[4:0] <= `PI_SD int_regin[12:8];
		//param_phya_reg_tx_clk_drvn_clkp0[4:0] <= `PI_SD int_regin[20:16];
		//param_phya_reg_tx_clk_drvn_clkp1[4:0] <= `PI_SD int_regin[28:24];
		ddr_mmio_rd32(4 * (64 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		Kn_clkn0_orig = get_bits_from_value(rddata, 4, 0);
		Kn_clkn1_orig = get_bits_from_value(rddata, 12, 8);
		Kn_clkp0_orig = get_bits_from_value(rddata, 20, 16);
		Kn_clkp1_orig = get_bits_from_value(rddata, 28, 24);
		Kn_clkn0_orig = ((kn60 * Kn_clkn0_orig + (kn60 << 3) + (Kn_clkn0_orig << 3) - (tx_zq_golden_drvn << 3)) << 2) / (tx_zq_golden_drvn + 8);
		Kn_clkn1_orig = ((kn60 * Kn_clkn1_orig + (kn60 << 3) + (Kn_clkn1_orig << 3) - (tx_zq_golden_drvn << 3)) << 2) / (tx_zq_golden_drvn + 8);
		Kn_clkp0_orig = ((kn60 * Kn_clkp0_orig + (kn60 << 3) + (Kn_clkp0_orig << 3) - (tx_zq_golden_drvn << 3)) << 2) / (tx_zq_golden_drvn + 8);
		Kn_clkp1_orig = ((kn60 * Kn_clkp1_orig + (kn60 << 3) + (Kn_clkp1_orig << 3) - (tx_zq_golden_drvn << 3)) << 2) / (tx_zq_golden_drvn + 8);
		Kn_clkn0_orig = get_bits_from_value(Kn_clkn0_orig, 6, 2) + get_bits_from_value(Kn_clkn0_orig, 1, 1);
		Kn_clkn1_orig = get_bits_from_value(Kn_clkn1_orig, 6, 2) + get_bits_from_value(Kn_clkn1_orig, 1, 1);
		Kn_clkp0_orig = get_bits_from_value(Kn_clkp0_orig, 6, 2) + get_bits_from_value(Kn_clkp0_orig, 1, 1);
		Kn_clkp1_orig = get_bits_from_value(Kn_clkp1_orig, 6, 2) + get_bits_from_value(Kn_clkp1_orig, 1, 1);
		rddata = modified_bits_by_value(rddata, Kn_clkn0_orig, 4, 0);
		rddata = modified_bits_by_value(rddata, Kn_clkn1_orig, 12, 8);
		rddata = modified_bits_by_value(rddata, Kn_clkp0_orig, 20, 16);
		rddata = modified_bits_by_value(rddata, Kn_clkp1_orig, 28, 24);
		mmio_wr32(4 * (64 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		//param_phya_reg_tx_clk_drvp_clkn0[4:0] <= `PI_SD int_regin[4:0];
		//param_phya_reg_tx_clk_drvp_clkn1[4:0] <= `PI_SD int_regin[12:8];
		//param_phya_reg_tx_clk_drvp_clkp0[4:0] <= `PI_SD int_regin[20:16];
		//param_phya_reg_tx_clk_drvp_clkp1[4:0] <= `PI_SD int_regin[28:24];
		ddr_mmio_rd32(4 * (65 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
		Kp_clkn0_orig = get_bits_from_value(rddata, 4, 0);
		Kp_clkn1_orig = get_bits_from_value(rddata, 12, 8);
		Kp_clkp0_orig = get_bits_from_value(rddata, 20, 16);
		Kp_clkp1_orig = get_bits_from_value(rddata, 28, 24);
		Kp_clkn0_orig = ((kp60 * Kp_clkn0_orig + (kp60 << 3) + (Kp_clkn0_orig << 3) - (tx_zq_golden_drvp << 3)) << 2) / (tx_zq_golden_drvp + 8);
		Kp_clkn1_orig = ((kp60 * Kp_clkn1_orig + (kp60 << 3) + (Kp_clkn1_orig << 3) - (tx_zq_golden_drvp << 3)) << 2) / (tx_zq_golden_drvp + 8);
		Kp_clkp0_orig = ((kp60 * Kp_clkp0_orig + (kp60 << 3) + (Kp_clkp0_orig << 3) - (tx_zq_golden_drvp << 3)) << 2) / (tx_zq_golden_drvp + 8);
		Kp_clkp1_orig = ((kp60 * Kp_clkp1_orig + (kp60 << 3) + (Kp_clkp1_orig << 3) - (tx_zq_golden_drvp << 3)) << 2) / (tx_zq_golden_drvp + 8);
		Kp_clkn0_orig = get_bits_from_value(Kp_clkn0_orig, 6, 2) + get_bits_from_value(Kp_clkn0_orig, 1, 1);
		Kp_clkn1_orig = get_bits_from_value(Kp_clkn1_orig, 6, 2) + get_bits_from_value(Kp_clkn1_orig, 1, 1);
		Kp_clkp0_orig = get_bits_from_value(Kp_clkp0_orig, 6, 2) + get_bits_from_value(Kp_clkp0_orig, 1, 1);
		Kp_clkp1_orig = get_bits_from_value(Kp_clkp1_orig, 6, 2) + get_bits_from_value(Kp_clkp1_orig, 1, 1);
		rddata = modified_bits_by_value(rddata, Kp_clkn0_orig, 4, 0);
		rddata = modified_bits_by_value(rddata, Kp_clkn1_orig, 12, 8);
		rddata = modified_bits_by_value(rddata, Kp_clkp0_orig, 20, 16);
		rddata = modified_bits_by_value(rddata, Kp_clkp1_orig, 28, 24);
		mmio_wr32(4 * (65 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	}
}

void set_dfi_init_start(void)
{
	// synp setting
	// phy is ready for initial dfi_init_start request
	// set umctl2 to tigger dfi_init_start
	uartlog("set_dfi_init_start\n");
	//ddr_debug_wr32(0x0d);
	//ddr_debug_num_write();
	mmio_wr32(cfg_base+0x00000320, 0x00000000);
	ddr_mmio_rd32(cfg_base+0x000001b0, read_data);       //dfi_init_start @ read_data[5];
	//original        read_data[5] = 0b1;
	read_data = modified_bits_by_value(read_data, 1, 5, 5);//
	mmio_wr32(cfg_base+0x000001b0, read_data);
	mmio_wr32(cfg_base+0x00000320, 1);
}

void rdgtlvl(void)
{
	// ###############################################################
	rddata = mmio_rd32(0x080040e0);
	// rddata = modified_bits_by_value(rddata, 1, 1, 1);
	rddata = modified_bits_by_value(rddata, 1, 6, 6);
	rddata = modified_bits_by_value(rddata, 1, 7, 7);
	bm_synp_mrw_lp4(13, get_bits_from_value(rddata, 15, 0));

	// ####static mode
	rddata = mmio_rd32(0x080040dc);
	rddata = modified_bits_by_value(rddata, 0, 19, 19); //static mode
	bm_synp_mrw_lp4(1, get_bits_from_value(rddata, 31, 16));
	pi_rdlvl_gate_status();
	pi_rdlvl_gate_req();

	// ####toggle mode
	rddata = mmio_rd32(0x080040dc);
	rddata = modified_bits_by_value(rddata, 1, 19, 19); //toggle mode
	bm_synp_mrw_lp4(1, get_bits_from_value(rddata, 31, 16));

	// ###############################################################
}

void ddr_freq_change_htol(void)
{
	// uint32_t en_dfi_dram_clk_disable;
	// uint32_t powerdown_en;
	// uint32_t selfref_en;
#if defined(DDR3) || defined(DDR4)
	uint32_t rtt_nom;
#endif
#ifdef DDR4
	uint32_t rtt_park;
#endif
#if defined(DDR3) || defined(DDR4)
	uint32_t rtt_wr;
	uint32_t mwr_temp;
#endif

	uartlog("ddr_freq_change_htol\n");

	//TOP_REG_EN_PLL_SPEED_CHG =1, TOP_REG_NEXT_PLL_SPEED = 0b01, TOP_REG_CUR_PLL_SPEED=0b10
	//original      apb_read(0x4c, apb_rddata);
	ddr_mmio_rd32(0x4c + CADENCE_PHYD_APB, apb_rddata);
	//apb_rddata[0]     = 1;        //TOP_REG_EN_PLL_SPEED_CHG
	//apb_rddata[5:4]   = 0b10;    //TOP_REG_CUR_PLL_SPEED
	//apb_rddata[9:8]   = 0b01;    //TOP_REG_NEXT_PLL_SPEED
	apb_rddata = modified_bits_by_value(apb_rddata, 1, 0, 0);
	apb_rddata = modified_bits_by_value(apb_rddata, 2, 5, 4);
	apb_rddata = modified_bits_by_value(apb_rddata, 1, 9, 8);

	////original      apb_write(0x4c, apb_rddata);
	mmio_wr32(0x4c + CADENCE_PHYD_APB, apb_rddata);

	//save lowpower setting
	ddr_mmio_rd32(cfg_base + 0x30, read_data);
	//en_dfi_dram_clk_disable = read_data[3];
	//powerdown_en            = read_data[1];
	//selfref_en              = read_data[0];
	// en_dfi_dram_clk_disable = get_bits_from_value(read_data, 3, 3);
	// powerdown_en = get_bits_from_value(read_data, 1, 1);
	// selfref_en = get_bits_from_value(read_data, 0, 0);
	read_data = modified_bits_by_value(read_data, 0, 3, 3);
	read_data = modified_bits_by_value(read_data, 0, 1, 1);
	read_data = modified_bits_by_value(read_data, 0, 0, 0);
	mmio_wr32(cfg_base + 0x30, read_data);

	// Write 0 to PCTRL_n.port_en
	for (int i = 0; i < 8; i++)
		mmio_wr32(cfg_base + 0x490 + 0xb0 * i, 0x0);

	// Poll PSTAT.rd_port_busy_n = 0
	// Poll PSTAT.wr_port_busy_n = 0
	while (1) {
		ddr_mmio_rd32(cfg_base + 0x3fc, read_data);
		if (read_data == 0)
			break;
	}

	//kc
	// // 3. Set RFSHCTL3.dis_auto_refresh=1, to disable automatic refreshes
	// ddr_mmio_rd32(cfg_base + 0x60, read_data);
	// //read_data[0] = 0x1; //RFSHCTL3.dis_auto_refresh
	// read_data = modified_bits_by_value(read_data, 1, 0, 0);
	// mmio_wr32(cfg_base + 0x60, read_data);

	// 5. Perform an MRS command (using MRCTRL0 and MRCTRL1 registers) to disable RTT_NOM:
	//    a. DDR3: Write ????to MR1[9], MR1[6] and MR1[2]
	//    b. DDR4: Write ????to MR1[10:8]
	// 6. For DDR4 only: Perform an MRS command (using MRCTRL0 and MRCTRL1 registers) to write ????to
	//    MR5[8:6] to disable RTT_PARK
	// 7. Perform an MRS command (using MRCTRL0 and MRCTRL1 registers) to write ????to MR2[10:9], to
	//    disable RTT_WR (and therefore disable dynamic ODT). This applies for both DDR3 and DDR4.

#ifdef DDR4
	//read mr1 @INIT3
	ddr_mmio_rd32(cfg_base + 0xdc, read_data);
	//rtt_nom = read_data[10:8];
	rtt_nom = get_bits_from_value(read_data, 10, 8);
	mwr_temp = 0;
	mwr_temp = get_bits_from_value(read_data, 15, 11) << 11 | get_bits_from_value(read_data, 7, 0);
	//bm_synp_mrw(0x1,  {read_data[15:11], 0b000, read_data[7:0]});
	bm_synp_mrw(0x1, mwr_temp);
	uartlog("disable RTT_NOM DDR4: Write ????to MR1[10:8]\n");
	//read mr5 @INIT6
	ddr_mmio_rd32(cfg_base + 0xe8, read_data);
	//rtt_park = read_data[8:6];
	rtt_park = get_bits_from_value(read_data, 8, 6);
	//bm_synp_mrw(0x5,  {read_data[15:9], 0b000, read_data[5:0]});
	mwr_temp = 0;
	mwr_temp = get_bits_from_value(read_data, 15, 9) << 9 | get_bits_from_value(read_data, 5, 0);
	bm_synp_mrw(0x5, mwr_temp);
	uartlog("write ????to MR5[8:6] to disable RTT_PARK\n");
	//read mr2 @INIT4
	ddr_mmio_rd32(cfg_base + 0xe0, read_data);
	//rtt_wr = read_data[25:24];
	rtt_wr = get_bits_from_value(read_data, 25, 24);
	mwr_temp = 0;
	mwr_temp = get_bits_from_value(read_data, 31, 26) << 10 | get_bits_from_value(read_data, 23, 16);
	//bm_synp_mrw(0x2,  {read_data[31:26], 0b00, read_data[23:16]});
	bm_synp_mrw(0x2, mwr_temp);
	uartlog("write ????to MR2[10:9], to disable RTT_WR\n");
#endif

#ifdef DDR3
	//read mr1 @INIT3
	ddr_mmio_rd32(cfg_base + 0xdc, read_data);
	//rtt_nom = {read_data[9], read_data[6], read_data[2]};
	rtt_nom = get_bits_from_value(read_data, 9, 9) << 3 | get_bits_from_value(read_data, 6, 6) | get_bits_from_value(read_data, 2, 2);
	//bm_synp_mrw(0x1,  {read_data[15:10], 0b0, read_data[8:7], 0b0, read_data[5:3], 0b0, read_data[1:0]});
	mwr_temp = 0;
	mwr_temp = get_bits_from_value(read_data, 15, 10) << 10 | get_bits_from_value(read_data, 8, 7) << 7 | get_bits_from_value(read_data, 5, 3) << 3 | get_bits_from_value(read_data, 1, 0);
	bm_synp_mrw(0x1, mwr_temp);
	uartlog("disable RTT_NOM DDR3: Write to MR1[9], MR1[6] and MR1[2]\n");
	//read mr2 @INIT4
	ddr_mmio_rd32(cfg_base + 0xe0, read_data);
	//rtt_wr = read_data[25:24];
	rtt_wr = get_bits_from_value(read_data, 25, 24);
	//bm_synp_mrw(0x2,  {read_data[31:26], 0b00, read_data[23:16]});
	bm_synp_mrw(0x2, get_bits_from_value(read_data, 31, 26) << 10 | get_bits_from_value(read_data, 23, 16));
	uartlog("write ????to MR2[10:9], to disable RTT_WR\n");
#endif

	//kc
	// 3. Set RFSHCTL3.dis_auto_refresh=1, to disable automatic refreshes
	ddr_mmio_rd32(cfg_base + 0x60, read_data);
	//read_data[0] = 0x1; //RFSHCTL3.dis_auto_refresh
	read_data = modified_bits_by_value(read_data, 1, 0, 0);
	mmio_wr32(cfg_base + 0x60, read_data);

	// 8. Perform an MRS command (using MRCTRL0 and MRCTRL1 registers) to disable the DLL. The timing of
	// this MRS is automatically handled by the uMCTL2.
	// a. DDR3: Write ????to MR1[0]
	// b. DDR4: Write ????to MR1[0]
#ifdef DDR4
	//read mr1 @INIT3
	ddr_mmio_rd32(cfg_base + 0xdc, read_data);
	//DDR4: Write ????to MR1[0]
	//bm_synp_mrw(0x1,  {read_data[15:1], 0b0});
	bm_synp_mrw(0x1, get_bits_from_value(read_data, 15, 1) << 1 | 0);
	uartlog("DDR4: Write ????to MR1[0]\n");
#endif
#ifdef DDR3
	ddr_mmio_rd32(cfg_base + 0x30, read_data);
	//read_data[3] = 0x0; //PWRCTL.en_dfi_dram_clk_disable
	//read_data[1] = 0x0; //PWRCTL.powerdown_en
	//read_data[0] = 0x0; //PWRCTL.selfref_en
	read_data = modified_bits_by_value(read_data, 0, 3, 3);
	read_data = modified_bits_by_value(read_data, 0, 1, 1);
	read_data = modified_bits_by_value(read_data, 0, 0, 0);
	mmio_wr32(cfg_base + 0x30, read_data);
	//read mr1 @INIT3
	ddr_mmio_rd32(cfg_base + 0xdc, read_data);
	//DDR3: Write ????to MR1[0]
	//bm_synp_mrw(0x1,  {read_data[15:1], 0b1});
	bm_synp_mrw(0x1, get_bits_from_value(read_data, 15, 1) << 1 | 1);
#endif
	// 9. Put the SDRAM into self-refresh mode by setting PWRCTL.selfref_sw = 1, and polling STAT.operating_
	// mode to ensure the DDRC has entered self-refresh.
	// Write 1 to PWRCTL.selfref_sw
	ddr_mmio_rd32(cfg_base + 0x30, read_data);
	//read_data[5] = 0x1; //PWRCTL.selfref_sw
	//read_data[3] = 0x0; //PWRCTL.en_dfi_dram_clk_disable
	//read_data[1] = 0x0; //PWRCTL.powerdown_en
	//read_data[0] = 0x0; //PWRCTL.selfref_en
	read_data = modified_bits_by_value(read_data, 1, 5, 5);
	read_data = modified_bits_by_value(read_data, 0, 3, 3);
	read_data = modified_bits_by_value(read_data, 0, 1, 1);
	read_data = modified_bits_by_value(read_data, 0, 0, 0);
	mmio_wr32(cfg_base + 0x30, read_data);

	// Poll STAT.selfref_type= 2?�b10
	// Poll STAT.selfref_state = 0b10 (LPDDR4 only)
#ifndef LP4
	while (1) {
		ddr_mmio_rd32(cfg_base + 0x4, read_data);
		if (get_bits_from_value(read_data, 5, 4) == 2)
			break;
	}
	//11. Set the MSTR.dll_off_mode = 1.
	ddr_mmio_rd32(cfg_base + 0x0, read_data);
	//read_data[15] = 0x1;
	read_data = modified_bits_by_value(read_data, 1, 15, 15);
	mmio_wr32(cfg_base + 0x0, read_data);
#else
	while (1) {
		ddr_mmio_rd32(cfg_base + 0x4, read_data);
		if (get_bits_from_value(read_data, 9, 8) == 0x2)
			break;
	}
#endif

	//Change PLL frequency
	chg_pll_freq();
	//test only when dll_cal is not use
/*
#ifdef DDR3
	//param_phyd_dll_sw_code    <= `PI_SD int_regin[23:16]
	//param_phyd_dll_sw_code_mode    <= `PI_SD int_regin[8];
	////original              REGREAD(169 + PHY_BASE_ADDR, rddata);
	ddr_mmio_rd32(4*(169 + PHY_BASE_ADDR)+CADENCE_PHYD,  rddata);
	//original              rddata[23:16] = 0x54;
	rddata=modified_bits_by_value(rddata, 0x54, 23, 16 );//
	//original              rddata[8] = 0b1;
	rddata=modified_bits_by_value(rddata, 0b1, 8, 8 );//
	//original              REGWR  (169 + PHY_BASE_ADDR, rddata, 0);
	mmio_wr32(4*(169 + PHY_BASE_ADDR)+CADENCE_PHYD,  rddata);
	uartlog("param_phyd_dll_sw_code\n");
	//param_phyd_dll_sw_clr          <= `PI_SD int_regin[7];
	//             REGREAD(169 + PHY_BASE_ADDR, rddata);
	//original              rddata[7] = 0b1;
	rddata=modified_bits_by_value(rddata, 0b1, 7, 7 );//
	//original              REGWR  (169 + PHY_BASE_ADDR, rddata, 0);
	mmio_wr32(4*(169 + PHY_BASE_ADDR)+CADENCE_PHYD,  rddata);
	//original              rddata[7] = 0b0;
	rddata=modified_bits_by_value(rddata, 0b0, 7, 7 );//
	//original              REGWR  (169 + PHY_BASE_ADDR, rddata, 0);
	mmio_wr32(4*(169 + PHY_BASE_ADDR)+CADENCE_PHYD,  rddata);
#endif
*/
	//dll_cal
	dll_cal();

	// refresh requirement
	// Program controller
	ddr_mmio_rd32(cfg_base + 0x64, read_data);
	//read_data[27:16] = read_data[27:17];
	read_data = modified_bits_by_value(read_data, get_bits_from_value(read_data, 27, 17), 27, 16);
	//read_data[9:0] = read_data[9:1] + read_data[0];
	read_data = modified_bits_by_value(read_data, get_bits_from_value(read_data, 9, 1) + get_bits_from_value(read_data, 0, 0), 9, 0);
	mmio_wr32(cfg_base + 0x64, read_data);
	ddr_mmio_rd32(cfg_base + 0x68, read_data);
	//read_data[23:16] = read_data[23:17] + read_data[16];
	read_data = modified_bits_by_value(read_data, get_bits_from_value(read_data, 23, 17) + get_bits_from_value(read_data, 16, 16), 23, 16);
	mmio_wr32(cfg_base + 0x68, read_data);
	// program phyupd_mask
	ddr_mmio_rd32(0x0800a504, read_data);
	//read_data[27:16] = read_data[27:17] + read_data[16];
	read_data = modified_bits_by_value(read_data, get_bits_from_value(read_data, 27, 17) + get_bits_from_value(read_data, 16, 16), 27, 16);
	mmio_wr32(0x0800a504, read_data);
	ddr_mmio_rd32(0x0800a500, read_data);
	//read_data[31:16] = read_data[31:17];
	read_data = modified_bits_by_value(read_data, get_bits_from_value(read_data, 31, 17), 31, 16);
	mmio_wr32(0x0800a500, read_data);
#ifndef LP4
	//9. Set MSTR.dll_off_mode = 0
	ddr_mmio_rd32(cfg_base + 0x0, read_data);
	//read_data[15] = 0x0;
	read_data = modified_bits_by_value(read_data, 0, 15, 15);
	mmio_wr32(cfg_base + 0x0, read_data);
#endif
	ddr_mmio_rd32(cfg_base + 0x30, read_data);
	read_data = modified_bits_by_value(read_data, 0, 5, 5);
	read_data = modified_bits_by_value(read_data, 0, 3, 3);
	read_data = modified_bits_by_value(read_data, 0, 1, 1);
	read_data = modified_bits_by_value(read_data, 0, 0, 0);
	mmio_wr32(cfg_base + 0x30, read_data);
	while (1) {
		ddr_mmio_rd32(cfg_base + 0x4, read_data);
		if (get_bits_from_value(read_data, 5, 4) == 0)
			break;
	}
	while (1) {
		ddr_mmio_rd32(cfg_base + 0x4, read_data);
		if (get_bits_from_value(read_data, 1, 0) == 1)
			break;
	}
#ifdef DDR4
	ddr_mmio_rd32(cfg_base + 0xdc, read_data);
	bm_synp_mrw(0x1, get_bits_from_value(read_data, 15, 1) << 1 | 1);
	uartlog("DDR4: Write ????to MR1[0]\n");
	ddr_mmio_rd32(cfg_base + 0xdc, read_data);
	bm_synp_mrw(0x0, get_bits_from_value(read_data, 31, 25) << 9 | 0b1 << 8 | get_bits_from_value(read_data, 23, 16));
	uartlog("15. Perform an MRS command\n");
	ddr_mmio_rd32(cfg_base + 0xdc, read_data);
	bm_synp_mrw(0x1, get_bits_from_value(15, 11) << 11 | get_bits_from_value(rtt_nom, 2, 0) << 8 | get_bits_from_value(read_data, 7, 0));
	uartlog("enable RTT_NOM\n");
	ddr_mmio_rd32(cfg_base + 0xe8, read_data);
	bm_synp_mrw(0x5, get_bits_from_value(read_data, 15, 9) << 9 | get_bits_from_value(rtt_park, 2, 0) << 6 | get_bits_from_value(read_data, 5, 0));
	uartlog("enable RTT_PARK\n");
	ddr_mmio_rd32(cfg_base + 0xe0, read_data);
	bm_synp_mrw(0x2, get_bits_from_value(read_data, 31, 26) << 11 | get_bits_from_value(rtt_wr, 1, 0) << 8 | get_bits_from_value(read_data, 23, 16));
	uartlog("enable RTT_WR\n");
#endif
#ifdef DDR3
	ddr_mmio_rd32(cfg_base + 0xe0, read_data);
	bm_synp_mrw(0x2, get_bits_from_value(read_data, 31, 26) << 11 | get_bits_from_value(rtt_wr, 1, 0) << 8 | get_bits_from_value(read_data, 23, 16));
	uartlog("enable RTT_WR\n");
	ddr_mmio_rd32(cfg_base + 0xdc, read_data);
	bm_synp_mrw(0x1, get_bits_from_value(read_data, 15, 10) << 10 | get_bits_from_value(rtt_nom, 2, 2) << 9 | get_bits_from_value(read_data, 8, 7) << 7 | get_bits_from_value(rtt_nom, 1, 1) << 5 | get_bits_from_value(read_data, 5, 3) << 3 | get_bits_from_value(rtt_nom, 0, 0) << 2 | get_bits_from_value(read_data, 1, 1) << 1 | 0b0);
	uartlog("enable RTT_NOM\n");
	ddr_mmio_rd32(cfg_base + 0xdc, read_data);
	bm_synp_mrw(0x0, get_bits_from_value(read_data, 31, 25) << 9 | 0b1 << 8 | get_bits_from_value(read_data, 23, 16));
	opdelay(2200);

#endif
	ddr_mmio_rd32(cfg_base + 0x60, read_data);
	read_data = modified_bits_by_value(read_data, 0, 0, 0);
	mmio_wr32(cfg_base + 0x60, read_data);
	ddr_mmio_rd32(cfg_base + 0x30, read_data);
	// Related to control power
	read_data = modified_bits_by_value(read_data, 0, 5, 5);
	// read_data = modified_bits_by_value(read_data, en_dfi_dram_clk_disable, 3, 3);
	read_data = modified_bits_by_value(read_data, 0, 3, 3);
	read_data = modified_bits_by_value(read_data, 0, 1, 1);
	// read_data = modified_bits_by_value(read_data, selfref_en, 0, 0);
	read_data = modified_bits_by_value(read_data, 0, 0, 0);
	mmio_wr32(cfg_base + 0x30, read_data);
	uartlog("restore selfref_en powerdown_en\n");
#ifndef DDR3
	uartlog("training\n");
#ifdef LP4
	ddr_mmio_rd32(4 * (197 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = modified_bits_by_value(rddata, 0x800, 12, 0);
	mmio_wr32(4 * (197 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	pi_wrlvl_req();
	rdgtlvl();
	// pi_rdlvl_gate_req();
	pi_rdlvl_req();
	pi_wdqlvl_req();
#else
	pi_rdlvl_gate_req();
#endif
#endif
	for (int i = 0; i < 8; i++)
		mmio_wr32(cfg_base + 0x490 + 0xb0 * i, 0x1);

	ddr_mmio_rd32(0x4c + CADENCE_PHYD_APB, apb_rddata);
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 0, 0);
	mmio_wr32(0x4c + CADENCE_PHYD_APB, apb_rddata);
}

void ddr_freq_change_ltoh(void)
{
	// uint32_t en_dfi_dram_clk_disable;
	// uint32_t powerdown_en;
	// uint32_t selfref_en;
#if defined(DDR3) || defined(DDR4)
	uint32_t rtt_nom;
#endif
#ifdef DDR4
	uint32_t rtt_park;
#endif
#if defined(DDR3) || defined(DDR4)
	uint32_t mwr_temp;
#endif

	uartlog("ddr_freq_change_ltoh\n");

	//TOP_REG_EN_PLL_SPEED_CHG =1, TOP_REG_NEXT_PLL_SPEED = 0b01, TOP_REG_CUR_PLL_SPEED=0b10
	//original      apb_read(0x4c, apb_rddata);
	ddr_mmio_rd32(0x4c + CADENCE_PHYD_APB, apb_rddata);
	//apb_rddata[0]     = 1;        //TOP_REG_EN_PLL_SPEED_CHG
	//apb_rddata[5:4]   = 0b01;    //TOP_REG_CUR_PLL_SPEED
	//apb_rddata[9:8]   = 0b10;    //TOP_REG_NEXT_PLL_SPEED
	apb_rddata = modified_bits_by_value(apb_rddata, 1, 0, 0);
	apb_rddata = modified_bits_by_value(apb_rddata, 1, 5, 4);
	apb_rddata = modified_bits_by_value(apb_rddata, 2, 9, 8);
	////original      apb_write(0x4c, apb_rddata);
	mmio_wr32(0x4c + CADENCE_PHYD_APB, apb_rddata);
	//save lowpower setting
	ddr_mmio_rd32(cfg_base + 0x30, read_data);
	//en_dfi_dram_clk_disable = read_data[3];
	// powerdown_en            = read_data[1];
	// selfref_en              = read_data[0];
	// en_dfi_dram_clk_disable = get_bits_from_value(read_data, 3, 3);
	// powerdown_en = get_bits_from_value(read_data, 1, 1);
	// selfref_en = get_bits_from_value(read_data, 0, 0);
	read_data = modified_bits_by_value(read_data, 0, 3, 3);
	read_data = modified_bits_by_value(read_data, 0, 1, 1);
	read_data = modified_bits_by_value(read_data, 0, 0, 0);
	mmio_wr32(cfg_base + 0x30, read_data);
	// Write 0 to PCTRL_n.port_en
	for (int i = 0; i < 8; i++)
		mmio_wr32(cfg_base + 0x490 + 0xb0 * i, 0x0);

	// Poll PSTAT.rd_port_busy_n = 0
	// Poll PSTAT.wr_port_busy_n = 0
	while (1) {
		ddr_mmio_rd32(cfg_base + 0x3fc, read_data);
		if (read_data == 0)
			break;
	}
	//kc
	// // 3. Set RFSHCTL3.dis_auto_refresh=1, to disable automatic refreshes
	// ddr_mmio_rd32(cfg_base + 0x60, read_data);
	// //read_data[0] = 0x1; //RFSHCTL3.dis_auto_refresh
	// read_data = modified_bits_by_value(read_data, 1, 0, 0);
	// mmio_wr32(cfg_base + 0x60, read_data);

	// 5. Perform an MRS command (using MRCTRL0 and MRCTRL1 registers) to disable RTT_NOM:
	//    a. DDR3: Write ????to MR1[9], MR1[6] and MR1[2]
	//    b. DDR4: Write ????to MR1[10:8]
	// 6. For DDR4 only: Perform an MRS command (using MRCTRL0 and MRCTRL1 registers) to write ????to
	//    MR5[8:6] to disable RTT_PARK
#ifdef DDR4
	//read mr1 @INIT3
	ddr_mmio_rd32(cfg_base + 0xdc, read_data);
	//rtt_nom = read_data[10:8];
	rtt_nom = get_bits_from_value(read_data, 10, 8);
	//bm_synp_mrw(0x1,  {read_data[15:11], 0b000, read_data[7:0]});
	mwr_temp = 0;
	mwr_temp = get_bits_from_value(read_data, 15, 11) << 11 | get_bits_from_value(read_data, 7, 0);
	bm_synp_mrw(0x1, mwr_temp);
	uartlog("disable RTT_NOM DDR4: Write ????to MR1[10:8]\n");
	//read mr5 @INIT6
	ddr_mmio_rd32(cfg_base + 0xe8, read_data);
	//rtt_park = read_data[8:6];
	rtt_park = get_bits_from_value(read_data, 8, 6);
	//bm_synp_mrw(0x5,  {read_data[15:9], 0b000, read_data[5:0]});
	mwr_temp = 0;
	mwr_temp = get_bits_from_value(read_data, 15, 9) << 9 | get_bits_from_value(read_data, 5, 0);
	uartlog("write ????to MR5[8:6] to disable RTT_PARK\n");
#endif
#ifdef DDR3
	//read mr1 @INIT3
	ddr_mmio_rd32(cfg_base + 0xdc, read_data);
	//rtt_nom = {read_data[9], read_data[6], read_data[2]};
	rtt_nom = get_bits_from_value(read_data, 9, 9) << 3 | get_bits_from_value(read_data, 6, 6) | get_bits_from_value(read_data, 2, 2);
	//bm_synp_mrw(0x1,  {read_data[15:10], 0b0, read_data[8:7], 0b0, read_data[5:3], 0b0, read_data[1:0]});
	mwr_temp = 0;
	mwr_temp = get_bits_from_value(read_data, 15, 10) << 10 | get_bits_from_value(read_data, 8, 7) << 7 | get_bits_from_value(read_data, 5, 3) << 3 | get_bits_from_value(read_data, 1, 0);
	bm_synp_mrw(0x1, mwr_temp);
	uartlog("disable RTT_NOM DDR3: Write to MR1[9], MR1[6] and MR1[2]\n");
#endif
//kc
	// 3. Set RFSHCTL3.dis_auto_refresh=1, to disable automatic refreshes
	ddr_mmio_rd32(cfg_base + 0x60, read_data);
	//read_data[0] = 0x1; //RFSHCTL3.dis_auto_refresh
	read_data = modified_bits_by_value(read_data, 1, 0, 0);
	mmio_wr32(cfg_base + 0x60, read_data);

	// 8. Perform an MRS command (using MRCTRL0 and MRCTRL1 registers) to disable the DLL. The timing of
	// this MRS is automatically handled by the uMCTL2.
	// a. DDR3: Write ????to MR1[0]
	// b. DDR4: Write ????to MR1[0]
#ifdef DDR4
	//read mr1 @INIT3
	ddr_mmio_rd32(cfg_base + 0xdc, read_data);
	//DDR4: Write ????to MR1[0]
	//bm_synp_mrw(0x1,  {read_data[15:1], 0b0});
	bm_synp_mrw(0x1, get_bits_from_value(read_data, 15, 1) << 1 | 0);
	uartlog("DDR4: Write ????to MR1[0]\n");
#endif
#ifdef DDR3
	//read mr1 @INIT3
	ddr_mmio_rd32(cfg_base + 0xdc, read_data);
	//DDR3: Write ????to MR1[0]
	//bm_synp_mrw(0x1,  {read_data[15:1], 0b1});
	bm_synp_mrw(0x1, get_bits_from_value(read_data, 15, 1) << 1 | 1);
#endif
	// 9. Put the SDRAM into self-refresh mode by setting PWRCTL.selfref_sw = 1, and polling STAT.operating_
	// mode to ensure the DDRC has entered self-refresh.
	// Write 1 to PWRCTL.selfref_sw
	ddr_mmio_rd32(cfg_base + 0x30, read_data);
	//read_data[5] = 0x1; //PWRCTL.selfref_sw
	//read_data[3] = 0x1; //PWRCTL.en_dfi_dram_clk_disable
	//read_data[1] = 0x0; //PWRCTL.powerdown_en
	//read_data[0] = 0x0; //PWRCTL.selfref_en
	read_data = modified_bits_by_value(read_data, 1, 5, 5);
	read_data = modified_bits_by_value(read_data, 0, 3, 3);
	read_data = modified_bits_by_value(read_data, 0, 1, 1);
	read_data = modified_bits_by_value(read_data, 0, 0, 0);
	mmio_wr32(cfg_base + 0x30, read_data);
	// Poll STAT.selfref_type= 2?�b10
	// Poll STAT.selfref_state = 0b10 (LPDDR4 only)
#ifndef LP4
	while (1) {
		ddr_mmio_rd32(cfg_base + 0x4, read_data);
		if (get_bits_from_value(read_data, 5, 4) == 2)
			break;
	}
	//11. Set the MSTR.dll_off_mode = 1.
	ddr_mmio_rd32(cfg_base + 0x0, read_data);
	//read_data[15] = 0x1;
	read_data = modified_bits_by_value(read_data, 1, 15, 15);
	mmio_wr32(cfg_base + 0x0, read_data);
#else
	while (1) {
		ddr_mmio_rd32(cfg_base + 0x4, read_data);
		if (get_bits_from_value(read_data, 9, 8) == 2)
			break;
	}
#endif
	// #ifdef DDR3
	// //param_phyd_dll_sw_code_mode    <= `PI_SD int_regin[8];
	// REGREAD(169 + PHY_BASE_ADDR, rddata);
	// rddata[8] = 0b0;
	// REGWR  (169 + PHY_BASE_ADDR, rddata, 0);
	// #endif
	//Change PLL frequency
	chg_pll_freq();
	dll_cal();
	// refresh requirement
	// Program controller
	ddr_mmio_rd32(cfg_base + 0x64, read_data);
	//read_data[27:16] = {read_data[26:16], 0b0};
	//read_data[9:0] = {read_data[8:0], 0b0};
	read_data = modified_bits_by_value(read_data, get_bits_from_value(read_data, 26, 16) << 1, 27, 16);
	mmio_wr32(cfg_base + 0x64, read_data);
	ddr_mmio_rd32(cfg_base + 0x68, read_data);
	//read_data[23:16] = {read_data[22:16], 0b0};
	read_data = modified_bits_by_value(read_data, get_bits_from_value(read_data, 22, 16) << 1, 23, 16);
	mmio_wr32(cfg_base + 0x68, read_data);
	// program phyupd_mask
	ddr_mmio_rd32(0x0800a504, read_data);
	//read_data[27:16] = {read_data[26:16],0b0};
	read_data = modified_bits_by_value(read_data, get_bits_from_value(read_data, 26, 16) << 1, 27, 16);
	mmio_wr32(0x0800a504, read_data);
	ddr_mmio_rd32(0x0800a500, read_data);
	//read_data[31:16] = {read_data[30:16],0b0};
	read_data = modified_bits_by_value(read_data, get_bits_from_value(read_data, 30, 16) << 1, 31, 16);
	mmio_wr32(0x0800a500, read_data);
#ifndef LP4
	//9. Set MSTR.dll_off_mode = 0
	ddr_mmio_rd32(cfg_base + 0x0, read_data);
	//read_data[15] = 0x0;
	read_data = modified_bits_by_value(read_data, 0, 15, 15);
	mmio_wr32(cfg_base + 0x0, read_data);
#endif
	ddr_mmio_rd32(cfg_base + 0x30, read_data);
	read_data = modified_bits_by_value(read_data, 0, 5, 5);
	read_data = modified_bits_by_value(read_data, 0, 3, 3);
	read_data = modified_bits_by_value(read_data, 0, 1, 1);
	read_data = modified_bits_by_value(read_data, 0, 0, 0);
	mmio_wr32(cfg_base + 0x30, read_data);
	while (1) {
		ddr_mmio_rd32(cfg_base + 0x4, read_data);
		if (get_bits_from_value(read_data, 5, 4) == 0)
			break;
	}
	while (1) {
		ddr_mmio_rd32(cfg_base + 0x4, read_data);
		if (get_bits_from_value(read_data, 1, 0) == 1)
			break;
	}
#ifdef DDR4
	ddr_mmio_rd32(cfg_base + 0xdc, read_data);
	bm_synp_mrw(0x1, get_bits_from_value(read_data, 15, 1) << 1 | 1);
	uartlog("DDR4: Write 1 to MR1[0]\n");
	ddr_mmio_rd32(cfg_base + 0xdc, read_data);
	bm_synp_mrw(0x0, get_bits_from_value(read_data, 31, 25) << 9 | 0b1 << 8 | get_bits_from_value(read_data, 23, 16));
	uartlog("15. Perform an MRS command\n");
	ddr_mmio_rd32(cfg_base + 0xdc, read_data);
	bm_synp_mrw(0x1, get_bits_from_value(15, 11) << 11 | get_bits_from_value(rtt_nom, 2, 0) << 8 | get_bits_from_value(read_data, 7, 0));
	uartlog("Re-enable RTT_NOM\n");
	ddr_mmio_rd32(cfg_base + 0xe8, read_data);
	bm_synp_mrw(0x5, get_bits_from_value(read_data, 15, 9) << 9 | get_bits_from_value(rtt_park, 2, 0) << 6 | get_bits_from_value(read_data, 5, 0));
	uartlog("Re-enable RTT_PARK\n");
#endif
#ifdef DDR3
	ddr_mmio_rd32(cfg_base + 0xdc, read_data);
	bm_synp_mrw(0x1, get_bits_from_value(read_data, 15, 10) << 10 | get_bits_from_value(rtt_nom, 2, 2) << 9 | get_bits_from_value(read_data, 8, 7) << 7 | get_bits_from_value(rtt_nom, 1, 1) << 5 | get_bits_from_value(read_data, 5, 3) << 3 | get_bits_from_value(rtt_nom, 0, 0) << 2 | get_bits_from_value(read_data, 1, 1) << 1 | 0b0);
	uartlog("Re-enable RTT_NOM\n");
	ddr_mmio_rd32(cfg_base + 0xdc, read_data);
	bm_synp_mrw(0x0, get_bits_from_value(read_data, 31, 25) << 9 | 0b1 << 8 | get_bits_from_value(read_data, 23, 16));
	opdelay(600); //don't remove. wait tDLLK ??

#endif
	ddr_mmio_rd32(cfg_base + 0x60, read_data);
	read_data = modified_bits_by_value(read_data, 0, 0, 0);
	mmio_wr32(cfg_base + 0x60, read_data);
	// Related to control power
	ddr_mmio_rd32(cfg_base + 0x30, read_data);
	read_data = modified_bits_by_value(read_data, 0, 5, 5);
	// read_data = modified_bits_by_value(read_data, en_dfi_dram_clk_disable, 3, 3);
	read_data = modified_bits_by_value(read_data, 0, 3, 3);
	// read_data = modified_bits_by_value(read_data, powerdown_en, 1, 1);
	read_data = modified_bits_by_value(read_data, 0, 1, 1);
	// read_data = modified_bits_by_value(read_data, selfref_en, 0, 0);
	read_data = modified_bits_by_value(read_data, 0, 0, 0);
	mmio_wr32(cfg_base + 0x30, read_data);
	uartlog("restore selfref_en powerdown_en\n");
#ifndef DDR3
	uartlog("training\n");
#ifdef LP4
	ddr_mmio_rd32(4 * (197 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	rddata = modified_bits_by_value(rddata, 0x400, 12, 0); //
	mmio_wr32(4 * (197 + PHY_BASE_ADDR) + CADENCE_PHYD, rddata);
	pi_wrlvl_req();
	rdgtlvl();
	// pi_rdlvl_gate_req();

	// set_rx_rdvld
	mmio_wr32(4*(105 + PHY_BASE_ADDR)+CADENCE_PHYD, 0x0f0f0f0f);

	pi_rdlvl_req();

	pi_wdqlvl_req();
#else
	pi_rdlvl_gate_req();
#endif
#endif
	for (int i = 0; i < 8; i++)
		mmio_wr32(cfg_base + 0x490 + 0xb0 * i, 0x1);

	ddr_mmio_rd32(0x4c + CADENCE_PHYD_APB, apb_rddata);
	apb_rddata = modified_bits_by_value(apb_rddata, 0, 0, 0);
	mmio_wr32(0x4c + CADENCE_PHYD_APB, apb_rddata);
}

#if 0
void bm_bist_wr_sram_init(void)
{
	#define DDR_BIST_SRAM_DQ_BASE   0x08011000
	#define DDR_BIST_SRAM_DM_BASE   0x08011800

	uint32_t rddata;

	// variable user should specified
	uint8_t sram_dq[] = {
		0x01, 0xFE, 0x01, 0xFE, 0x01, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x02, 0xFD, 0x02, 0xFD, 0x02, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x04, 0xFB, 0x04, 0xFB,
		0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0xF7,
		0x08, 0xF7, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x10, 0xEF, 0x10, 0xEF, 0x10, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x20, 0xDF, 0x20, 0xDF, 0x20, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x40, 0xBF, 0x40, 0xBF,
		0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x7F,
		0x80, 0x7F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x01, 0xFE, 0x01, 0xFE, 0x01, 0xFE, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x02, 0xFD, 0x02, 0xFD, 0x02,
		0xFD, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0xFB,
		0x04, 0xFB, 0x04, 0xFB, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x08, 0xF7, 0x08, 0xF7, 0x08, 0xF7, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x10, 0xEF, 0x10, 0xEF,
		0x10, 0xEF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20,
		0xDF, 0x20, 0xDF, 0x20, 0xDF, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x40, 0xBF, 0x40, 0xBF, 0x40, 0xBF,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x7F, 0x80,
		0x7F, 0x80, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x01, 0xFE, 0x01, 0xFE, 0x01, 0xFE, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x02, 0xFD, 0x02, 0xFD,
		0x02, 0xFD, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x04, 0xFB, 0x04, 0xFB, 0x04, 0xFB, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x08, 0xF7, 0x08, 0xF7,
		0x08, 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x10, 0xEF, 0x10, 0xEF, 0x10, 0xEF, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x20, 0xDF, 0x20, 0xDF,
		0x20, 0xDF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x40, 0xBF, 0x40, 0xBF, 0x40, 0xBF, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x80, 0x7F, 0x80, 0x7F,
		0x80, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	};

	uint32_t sram_st;
	uint32_t sram_sp;

	uint32_t cmd[6];
	int i;

	// sram_st = 0;
	// sram_sp = (sizeof(sram_dq) / 4) - 1;
	// uartlog("sram_sp=%d\n", sram_sp);
	// cmd queue
	//        op_code         start              stop        pattern    dq_inv     dm_inv    dq_rotate   repeat
	// cmd[0] = (1 << 30) | (sram_st << 21) | (sram_sp << 12) | (0 << 9) | (0 << 8) | (0 << 7) | (0 << 4) | (0 << 0); // W  sram  repeat0
	// cmd[1] = (2 << 30) | (sram_st << 21) | (sram_sp << 12) | (0 << 9) | (0 << 8) | (0 << 7) | (0 << 4) | (0 << 0); // R  sram  repeat0
	// cmd[2] = 0; // NOP
	// cmd[3] = 0; // NOP
	// cmd[4] = 0; // NOP
	// cmd[5] = 0; // NOP

	// cmd queue
	sram_st = 0;
	sram_sp = 511;
	//        op_code         start              stop        pattern    dq_inv     dm_inv    dq_rotate   repeat
	cmd[0] = (1 << 30) | (sram_st << 21) | (sram_sp << 12) | (5 << 9) | (0 << 8) | (0 << 7) | (0 << 4) | (0xf << 0);  // W  1~17  prbs  repeat0
	cmd[1] = (2 << 30) | (sram_st << 21) | (sram_sp << 12) | (5 << 9) | (0 << 8) | (0 << 7) | (0 << 4) | (0xf << 0);  // R  1~17  prbs  repeat0
	sram_st = 0;
	sram_sp = (sizeof(sram_dq) / 4) - 1;
	//        op_code         start              stop        pattern    dq_inv     dm_inv    dq_rotate   repeat
	cmd[2] = (1 << 30) | (sram_st << 21) | (sram_sp << 12) | (0 << 9) | (0 << 8) | (0 << 7) | (0 << 4) | (0xf << 0);  // W  sram  repeat0
	cmd[3] = (2 << 30) | (sram_st << 21) | (sram_sp << 12) | (0 << 9) | (0 << 8) | (0 << 7) | (0 << 4) | (0xf << 0);  // R  sram  repeat0
	cmd[4] = (3 << 30);  // goto
	cmd[5] = 0;  // NOP

	// write bist sram
	for (i = sram_st; i <= sram_sp; i++) {
		mmio_wr32(DDR_BIST_SRAM_DQ_BASE + i * 4, (sram_dq[4 * i + 3] << 24) | (sram_dq[4 * i + 2] << 16) | (sram_dq[4 * i + 1] << 8) | (sram_dq[4 * i + 0] << 0));
		mmio_wr32(DDR_BIST_SRAM_DM_BASE + i * 4, 0x0f);
	}

	// write cmd queue
	for (i = 0; i < 6; i = i + 1) {
		mmio_wr32(DDR_BIST_BASE + 0x40 + i * 4, cmd[i]);
	};

	// specified DDR space
	mmio_wr32(DDR_BIST_BASE + 0x10, 0x00100000);
	mmio_wr32(DDR_BIST_BASE + 0x14, 0x001fffff);

	// bist enable
	mmio_wr32(DDR_BIST_BASE + 0x0, 0x00010001);
	info("bist start\n");

	// polling bist done
	do {
		rddata = mmio_rd32(DDR_BIST_BASE + 0x80);
		info("reg[%x]=%x\n", DDR_BIST_BASE + 0x80, rddata);
	} while (rddata == 0);
	// } while ((rddata & 0x4) == 0);
	// info("bist done reg[0x%08x]=0x%08x\n", DDR_BIST_BASE + 0x80, rddata);

	//check bist result
	if (rddata & 0x08) {
		info("bist fail\n");
		if (get_bits_from_value(rddata, 3, 3) == 1) {
			uint32_t err_data_odd_l;
			uint32_t err_data_odd_h;
			uint32_t err_data_even_l;
			uint32_t err_data_even_h;

			opdelay(10);
			// read err_data
			err_data_odd_l = mmio_rd32(DDR_BIST_BASE + 0x88);
			err_data_odd_h = mmio_rd32(DDR_BIST_BASE + 0x8c);
			err_data_even_l = mmio_rd32(DDR_BIST_BASE + 0x90);
			err_data_even_h = mmio_rd32(DDR_BIST_BASE + 0x94);
			info("err_data_odd(h/l)=%x/%x\n", err_data_odd_h, err_data_odd_l);
			info("err_data_even(h/l)=%x/%x\n", err_data_even_h, err_data_even_l);
		} else {
			info("bist pass\n");
		}
	}

	// bist disable
	mmio_wr32(DDR_BIST_BASE + 0x0, 0x00010000);
}
#endif
