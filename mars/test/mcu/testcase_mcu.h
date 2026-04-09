#ifndef _TESTCASE_MCU_H_
#define _TESTCASE_MCU_H_

#define DEFINE_REG(BASE, NAME, OFFSET, MSB, LSB)                                                   \
	const uintptr_t NAME##_ADDR = (BASE + OFFSET);                                             \
	const int NAME##_MSB = (MSB);                                                              \
	const int NAME##_LSB = (LSB);

#define REG_READ(NAME) get_bits_from_value(mmio_read_32(NAME##_ADDR), NAME##_MSB, NAME##_LSB)
#define REG_WRITE(NAME, value)                                                                     \
	mmio_write_32(NAME##_ADDR, modified_bits_by_value(mmio_read_32(NAME##_ADDR), value,        \
							  NAME##_MSB, NAME##_LSB))

#define RTC_SRAM_BASE 0x05200000
// #define RTC_SRAM_SIZE 0x2000 // 8KB
#define RTC_SRAM_SIZE 0x6000 // 24KB

#define RTC_CORE_SRAM_BASE 0x05026800
#define RTC_CORE_SRAM_SIZE 0x0800 // 2KB

/*
 * Regsiters
 */
#define RTC_CTRL_BASE 0x05025000

/*
 * reg_soft_rstn_mcu (0x18[1])
 * 1: de-assert mcu reset
 * 0: assert mcu reset (default)
 */
DEFINE_REG(RTC_CTRL_BASE, REG_SOFT_RSTN_MCU, 0x18, 1, 1)

/*
 * reg_51_rom_addr_size   (0x05025020[4:0])
 * reg_51_mem_ea_n        (0x05025020[5])
 * 	//(0 :use external rom, 1 : use internal rom)
 * reg_51irom_ioffset[4:0]  (0x05025020[11:8])
 * 	// Set offset address[15:12]  to select mcu8051 irom boot address
 *
 * reg_51xdata_ioffset0       (0x05025020[31:12])
 * 	//Add offset of mcu xdata address[31:12] for boot device
 * reg_51xdata_ioffset1       (0x05025024[15:0])
 * 	//Add offset of mcu xdata address[31:20] for boot device
 */
DEFINE_REG(RTC_CTRL_BASE, REG_51_ROM_ADDR_SIZE, 0x20, 4, 0)
DEFINE_REG(RTC_CTRL_BASE, REG_51_MEM_EA_N, 0x20, 5, 5)
DEFINE_REG(RTC_CTRL_BASE, REG_51IROM_IOFFSET, 0x20, 11, 8)
DEFINE_REG(RTC_CTRL_BASE, REG_51XDATA_IOFFSET0, 0x20, 31, 12)
DEFINE_REG(RTC_CTRL_BASE, REG_51XDATA_IOFFSET1, 0x24, 15, 0)

/*
 * h05c h05c rtcsys_mcu51_dbg rstn reg_51_dbg_sel       3  0
 * -    -    -                rstn reg_51_dbg_snap_shot 4  4
 * -    -    -                rstn reg_51_dbg_step_en   5  5
 * -    -    -                rstn reg_51_dbg_step      6  6
 * -    -    -                rstn reg_51_dbg_jump      7  7
 * -    -    -                rstn reg_51_dbg_jump2pc   31 16
 */
DEFINE_REG(RTC_CTRL_BASE, reg_51_dbg_sel, 0x5C, 3, 0)
DEFINE_REG(RTC_CTRL_BASE, reg_51_dbg_snap_shot, 0x5C, 4, 4)
DEFINE_REG(RTC_CTRL_BASE, reg_51_dbg_step_en, 0x5C, 5, 5)
DEFINE_REG(RTC_CTRL_BASE, reg_51_dbg_step, 0x5C, 6, 6)
DEFINE_REG(RTC_CTRL_BASE, reg_51_dbg_jump, 0x5C, 7, 7)
DEFINE_REG(RTC_CTRL_BASE, reg_51_dbg_jump2pc, 0x5C, 31, 16)

int testcase_mcu(void);

#endif /* _TESTCASE_MCU_H_ */
