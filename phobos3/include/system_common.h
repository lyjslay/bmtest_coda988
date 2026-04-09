#ifndef _SYSTEM_COMMON_H_
#define _SYSTEM_COMMON_H_

#include <stdio.h>

#include "bmtest_type.h"
#include "fw_config.h"

#ifdef __riscv
#include "phobosrv_common.h"
#else
#include "phobos_common.h"
#endif

#include "phobos_ip_rst.h"
#include "phobos_pinlist_swconfig.h"
#include "phobos_reg_fmux_gpio.h"

static inline u32 float_to_u32(float x)
{
  union {
    int ival;
    float fval;
  } v = { .fval = x };
  return v.ival;
}

static inline float u32_to_float(u32 x)
{
  union {
    int ival;
    float fval;
  } v = { .ival = x };
  return v.fval;
}

#define array_len(a)              (sizeof(a) / sizeof(a[0]))

#define ALIGNMENT(x, a)			  __ALIGNMENT_MASK((x), (typeof(x))(a)-1)
#define __ALIGNMENT_MASK(x, mask)	(((x)+(mask))&~(mask))
#define PTR_ALIGNMENT(p, a)		 ((typeof(p))ALIGNMENT((unsigned long)(p), (a)))
#define IS_ALIGNMENT(x, a)		(((x) & ((typeof(x))(a) - 1)) == 0)

#define __raw_readb(a)		  (*(volatile unsigned char *)(a))
#define __raw_readw(a)		  (*(volatile unsigned short *)(a))
#define __raw_readl(a)		  (*(volatile unsigned int *)(a))
#define __raw_readq(a)		  (*(volatile unsigned long long *)(a))

#define __raw_writeb(a,v)	   (*(volatile unsigned char *)(a) = (v))
#define __raw_writew(a,v)	   (*(volatile unsigned short *)(a) = (v))
#define __raw_writel(a,v)	   (*(volatile unsigned int *)(a) = (v))
#define __raw_writeq(a,v)	   (*(volatile unsigned long long *)(a) = (v))

#define readb(a)		__raw_readb(a)
#define readw(a)		__raw_readw(a)
#define readl(a)		__raw_readl(a)
#define readq(a)		__raw_readq(a)

#define writeb(a, v)		__raw_writeb(a,v)
#define writew(a, v)		__raw_writew(a,v)
#define writel(a, v)		__raw_writel(a,v)
#define writeq(a, v)		__raw_writeq(a,v)

#define cpu_write8(a, v)	writeb(a, v)
#define cpu_write16(a, v)	writew(a, v)
#define cpu_write32(a, v)	writel(a, v)

#define cpu_read8(a)		readb(a)
#define cpu_read16(a)		readw(a)
#define cpu_read32(a)		readl(a)

#ifndef __weak
#define __weak __attribute__((weak))
#endif

#ifdef ENABLE_DEBUG
#define debug(fmt, args...)	printf(fmt, ##args)
#else
#define debug(...)
#endif

#ifdef ENABLE_PRINT
#define uartlog(fmt, args...)	printf(fmt, ##args)
#else
#define uartlog(...)
#endif

#if 0
extern u32 debug_level;
#define debug_out(flag, fmt, args...)           \
  do {                                          \
    if (flag <= debug_level)                    \
      printf(fmt, ##args);                      \
  } while (0)
#endif

static inline void opdelay(unsigned int times)
{
	while (times--)
		__asm__ volatile("nop");
}

#ifdef USE_BMTAP
#define call_atomic(nodechip_idx, atomic_func, p_command, eng_id)       \
  emit_task_descriptor(p_command, eng_id)
#endif

#define TOP_USB_PHY_CTRSTS_REG	(TOP_BASE + 0x48)
#define UPCR_EXTERNAL_VBUS_VALID_OFFSET		0

#define TOP_DDR_ADDR_MODE_REG	(TOP_BASE + 0x64)
#define DAMR_REG_USB_REMAP_ADDR_39_32_OFFSET	16
#define DAMR_REG_USB_REMAP_ADDR_39_32_MSK		(0xff)

#define DAMR_REG_VD_REMAP_ADDR_39_32_OFFSET  24
#define DAMR_REG_VD_REMAP_ADDR_39_32_MSK   (0xff)

#define SW_RESET  (TOP_BASE + 0x3000)
#define JPEG_RESET   4

#define TOP_USB_CTRSTS_REG		(TOP_BASE + 0x38)
#define UCR_MODE_STRAP_OFFSET	0
#define UCR_MODE_STRAP_NON		0x0
#define UCR_MODE_STRAP_HOST		0x2
#define UCR_MODE_STRAP_DEVICE	0x4
#define UCR_MODE_STRAP_MSK		(0x7)
#define UCR_PORT_OVER_CURRENT_ACTIVE_OFFSET		10
#define UCR_PORT_OVER_CURRENT_ACTIVE_MSK		1

#define PINMUX_UART0    0
#define PINMUX_UART1    1
#define PINMUX_UART2    2
#define PINMUX_UART3    3
#define PINMUX_UART3_2  4
#define PINMUX_I2C0     5
#define PINMUX_I2C1     6
#define PINMUX_I2C2     7
#define PINMUX_I2C3     8
#define PINMUX_I2C4     9
#define PINMUX_I2C4_2   10
#define PINMUX_SPI0     11
#define PINMUX_SPI1     12
#define PINMUX_SPI2     13
#define PINMUX_SPI2_2   14
#define PINMUX_SPI3     15
#define PINMUX_SPI3_2   16
#define PINMUX_I2S0     17
#define PINMUX_I2S1     18
#define PINMUX_I2S2     19
#define PINMUX_I2S3     20
#define PINMUX_USBID    21
#define PINMUX_SDIO0    22
#define PINMUX_SDIO1    23
#define PINMUX_ND       24
#define PINMUX_EMMC     25
#define PINMUX_SPI_NOR  26
#define PINMUX_SPI_NAND 27
#define PINMUX_CAM0     28
#define PINMUX_CAM1     29
#define PINMUX_PCM0     30
#define PINMUX_PCM1     31
#define PINMUX_CSI0     32
#define PINMUX_CSI1     33
#define PINMUX_CSI2     34
#define PINMUX_DSI      35
#define PINMUX_VI0      36
#define PINMUX_VO       37
#define PINMUX_PWM1     38
#define PINMUX_UART4    39
#define PINMUX_SPI_NOR1 40

/* addr remap */
#define REG_TOP_ADDR_REMAP      0x0064
#define ADDR_REMAP_USB(a)       ((a&0xFF)<<16)

/* rst */
#define BITS_PER_REG 32
#define REG_TOP_SOFT_RST        0x3000

/* irq */
#define IRQ_LEVEL   0
#define IRQ_EDGE    3

#define NA 0xFFFF
#ifdef __riscv // C906B
#ifdef CONFIG_C906B
#define TEMPSEN_IRQ_O 16
#define RTC_ALARM_O 17
#define RTC_PWR_BUTTON1_LONGPRESS_O 18
#define VBAT_DEB_IRQ_O 19
#define JPEG_INTERRUPT 20
#define H264_INTERRUPT 21
#define H265_INTERRUPT 22
#define VC_SBM_INT 23
#define ISP_INT 24
#define SC_INTR_0 25
#define VIP_INT_CSI_MAC0 26
#define VIP_INT_CSI_MAC1 27
#define LDC_INT 28
#define SDMA_INTR_CPU0 NA
#define SDMA_INTR_CPU1 29
#define SDMA_INTR_CPU2 NA
#define USB_IRQS 30
#define ETH0_SBD_INTR_O 31
#define ETH0_LPI_INTR_O 32
#define EMMC_WAKEUP_INTR 33
#define EMMC_INTR 34
#define SD0_WAKEUP_INTR 35
#define SD0_INTR 36
#define SD1_WAKEUP_INTR 37
#define SD1_INTR 38
#define SPI_NAND_INTR 39
#define I2S0_INT 40
#define I2S1_INT 41
#define I2S2_INT 42
#define I2S3_INT 43
#define UART0_INTR 44
#define UART1_INTR 45
#define UART2_INTR 46
#define UART3_INTR 47
#define UART4_INTR 48
#define I2C0_INTR 49
#define I2C1_INTR 50
#define I2C2_INTR 51
#define I2C3_INTR 52
#define I2C4_INTR 53
#define SPI_0_SSI_INTR 54
#define SPI_1_SSI_INTR 55
#define SPI_2_SSI_INTR 56
#define SPI_3_SSI_INTR 57
#define WDT0_INTR NA
#define WDT1_INTR 58
#define WDT2_INTR NA
#define KEYSCAN_IRQ 59
#define GPIO0_INTR_FLAG 60
#define GPIO1_INTR_FLAG 61
#define GPIO2_INTR_FLAG 62
#define GPIO3_INTR_FLAG 63
#define WGN0_IRQ 64
#define WGN1_IRQ 65
#define WGN2_IRQ 66
#define MBOX_INT1 67
// #define NA 68
#define IRRX_INT 69
#define GPIO_INT 70
#define UART_INT 71
#define SF1_SPI_INT 72
#define I2C_INT 73
#define WDT_INT 74
#define TPU_INTR 75
#define TDMA_INTERRUPT 76
#define SW_INT_0_CPU0 NA
#define SW_INT_1_CPU0 NA
#define SW_INT_0_CPU1 77
#define SW_INT_1_CPU1 78
#define SW_INT_0_CPU2 NA
#define SW_INT_1_CPU2 NA
#define TIMER_INTR_0 79
#define TIMER_INTR_1 80
#define TIMER_INTR_2 81
#define TIMER_INTR_3 82
#define TIMER_INTR_4 83
#define TIMER_INTR_5 84
#define TIMER_INTR_6 85
#define TIMER_INTR_7 86
#define PERI_FIREWALL_IRQ 87
#define HSPERI_FIREWALL_IRQ 88
#define DDR_FW_INTR 89
#define ROM_FIREWALL_IRQ 90
#define SPACC_IRQ 91
#define TRNG_IRQ 92
#define AXI_MON_INTR 93
#define DDRC_PI_PHY_INTR 94
#define SF_SPI_INT 95
#define EPHY_INT_N_O 96
#define IVE_INT 97
// #define NA 98
#define DBGSYS_APBUSMON_HANG_INT 99
#define INTR_SARADC 100
// #define NA NA
// #define NA NA
// #define NA NA
#define MBOX_INT_CA53 NA
#define MBOX_INT_C906 101
#define MBOX_INT_C906_2ND NA
#define NPMUIRQ_0 NA
#define CTIIRQ_0 NA
#define NEXTERRIRQ NA

#elif defined CONFIG_C906L //C906L
#define TEMPSEN_IRQ_O NA
#define RTC_ALARM_O NA
#define RTC_PWR_BUTTON1_LONGPRESS_O NA
#define VBAT_DEB_IRQ_O NA
#define JPEG_INTERRUPT 16
#define H264_INTERRUPT 17
#define H265_INTERRUPT 18
#define VC_SBM_INT 19
#define ISP_INT 20
#define SC_INTR_0 21
#define VIP_INT_CSI_MAC0 22
#define VIP_INT_CSI_MAC1 23
#define LDC_INT 24
#define SDMA_INTR_CPU0 NA
#define SDMA_INTR_CPU1 NA
#define SDMA_INTR_CPU2 25
#define USB_IRQS NA
#define ETH0_SBD_INTR_O NA
#define ETH0_LPI_INTR_O NA
#define EMMC_WAKEUP_INTR NA
#define EMMC_INTR NA
#define SD0_WAKEUP_INTR NA
#define SD0_INTR NA
#define SD1_WAKEUP_INTR NA
#define SD1_INTR NA
#define SPI_NAND_INTR NA
#define I2S0_INT 26
#define I2S1_INT 27
#define I2S2_INT 28
#define I2S3_INT 29
#define UART0_INTR 30
#define UART1_INTR 31
#define UART2_INTR NA
#define UART3_INTR NA
#define UART4_INTR NA
#define I2C0_INTR 32
#define I2C1_INTR 33
#define I2C2_INTR 34
#define I2C3_INTR 35
#define I2C4_INTR 36
#define SPI_0_SSI_INTR 37
#define SPI_1_SSI_INTR 38
#define SPI_2_SSI_INTR NA
#define SPI_3_SSI_INTR NA
#define WDT0_INTR NA
#define WDT1_INTR NA
#define WDT2_INTR 39
#define KEYSCAN_IRQ 40
#define GPIO0_INTR_FLAG 41
#define GPIO1_INTR_FLAG 42
#define GPIO2_INTR_FLAG 43
#define GPIO3_INTR_FLAG 44
#define WGN0_IRQ 45
#define WGN1_IRQ NA
#define WGN2_IRQ NA
#define MBOX_INT1 46
// #define NA NA
#define IRRX_INT 47
#define GPIO_INT 48
#define UART_INT 49
#define SPI_INT NA
#define I2C_INT 50
#define WDT_INT 51
#define TPU_INTR NA
#define TDMA_INTERRUPT 52
#define SW_INT_0_CPU0 NA
#define SW_INT_1_CPU0 NA
#define SW_INT_0_CPU1 NA
#define SW_INT_1_CPU1 NA
#define SW_INT_0_CPU2 53
#define SW_INT_1_CPU2 54
#define TIMER_INTR_0 NA
#define TIMER_INTR_1 NA
#define TIMER_INTR_2 NA
#define TIMER_INTR_3 NA
#define TIMER_INTR_4 55
#define TIMER_INTR_5 56
#define TIMER_INTR_6 57
#define TIMER_INTR_7 58
#define PERI_FIREWALL_IRQ NA
#define HSPERI_FIREWALL_IRQ NA
#define DDR_FW_INTR NA
#define ROM_FIREWALL_IRQ NA
#define SPACC_IRQ 59
#define TRNG_IRQ NA
#define AXI_MON_INTR NA
#define DDRC_PI_PHY_INTR NA
#define SF_SPI_INT NA
#define EPHY_INT_N_O NA
#define IVE_INT 60
// #define NA NA
#define DBGSYS_APBUSMON_HANG_INT NA
#define INTR_SARADC NA
// #define NA NA
// #define NA NA
// #define NA NA
#define MBOX_INT_CA53 NA
#define MBOX_INT_C906 NA
#define MBOX_INT_C906_2ND 61
#define NPMUIRQ_0 NA
#define CTIIRQ_0 NA
#define NEXTERRIRQ NA
#endif

#else // CA53
//No interrupt definition for CA53 in Phobos
#endif

/* irqf */
#define IRQF_TRIGGER_NONE    0x00000000
#define IRQF_TRIGGER_RISING  0x00000001
#define IRQF_TRIGGER_FALLING 0x00000002
#define IRQF_TRIGGER_HIGH    0x00000004
#define IRQF_TRIGGER_LOW     0x00000008
#define IRQF_TRIGGER_MASK   (IRQF_TRIGGER_HIGH | IRQF_TRIGGER_LOW | \
		                 IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING)

// IRQ API
typedef int (*irq_handler_t)(int irqn, void *priv);

extern int request_irq(unsigned int irqn, irq_handler_t handler,
		       unsigned long flags, const char *name, void *priv);

void disable_irq(unsigned int irqn);
void enable_irq(unsigned int irqn);

void cpu_enable_irqs(void);
void cpu_disable_irqs(void);

extern void irq_trigger(int irqn);
extern void irq_clear(int irqn);
extern int irq_get_nums(void);

// PINMUX API
void pinmux_config(int io_type);

// RESET API
void cv_reset_assert(uint32_t id);
void cv_reset_deassert(uint32_t id);

#define NUM_IRQ (256)

#ifdef __riscv
// cache operation api
void dcache_disable(void);
void dcache_enable(void);
void inv_dcache_all(void);
void clean_dcache_all(void);
void inv_clean_dcache_all(void);
#endif


#endif
