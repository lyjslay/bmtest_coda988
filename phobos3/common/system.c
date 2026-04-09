#include <stdint.h>
#include <string.h>
#include "system_common.h"
#include "uart.h"
#include "mmio.h"
#include "bitwise_ops.h"
#include "phobos_reg_fmux_gpio.h"
#include "phobos_pinlist_swconfig.h"
#ifdef __riscv
#include "asm/csr.h"
#include "phobosrv_common.h"
#endif

extern void system_init(void);
void irq_init(void);


// #define PINMUX_CONFIG(PIN_NAME, FUNC_NAME) printf ("%s\n", PIN_NAME ##_ ##FUNC_NAME);
#define PINMUX_MASK(PIN_NAME) FMUX_GPIO_FUNCSEL_##PIN_NAME##_MASK
#define PINMUX_OFFSET(PIN_NAME) FMUX_GPIO_FUNCSEL_##PIN_NAME##_OFFSET
#define PINMUX_VALUE(PIN_NAME, FUNC_NAME) PIN_NAME##__##FUNC_NAME
#define PINMUX_CONFIG(PIN_NAME, FUNC_NAME) \
		mmio_clrsetbits_32(PINMUX_BASE + FMUX_GPIO_FUNCSEL_##PIN_NAME, \
						   PINMUX_MASK(PIN_NAME) << PINMUX_OFFSET(PIN_NAME), \
						   PINMUX_VALUE(PIN_NAME, FUNC_NAME))

void pinmux_config(int io_type)
{
	switch(io_type) {
		case PINMUX_UART0:
			// default config
		break;

		case PINMUX_UART1:
			PINMUX_CONFIG(UART0_TX, UART1_TX);
			PINMUX_CONFIG(UART0_RX, UART1_RX);
			break;

		case PINMUX_UART2:
			PINMUX_CONFIG(IIC0_SCL, UART2_TX);
			PINMUX_CONFIG(IIC0_SDA, UART2_RX);
			break;

		case PINMUX_I2C0:
			PINMUX_CONFIG(IIC0_SDA, IIC0_SDA);
			PINMUX_CONFIG(IIC0_SCL, IIC0_SCL);
			break;

		case PINMUX_I2C1:
			PINMUX_CONFIG(SD1_D1, IIC1_SDA);
			PINMUX_CONFIG(SD1_D2, IIC1_SCL);
			break;

		case PINMUX_I2C3:
			PINMUX_CONFIG(SD1_CMD, IIC3_SCL);
			PINMUX_CONFIG(SD1_CLK, IIC3_SDA);
			break;
		case PINMUX_CAM0:
			PINMUX_CONFIG(SD0_D0, CAM_MCLK1);
			PINMUX_CONFIG(SD0_D3, CAM_MCLK0);
			break;
		case PINMUX_SPI0:
			PINMUX_CONFIG(SD0_CLK, SPI0_SCK);
			PINMUX_CONFIG(SD0_D3, SPI0_CS_X);
			PINMUX_CONFIG(SD0_D0, SPI0_SDI);
			PINMUX_CONFIG(SD0_CMD, SPI0_SDO);
			break;

		// case PINMUX_SPI1:
		// 	PINMUX_CONFIG(RMII0_MDCK, SPI1_SCK);
		// 	PINMUX_CONFIG(RMII0_RXDV, SPI1_CS_X);
		// 	PINMUX_CONFIG(RMII0_REFCLKI, SPI1_SDI);
		// 	PINMUX_CONFIG(RMII0_MDIO, SPI1_SDO);
		// break;
		// case PINMUX_SPI2:
		// 	PINMUX_CONFIG(IIC3_SCL, SPI2_SCK);
		// 	PINMUX_CONFIG(IIC3_SDA, SPI2_CS_X);
		// 	PINMUX_CONFIG(IIC0_SCL, SPI2_SDI);
		// 	PINMUX_CONFIG(IIC0_SDA, SPI2_SDO);
		// break;
		// case PINMUX_SPI3:
		// 	printf ("fix me\n");
		// break;
		case PINMUX_I2S1:
			PINMUX_CONFIG(PAD_AUD_AOUTR, IIS1_DI);
			PINMUX_CONFIG(AUX0, IIS1_MCLK);
			PINMUX_CONFIG(PAD_AUD_AINR_MIC, IIS1_DO);
			PINMUX_CONFIG(PAD_AUD_AINL_MIC, IIS1_BCLK);
			PINMUX_CONFIG(PAD_AUD_AOUTL, IIS1_LRCK);
			break;
		case PINMUX_I2S2:
			PINMUX_CONFIG(PAD_ETH_RXM, IIS2_DI);
			PINMUX_CONFIG(GPIO_RTX, IIS2_MCLK);
			PINMUX_CONFIG(PAD_ETH_RXP, IIS2_DO);
			PINMUX_CONFIG(PAD_ETH_TXM, IIS2_BCLK);
			PINMUX_CONFIG(PAD_ETH_TXP, IIS2_LRCK);
			break;
		case PINMUX_SDIO0:
			PINMUX_CONFIG(SD0_CD, SDIO0_CD);
			PINMUX_CONFIG(SD0_PWR_EN, SDIO0_PWR_EN);
			PINMUX_CONFIG(SD0_CMD, SDIO0_CMD);
			PINMUX_CONFIG(SD0_CLK, SDIO0_CLK);
			PINMUX_CONFIG(SD0_D0, SDIO0_D_0);
			PINMUX_CONFIG(SD0_D1, SDIO0_D_1);
			PINMUX_CONFIG(SD0_D2, SDIO0_D_2);
			PINMUX_CONFIG(SD0_D3, SDIO0_D_3);
			break;
		case PINMUX_SDIO1:
#if defined(SDIO1_1ST_PAD)
			/*
			 * Name            Address            SD1  MIPI
			 * reg_sd1_phy_sel REG_0x300_0294[10] 0x0  0x1
			 */
			mmio_write_32(TOP_BASE + 0x294,
				(mmio_read_32(TOP_BASE + 0x294) & 0xFFFFFBFF));
			PINMUX_CONFIG(SD1_CMD, PWR_SD1_CMD_VO36);
			PINMUX_CONFIG(SD1_CLK, PWR_SD1_CLK_VO37);
			PINMUX_CONFIG(SD1_D0, PWR_SD1_D0_VO35);
			PINMUX_CONFIG(SD1_D1, PWR_SD1_D1_VO34);
			PINMUX_CONFIG(SD1_D2, PWR_SD1_D2_VO33);
			PINMUX_CONFIG(SD1_D3, PWR_SD1_D3_VO32);
#elif defined(SDIO1_2ND_PAD)
			/*
			 * Name            Address            SD1  MIPI
			 * reg_sd1_phy_sel REG_0x300_0294[10] 0x0  0x1
			 */
			mmio_write_32(TOP_BASE + 0x294,
				(mmio_read_32(TOP_BASE + 0x294) & 0xFFFFFBFF) | BIT(10));
			PINMUX_CONFIG(PAD_MIPI_TXM4, SD1_CLK);
			PINMUX_CONFIG(PAD_MIPI_TXP4, SD1_CMD);
			PINMUX_CONFIG(PAD_MIPI_TXM3, SD1_D0);
			PINMUX_CONFIG(PAD_MIPI_TXP3, SD1_D1);
			PINMUX_CONFIG(PAD_MIPI_TXM2, SD1_D2);
			PINMUX_CONFIG(PAD_MIPI_TXP2, SD1_D3);
#endif
			break;
		case PINMUX_EMMC:
			break;
		case PINMUX_SPI_NOR:
		 	PINMUX_CONFIG(SPINOR_SCK, SPINOR_SCK);
		 	PINMUX_CONFIG(SPINOR_MISO, SPINOR_MISO);
		 	PINMUX_CONFIG(SPINOR_CS_X, SPINOR_CS_X);
		 	PINMUX_CONFIG(SPINOR_MOSI, SPINOR_MOSI);
		 	PINMUX_CONFIG(SPINOR_HOLD_X, SPINOR_HOLD_X);
		 	PINMUX_CONFIG(SPINOR_WP_X, SPINOR_WP_X);
			break;
		 case PINMUX_SPI_NAND:
		 	PINMUX_CONFIG(SPINOR_SCK, SPINAND_CLK);
		 	PINMUX_CONFIG(SPINOR_MISO, SPINAND_MISO);
		 	PINMUX_CONFIG(SPINOR_CS_X, SPINAND_CS);
		 	PINMUX_CONFIG(SPINOR_MOSI, SPINAND_MOSI);
		 	PINMUX_CONFIG(SPINOR_HOLD_X, SPINAND_HOLD);
		 	PINMUX_CONFIG(SPINOR_WP_X, SPINAND_WP);
			break;
		// case PINMUX_CAM0:
		// 	PINMUX_CONFIG(CAM_PD0, CAM_MCLK1);
		// 	PINMUX_CONFIG(CAM_MCLK0, CAM_MCLK0);
		// break;
		// case PINMUX_VI0:
		// 	printf ("fix me\n");
		// break;
		// case PINMUX_VO:
		// 	printf ("fix me\n");
		// break;
		// case PINMUX_PWM1:
		// 	PINMUX_CONFIG(UART1_TX, PWM_4);
		// 	PINMUX_CONFIG(UART1_RX, PWM_5);
		// 	PINMUX_CONFIG(UART1_CTS, PWM_6);
		// 	PINMUX_CONFIG(UART1_RTS, PWM_7);
		// break;
		case PINMUX_SPI_NOR1:
			PINMUX_CONFIG(SD1_D3, PWR_SPINOR1_CS_X);
			PINMUX_CONFIG(SD1_D2, PWR_SPINOR1_HOLD_X);
			PINMUX_CONFIG(SD1_D1, PWR_SPINOR1_WP_X);
			PINMUX_CONFIG(SD1_D0, PWR_SPINOR1_MISO);
			PINMUX_CONFIG(SD1_CMD, PWR_SPINOR1_MOSI);
			PINMUX_CONFIG(SD1_CLK, PWR_SPINOR1_SCK);
			break;
		default:
		break;
	}
}

void cv_reset_assert(uint32_t id)
{
	int bank = id / BITS_PER_REG;
	int offset = id % BITS_PER_REG;
	uint32_t reg;

	reg = mmio_read_32(TOP_BASE + REG_TOP_SOFT_RST + (bank * 4));
	mmio_write_32(reg & ~BIT(offset), TOP_BASE + REG_TOP_SOFT_RST + (bank * 4));

	return;
}

void cv_reset_deassert(uint32_t id)
{
	int bank = id / BITS_PER_REG;
	int offset = id % BITS_PER_REG;
	uint32_t reg;

	reg = mmio_read_32(TOP_BASE + REG_TOP_SOFT_RST + (bank * 4));
	mmio_write_32(reg | BIT(offset), TOP_BASE + REG_TOP_SOFT_RST + (bank * 4));

	return;
}

void system_init(void)
{
	pinmux_config(PINMUX_UART0);
	// pinmux_config(PINMUX_EMMC);
	// pinmux_config(PINMUX_SDIO);
	// pinmux_config(PINMUX_UART1);
	// pinmux_config(PINMUX_UART2);
	// pinmux_config(PINMUX_UART3);
	// pinmux_config(PINMUX_UART4);
	// pinmux_config(PINMUX_UART5);
	// pinmux_config(PINMUX_UART6);
	// pinmux_config(PINMUX_I2C0);
	// pinmux_config(PINMUX_I2S1);
	// pinmux_config(PINMUX_I2S2);
	// pinmux_config(PINMUX_SPI0);

#if defined(UART_INTR_TEST)
	/* uart init seting, user-defined */
	uart_init(5);//pwr_uart init
#endif

	uart_init(0);// bmtest debug form uart1
	irq_init();
	printf("system init done\n");
}

struct irq_action {
	irq_handler_t handler;
	unsigned long flags;
	unsigned int irqn;
	void *priv;
	char name[32];
};

static struct irq_action g_irq_action[256];

#ifdef __riscv
#define ICACHE  (1 << 0)
#define DCACHE  (1 << 1)

#define MCOR_IC_SEL  (1 << 0)
#define MCOR_DC_SEL  (1 << 1)
#define MCOR_INV_SEL (1 << 4)
#define MCOR_CLR_SEL (1 << 5)

void dcache_disable(void)
{
    asm volatile("csrc mhcr, %0"::"rk"(DCACHE));
}

inline void dcache_enable(void)
{
    asm volatile("csrs mhcr, %0"::"rK"(DCACHE));
}

void inv_dcache_all(void)
{
    asm volatile("csrs mcor, %0"::"rk"(MCOR_DC_SEL | MCOR_INV_SEL));
}

void clean_dcache_all(void)
{
    asm volatile("csrs mcor, %0"::"rk"(MCOR_DC_SEL | MCOR_CLR_SEL));
}

inline void inv_clean_dcache_all(void)
{
    asm volatile("csrs mcor, %0"::"rk"(MCOR_DC_SEL | MCOR_INV_SEL | MCOR_CLR_SEL));
}


struct irq_chip {
	const char  *name;
	void        (*irq_mask)(int irq_num);
	void        (*irq_unmask)(int irq_num);
	int         (*irq_ack)(void);
	void        (*irq_set_priority)(int irq_num, int priority);
	void        (*irq_eoi)(int irq_num);
	void        (*irq_set_threshold)(uint32_t threshold);
};

__weak void invalidate_dcache_range(unsigned long start, unsigned long size) {}
__weak void flush_dcache_range(unsigned long start, unsigned long size) {}
__weak void invalidate_icache_all(void) {}
__weak void invalidate_dcache_all(void) {}

static struct irq_chip sirq_chip;

static void plic_mask_irq( int irq_num)
{
	uint32_t mask = (irq_num);
	uint32_t value = 0;
	if(irq_num < 16)
	{
		printf("mask irq_num is %d\n",irq_num);
		return;
	}
	value = mmio_read_32(PLIC_ENABLE1+4*(mask/32));
	value &= ~(0x1 << (mask%32));
	mmio_write_32((PLIC_ENABLE1+(mask/32)*4),value);

}
static void plic_unmask_irq( int irq_num)
{
	uint32_t mask = (irq_num);
	uint32_t value=0;
	if(irq_num < 16)
	{
		printf("unmask irq_num is %d\n",irq_num);
		return;
	}
	//printf("irq_num unmask=%d\n",mask);
	value = mmio_read_32(PLIC_ENABLE1+4*(mask/32));
	value |= (0x1 << (mask%32));
	//printf("value=%x\n",value);
	mmio_write_32((PLIC_ENABLE1+(mask/32)*4),value);

}
static int plic_ack_irq()
{
	return mmio_read_32(PLIC_CLAIM);
}
static void plic_eoi_irq(int irq_num)
{
	mmio_write_32(PLIC_CLAIM, irq_num);
}

static void plic_set_priority_irq(int irq_num, int priority)
{
	//printf("plic_set_priority_irq addr(%x)=%d\n",PLIC_PRIORITY0 + irq_num*4,priority);
	mmio_write_32((PLIC_PRIORITY0 + irq_num*4), priority);
}

static void plic_set_threshold(uint32_t threshold)
{
	mmio_write_32((PLIC_THRESHOLD), threshold);
}

static struct irq_chip sirq_chip = {
	.name           = "RISCV PLIC",
	.irq_mask       = plic_mask_irq,
	.irq_unmask     = plic_unmask_irq,
	.irq_ack        = plic_ack_irq,
	.irq_set_priority = plic_set_priority_irq,
	.irq_eoi        = plic_eoi_irq,
	.irq_set_threshold = plic_set_threshold,
};


#else
__weak void invalidate_dcache_range(unsigned long start, unsigned long size) {}
__weak void flush_dcache_range(unsigned long start, unsigned long size) {}
__weak void invalidate_icache_all(void) {}
__weak void invalidate_dcache_all(void) {}

/*
 * GIC driver
 */

#define REG_BASE_GICD	0x01F01000
#define REG_BASE_GICC	0x01F02000

#define GIC_CPU_CTRL            0x00
#define GIC_CPU_PRIMASK         0x04
#define GIC_CPU_BINPOINT        0x08
#define GIC_CPU_INTACK          0x0c
#define GIC_CPU_EOI             0x10
#define GIC_CPU_RUNNINGPRI      0x14
#define GIC_CPU_HIGHPRI         0x18

#define GIC_DIST_CTRL           0x000
#define GIC_DIST_CTR            0x004
#define GIC_DIST_IGROUP         0x080
#define GIC_DIST_ENABLE_SET     0x100
#define GIC_DIST_ENABLE_CLEAR       0x180
#define GIC_DIST_PENDING_SET        0x200
#define GIC_DIST_PENDING_CLEAR      0x280
#define GIC_DIST_ACTIVE_SET         0x300
#define GIC_DIST_ACTIVE_CLEAR       0x380
#define GIC_DIST_PRI            0x400
#define GIC_DIST_TARGET         0x800
#define GIC_DIST_CONFIG         0xc00
#define GIC_DIST_SOFTINT        0xf00

#define GICC_DIS_BYPASS_MASK	0x1e0

static inline uint32_t read_reg(uint64_t addr)
{
	return *((volatile uint32_t *)addr);
}

static inline void write_reg(uint64_t addr, uint32_t value)
{
	*((volatile uint32_t *)addr) = value;
}

struct gic_data {
	uint64_t dist_base;
	uint64_t cpu_base;
	int gic_irqs;
	int hwirq_base;
	void *chip;
};

struct irq_chip {
	const char  *name;
	void        (*irq_mask)(struct gic_data *data, int irq_num);
	void        (*irq_unmask)(struct gic_data *data, int irq_num);
	int         (*irq_ack)(struct gic_data *data);
	void        (*irq_set_priority)(struct gic_data *data, int irq_num, int priority);
	void        (*irq_eoi)(struct gic_data *data, int irq_num);
	void        (*irq_set_type)(struct gic_data *data, uint32_t flow_type);
};

static void gicd_init(void);
static void gicc_init(void);
static struct irq_chip gic_chip;
static struct gic_data gic_data = {
	.dist_base  = REG_BASE_GICD,
	.cpu_base   = REG_BASE_GICC,
	.chip       = &gic_chip
};

static inline uint32_t gic_data_dist_base(struct gic_data *gic)
{
	return gic->dist_base;
}

static inline uint32_t gic_data_cpu_base(struct gic_data *gic)
{
	return gic->cpu_base;
}

static void irq_trigger_mode(int irqn, int mode)
{
	uint32_t reg_addr, off_in_reg, data;

	reg_addr = gic_data_dist_base(&gic_data) + GIC_DIST_CONFIG + (irqn / 16) * 4;
	off_in_reg = (irqn % 16) * 2;

	data = read_reg(reg_addr);
	data &= ~(3 << off_in_reg);
	data |= mode << off_in_reg;
	write_reg(reg_addr, data);
}
void irq_trigger(int irqn)
{
	irqn += gic_data.hwirq_base;

	write_reg(gic_data_dist_base(&gic_data) + GIC_DIST_PENDING_SET + (irqn / 32) * 4, 1 << (irqn % 32));
}

void irq_clear(int irqn)
{
	irqn += gic_data.hwirq_base;

	write_reg(gic_data_dist_base(&gic_data) + GIC_DIST_PENDING_CLEAR + (irqn / 32) * 4, 1 << (irqn % 32));
}

int irq_get_nums(void)
{
	return gic_data.gic_irqs - gic_data.hwirq_base;
}

static void gic_mask_irq(struct gic_data *d, int irq_num)
{
	uint32_t mask = 1 << (irq_num % 32);

	write_reg(gic_data_dist_base(d) + GIC_DIST_ENABLE_CLEAR + (irq_num / 32) * 4, mask);
}

static void gic_unmask_irq(struct gic_data *d, int irq_num)
{
	uint32_t mask = 1 << (irq_num % 32);

	write_reg(gic_data_dist_base(d) + GIC_DIST_ENABLE_SET + (irq_num / 32) * 4, mask);
}

static void gic_eoi_irq(struct gic_data *d, int irq_num)
{
	write_reg(gic_data_cpu_base(d) + GIC_CPU_EOI, irq_num);
}

static int gic_ack_irq(struct gic_data *d)
{
	return read_reg(gic_data_cpu_base(d) + GIC_CPU_INTACK);
}

static void gic_set_priority_irq(struct gic_data *d, int irq_num, int priority)
{
	uint32_t reg;
	uint32_t spi_pri_base = gic_data_dist_base(d) + GIC_DIST_PRI;
	int bit;

	reg = read_reg(spi_pri_base + (irq_num / 4 * 4));
	bit = (irq_num % 4) * 8;
	reg &= ~(0xF0UL << bit);
	reg |= (0xF0UL & (priority << 4)) << bit;
	write_reg(spi_pri_base + (irq_num / 4 * 4), reg);
}

static void gic_set_type(struct gic_data *d, uint32_t type)
{
	return;
}

static struct irq_chip gic_chip = {
	.name           = "GIC",
	.irq_mask       = gic_mask_irq,
	.irq_unmask     = gic_unmask_irq,
	.irq_ack        = gic_ack_irq,
	.irq_set_priority = gic_set_priority_irq,
	.irq_eoi        = gic_eoi_irq,
	.irq_set_type   = gic_set_type,
};

static uint8_t gic_get_cpumask(struct gic_data *data)
{
	uint32_t base = gic_data_dist_base(&gic_data);
	uint32_t mask, i;

	for (i = mask = 0; i < 32; i += 4) {
		mask = read_reg(base + GIC_DIST_TARGET + i);
		mask |= mask >> 16;
		mask |= mask >> 8;
		if (mask)
			break;
	}

	return mask;
}

static void gicd_init(void)
{
	int hwirq_base = 32;
	int gic_irqs;
	uint32_t base = gic_data_dist_base(&gic_data);
	uint32_t cpumask;
	int i;

	gic_irqs = read_reg(base + GIC_DIST_CTR) & 0x1f;
	gic_irqs = (gic_irqs + 1) * 32;
	if (gic_irqs > 1020)
		gic_irqs = 1020;
	gic_data.gic_irqs = gic_irqs;
	gic_data.hwirq_base = hwirq_base;

	write_reg(base + GIC_DIST_CTRL, 0);

	cpumask = gic_get_cpumask(&gic_data);
	cpumask |= cpumask << 8;
	cpumask |= cpumask << 16;
	for (i = 32; i < gic_irqs; i += 4)
		write_reg(base + GIC_DIST_TARGET + i * 4 / 4, cpumask);

	/* Set all global interrupts to be level triggered, active low. */
	for (i = 32; i < gic_irqs; i += 16)
		write_reg(base + GIC_DIST_CONFIG + i / 4, 0);

	/* Set priority on all global interrupts. */
	for (i = 32; i < gic_irqs; i += 4)
		write_reg(base + GIC_DIST_PRI + i, 0xa0a0a0a0);

	/* Disable all global interrupts. */
	for (i = 32; i < gic_irqs; i += 32) {
		write_reg(base + GIC_DIST_ACTIVE_CLEAR + i / 8, 0xffffffff);
		write_reg(base + GIC_DIST_ENABLE_CLEAR + i / 8, 0xffffffff);
	}

	write_reg(base + GIC_DIST_CTRL, 1);
}

static void gicc_init(void)
{
	int i;
	uint32_t base, bypass;

	base = gic_data_dist_base(&gic_data);
	write_reg(base + GIC_DIST_ACTIVE_CLEAR, 0xffffffff);
	write_reg(base + GIC_DIST_ENABLE_CLEAR, 0xffff0000);
	write_reg(base + GIC_DIST_ENABLE_SET, 0x0000ffff);

	for (i = 0; i < 32; i += 4)
		write_reg(base + GIC_DIST_PRI + i * 4 / 4, 0xa0a0a0a0);

	base = gic_data_cpu_base(&gic_data);
	write_reg(base + GIC_CPU_PRIMASK, 0xf0);

	bypass = read_reg(base + GIC_CPU_CTRL);
	bypass &= GICC_DIS_BYPASS_MASK;

	/* Enable CPU interrupt */
	write_reg(base + GIC_CPU_CTRL, bypass | 1);
}
#endif
#ifndef __riscv
static void cpu_route_irqs_el2(void)
{
	int get_arm_current_el(void);

	uint64_t el;
	uint64_t hcr_el2 = 0x0038; // set AMO, IMO, and FMO

	asm volatile(
		"mrs %0, currentel\n"
		"lsr %0, %0, #2\n"
		"and %0, %0, #3\n"
		: "=r" (el) : );

	if (el == 2) {
		asm volatile("mrs x0, hcr_el2\n"
			     "bic x0, x0, #0x08000000\n" // clear TGE
			     "orr x0, x0, %0\n"
			     "msr hcr_el2, x0\n"
			     :
			     : "r"(hcr_el2)
			     : "x0");
	}
}
#endif

void cpu_enable_irqs(void)
{
#ifdef __riscv
    set_csr(mstatus, MSTATUS_MIE);
    set_csr(mie, MIP_MEIE);
#else
	cpu_route_irqs_el2();

    asm volatile (    \
        "msr	daifclr, #6	  "   \
        :                         \
        :                         \
        : "memory");              \

#endif
}

void cpu_disable_irqs(void)
{
#ifdef __riscv
    clear_csr(mstatus, MSTATUS_MIE);
    clear_csr(mie, MIP_MEIE);
#else
    asm volatile (    \
        "msr	daifset, #6	  "   \
        :                         \
        :                         \
        : "memory");              \

#endif
}

void irq_init(void)
{
#ifdef __riscv
	int i;
	// clear interrupt enable
	write_csr(mie, 0);
	// clear interrupt pending
	write_csr(mip, 0);

	// Clean the setting of all IRQ
	for (i = 0; i < 256 * 4; i = i + 4) {
		mmio_write_32(((uintptr_t)PLIC_PRIORITY0 + i), 0);
	}
	for (i = 0; i <= 256/32; i++) {
		mmio_write_32((PLIC_PENDING1  + i*4), 0);
		mmio_write_32((PLIC_ENABLE1  + i*4), 0);
	}
	memset(g_irq_action, 0, sizeof(struct irq_action)*256);
	sirq_chip.irq_set_threshold(0);
	cpu_enable_irqs();
#else
	cpu_enable_irqs();
	gicd_init();
	gicc_init();
#endif
}

int request_irq(unsigned int irqn, irq_handler_t handler, unsigned long flags,
		const char *name, void *priv)
{
	if ((irqn < 0) || (irqn >= NUM_IRQ))
		return -1;
#ifdef __riscv
	//printf("request_irq irqn=%d\n handler=%lx  name = %s\n",irqn,(long) handler,name);
	g_irq_action[irqn].handler = handler;
	if (name) {
		memcpy(g_irq_action[irqn].name, name, sizeof(g_irq_action[irqn].name));
		g_irq_action[irqn].name[sizeof(g_irq_action[irqn].name) - 1] = 0;
	}
	g_irq_action[irqn].irqn = irqn ; //- gic_data.hwirq_base;
	g_irq_action[irqn].flags = flags;
	g_irq_action[irqn].priv = priv;
	// set highest priority
	sirq_chip.irq_set_priority( irqn, 7);
	// unmask irq
	sirq_chip.irq_unmask( irqn);
#else
	irqn += gic_data.hwirq_base;

	if (flags & (IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING))
		irq_trigger_mode(irqn, IRQ_EDGE);
	else
		irq_trigger_mode(irqn, IRQ_LEVEL);

	g_irq_action[irqn].handler = handler;
	if (name) {
		memcpy(g_irq_action[irqn].name, name, sizeof(g_irq_action[irqn].name));
		g_irq_action[irqn].name[sizeof(g_irq_action[irqn].name) - 1] = 0;
	}
	g_irq_action[irqn].irqn = irqn - gic_data.hwirq_base;
	g_irq_action[irqn].flags = flags;
	g_irq_action[irqn].priv = priv;

	gic_chip.irq_set_priority(&gic_data, irqn, 10);

	gic_chip.irq_unmask(&gic_data, irqn);
#endif
	return 0;
}

void  printf_once(char *string, int irq)
{
        static uint8_t __print_once = false;
        static int id = 0;

        if (!__print_once || (id != irq)) {
                __print_once = true;
		id =(int) irq;
                printf("%s %d\n",string, irq);
        }
}

void do_irq(void)
{
	int irqn;
#ifdef __riscv
	do{
		irqn = sirq_chip.irq_ack();
		if(g_irq_action[irqn].handler && irqn)
		{
//			printf("do_irq irqn=%d\n",irqn);
			g_irq_action[irqn].handler(g_irq_action[irqn].irqn, g_irq_action[irqn].priv);
		}
		else if(irqn)
			printf("g_irq_action[%i] NULL",irqn);
		else //plic_claim =0
			break;
		// clear plic pending
		sirq_chip.irq_eoi(irqn);
	}while(1);
	// clear external interrupt pending
	clear_csr(mip, MIP_MEIE);
#else
	irqn = gic_chip.irq_ack(&gic_data);
	printf_once("do_irq", irqn - 32);
	g_irq_action[irqn].handler(g_irq_action[irqn].irqn, g_irq_action[irqn].priv);
	gic_chip.irq_eoi(&gic_data, irqn);
#endif
}

void disable_irq(unsigned int irqn)
{
#ifdef __riscv
	sirq_chip.irq_mask( irqn);
#else
	irqn += gic_data.hwirq_base;
	gic_chip.irq_mask(&gic_data, irqn);
#endif
}

void enable_irq(unsigned int irqn)
{
#ifdef __riscv
	sirq_chip.irq_unmask( irqn);
#else
	irqn += gic_data.hwirq_base;
	gic_chip.irq_unmask(&gic_data, irqn);
#endif
}
