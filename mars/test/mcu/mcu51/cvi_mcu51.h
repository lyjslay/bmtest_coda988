#ifndef CVI_MCU51_H_ILTCKRDQ
#define CVI_MCU51_H_ILTCKRDQ

#include "config.h"

typedef unsigned long uint32_t;
typedef unsigned int uint16_t;
typedef unsigned char uint8_t;

typedef signed long int32_t;
typedef signed int int16_t;
typedef signed char int8_t;

#define AS_BYTES(x) ((uint8_t *)&(x))

#define REG_READ(NAME) get_bits_from_value(mmio_read_32(NAME##_ADDR), NAME##_MSB, NAME##_LSB)
#define REG_WRITE(NAME, value)                                                                     \
	mmio_write_32(NAME##_ADDR, modified_bits_by_value(mmio_read_32(NAME##_ADDR), value,        \
							  NAME##_MSB, NAME##_LSB)

/*
 * reg_51robot_rd [31:0]	0xE7~E4	Robot read data
 * reg_51robot_fire	0xF2	Robot fire = 1, auto set low when finish
 * reg_51robot_we	0xF3[0]	Robot write enable
 * reg_51robot_size [1:0]	0xF3[2:1]	Robot r/w size
 * reg_51robot_wd [31:0]	0xF7~F4	Robot write data
 * reg_51robot_adr [31:0]	0xFB~F8	Robot address
 * reg_51_code_banking[5:0]	0xFE[5:0]	Code banking register (for xdata)
 * reg_51xdata_doffset[15:0]	0xFD~FC	xdata data address offset (for xdata)
 */

sfr r51_rd0 = 0xE4;
sfr r51_rd1 = 0xE5;
sfr r51_rd2 = 0xE6;
sfr r51_rd3 = 0xE7;

sfr r51_fire = 0xF2;
sfr r51_we = 0xF3;

sfr r51_wd0 = 0xF4;
sfr r51_wd1 = 0xF5;
sfr r51_wd2 = 0xF6;
sfr r51_wd3 = 0xF7;

sfr r51_adr0 = 0xF8;
sfr r51_adr1 = 0xF9;
sfr r51_adr2 = 0xFA;
sfr r51_adr3 = 0xFB;

sfr r51_code_banking = 0xF7;

sfr r51_xdata_doffset0 = 0xFC;
sfr r51_xdata_doffset1 = 0xFD;

/*
 *
 */
extern unsigned char xdata xs1;
extern unsigned char xdata xs2;
extern unsigned char xdata xs3;
extern unsigned char xdata xs4;

uint32_t read_robot(uint32_t addr);
void write_robot(uint32_t addr, uint32_t value);

uint32_t read_robot_irq(uint32_t addr);
uint32_t read_robot_irq1(uint32_t addr);
void write_robot_irq(uint32_t addr, uint32_t value);

#endif /* end of include guard: CVI_MCU51_H_ILTCKRDQ */
