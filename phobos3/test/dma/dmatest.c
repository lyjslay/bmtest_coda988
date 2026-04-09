#include "boot_test.h"
#include "uart.h"
#include "cli.h"
#include "sysdma.h"
#include "command.h"
#include "mmio.h"

#define SDMA_INTR_NUM	0
#define MEM_SRC_BLOCK	0x82000000
#define MEM_DST_BLOCK	0x84000000
//#define MEM_BLOCK_SIZE	(2 * 1024 * 1024)
#define MEM_BLOCK_SIZE 1 * 1024
#define MEM_BLOCK_SIZE2	(16 * 4)

#define UART_PORT(port)	uart##port
#define ARRAY_SIZE(x)	(sizeof(x) / sizeof((x)[0]))

#define thr rbr
#define iir fcr
#define dll rbr
#define dlm ier

struct dw_regs {
	volatile uint32_t	rbr;		/* 0x00 Data register */
	volatile uint32_t	ier;		/* 0x04 Interrupt Enable Register */
	volatile uint32_t	fcr;		/* 0x08 FIFO Control Register */
	volatile uint32_t	lcr;		/* 0x0C Line control register */
	volatile uint32_t	mcr;		/* 0x10 Line control register */
	volatile uint32_t	lsr;		/* 0x14 Line Status Register */
	volatile uint32_t	msr;		/* 0x18 Modem Status Register */
	volatile uint32_t	spr;		/* 0x20 Scratch Register */
};

#define UART_LCR_WLS_MSK 0x03       /* character length select mask */
#define UART_LCR_WLS_5  0x00        /* 5 bit character length */
#define UART_LCR_WLS_6  0x01        /* 6 bit character length */
#define UART_LCR_WLS_7  0x02        /* 7 bit character length */
#define UART_LCR_WLS_8  0x03        /* 8 bit character length */
#define UART_LCR_STB    0x04        /* # stop Bits, off=1, on=1.5 or 2) */
#define UART_LCR_PEN    0x08        /* Parity eneble */
#define UART_LCR_EPS    0x10        /* Even Parity Select */
#define UART_LCR_STKP   0x20        /* Stick Parity */
#define UART_LCR_SBRK   0x40        /* Set Break */
#define UART_LCR_BKSE   0x80        /* Bank select enable */
#define UART_LCR_DLAB   0x80        /* Divisor latch access bit */

#define UART_MCR_DTR    0x01        /* DTR   */
#define UART_MCR_RTS    0x02        /* RTS   */

#define UART_LSR_THRE		0x20 /* Transmit-hold-register empty */
#define UART_LSR_DR		0x01 /* Receiver data ready */
#define UART_LSR_TEMT   0x40        /* Xmitter empty */

#define UART_FCR_FIFO_EN    0x01 /* Fifo enable */
#define UART_FCR_RXSR       0x02 /* Receiver soft reset */
#define UART_FCR_TXSR       0x04 /* Transmitter soft reset */

#define UART_MCRVAL (UART_MCR_DTR | UART_MCR_RTS)      /* RTS/DTR */
#define UART_FCR_DEFVAL	(UART_FCR_FIFO_EN | UART_FCR_RXSR | UART_FCR_TXSR)
#define UART_LCR_8N1    0x03

static struct dw_regs *uart0 = (struct dw_regs *)0x04140000;
static struct dw_regs *uart1 = (struct dw_regs *)0x04150000;
static struct dw_regs *uart2 = (struct dw_regs *)0x04160000;
static struct dw_regs *uart3 = (struct dw_regs *)0x04170000;
static struct dw_regs *uart4 = (struct dw_regs *)0x041c0000;

struct slave_config mem2mem_cfg = {
	.direction			= DMA_MEM_TO_MEM,
	.src_addr			= MEM_SRC_BLOCK,
	.dst_addr			= MEM_DST_BLOCK,
	.data_width			= SLAVE_BUSWIDTH_4_BYTES,
	.src_addr_width			= SLAVE_BUSWIDTH_4_BYTES,
	.dst_addr_width			= SLAVE_BUSWIDTH_4_BYTES,
	.src_msize			= DW_DMA_MSIZE_32,
	.dst_msize			= DW_DMA_MSIZE_32,
	.device_fc			= DW_DMA_FC_D_M2M,
	.src_master			= DMA0,
	.dst_master			= DMA0,
	.len				= MEM_BLOCK_SIZE
};

struct slave_config mem2uart_cfg = {
	.direction			= DMA_MEM_TO_DEV,
	.src_addr			= MEM_SRC_BLOCK,
	.dst_addr			= MEM_DST_BLOCK,
	.data_width			= SLAVE_BUSWIDTH_4_BYTES,
	.src_addr_width			= SLAVE_BUSWIDTH_4_BYTES,
	.dst_addr_width			= SLAVE_BUSWIDTH_4_BYTES,
	.src_msize			= DW_DMA_MSIZE_16,
	.dst_msize			= DW_DMA_MSIZE_16,
	.device_fc			= DW_DMA_FC_D_M2P,
	.src_master			= DMA0,
	.dst_master			= DMA0,
	.len				= MEM_BLOCK_SIZE2,
	.src_handshake_id		= 1, //uart1 tx mapped to dma_remap ch 1
	.dst_handshake_id		= 1
};

struct slave_config uart2mem_cfg = {
	.direction			= DMA_DEV_TO_MEM,
	.src_addr			= MEM_SRC_BLOCK,
	.dst_addr			= MEM_DST_BLOCK,
	.data_width			= SLAVE_BUSWIDTH_4_BYTES,
	.src_addr_width			= SLAVE_BUSWIDTH_4_BYTES,
	.dst_addr_width			= SLAVE_BUSWIDTH_4_BYTES,
	.src_msize			= DW_DMA_MSIZE_16,
	.dst_msize			= DW_DMA_MSIZE_16,
	.device_fc			= DW_DMA_FC_D_P2M,
	.src_master			= DMA0,
	.dst_master			= DMA0,
	.len				= MEM_BLOCK_SIZE2,
	.src_handshake_id		= 2, //uart2 rx mapped to dma_remap ch 2
	.dst_handshake_id		= 2
};

static struct dw_regs * get_uart_port(int port)
{
	struct dw_regs * uart;

	switch(port) {
	case 0:
		uart = UART_PORT(0);
		break;
	case 1:
		uart = UART_PORT(1);
		break;
	case 2:
		uart = UART_PORT(2);
		break;
	case 3:
		uart = UART_PORT(3);
		break;
	case 4:
		uart = UART_PORT(4);
		break;
	default:
		break;
	}

	return uart;
}

static void dma_uart_init(int port)
{
#if defined(BOARD_PALLADIUM)
	int baudrate = 9600;
	int uart_clock = 9600 * 16;
#else
	int baudrate = 115200;
	int uart_clock = 25 * 1000 * 1000;
#endif
	int divisor = uart_clock / (16 * baudrate);
	struct dw_regs * uart = get_uart_port(port);

	uart->lcr = uart->lcr | UART_LCR_DLAB | UART_LCR_8N1;
	uart->dll = divisor & 0xff;
	uart->dlm = (divisor >> 8) & 0xff;
	uart->lcr = uart->lcr & (~UART_LCR_DLAB);

	//uart->ier = 1 << 7;
	uart->ier = 0x8f;
	uart->mcr = UART_MCRVAL;
	uart->fcr = UART_FCR_DEFVAL | 0b10 << 6;

	uart->lcr = 3;
}

#if 0
static void wiatting_uart_int(int port)
{
	struct dw_regs * uart = get_uart_port(port);

	do {
		udelay(10);
	} while(!uart->iir);
}
#endif

static void prepare_src_memory(void *addr, unsigned int len, char value)
{
	memset(addr, value, len);
}

static int check_dst_memory(void *src, void *dst, unsigned int len)
{
	return memcmp(src, dst, len);
}

static int test_uart_loop(void)
{
	int ret;
	struct dma_chan chan0, chan1;
	int i;

	chan0.chan_id = 3;//channel 3, there is no UART1 io on cv1822, then we use UART2 TX
	chan0.config = &mem2uart_cfg;
	chan0.direction = DMA_MEM_TO_DEV;
	chan0.config->reg = (u64)uart2; //use UART2 TX to instead of UART1 TX

	chan1.chan_id = 2;//channel 2
	chan1.config = &uart2mem_cfg;
	chan1.direction = DMA_DEV_TO_MEM;
	chan1.config->reg = (u64)uart2;

	ret = device_alloc_chan(&chan0);
	if(ret < 0)
		return ret;
	ret = device_alloc_chan(&chan1);
	if(ret < 0)
		return ret;

	prepare_src_memory((void *)MEM_SRC_BLOCK, MEM_BLOCK_SIZE2, 0xa6);

	ret = device_prep_dma_slave(&chan1); //prepare RX first
	if(ret < 0)
		return ret;

	ret = device_prep_dma_slave(&chan0);
	if(ret < 0)
		return ret;



	ret = device_issue_pending(&chan0);
	if(ret < 0)
		return ret;

	/* uart interrupts arrive */
	//wiatting_uart_int(1);

	ret = device_issue_pending(&chan1);
	if(ret < 0)
		return ret;

	for (i = 0; i < (0x40 / 8); i++){
		if ((i % 16 == 0) && (i != 0))
			printf("\n");
		printf("0x%x ", *(char *)(chan1.config->dst_addr+i*8));
	}
	printf("\n");

	return 0;
}

static int test_mem_to_mem(void)
{
	int ret;
	struct dma_chan *chan;

	chan = malloc(sizeof(*chan));
	if(!chan)
		printf("chan mem alloc failed \n");

	chan->chan_id = 0;//channel 0
	chan->config = &mem2mem_cfg;
	chan->direction = DMA_MEM_TO_MEM;

	ret = device_alloc_chan(chan);
	if(ret < 0)
		return ret;
	printf("start device_prep_dma_slave\n");
	ret = device_prep_dma_slave(chan);
	if(ret < 0)
		return ret;
	srand(10);
	printf("start prepare_src_memory\n");
	prepare_src_memory((void *)chan->config->src_addr,
		chan->config->len, rand() & 0xff);
	printf("start dma tx/rx\n");
	ret = device_issue_pending(chan);
	if(ret < 0)
		return ret;
	printf("check dst memory\n");
	ret = check_dst_memory((void *)chan->config->src_addr,
		(void *)chan->config->dst_addr, chan->config->len);
	printf("check finished\n");
	if(ret) {
		return ret;
	}
	return 0;
}

static int do_mcpy(int argc, char **argv)
{
	int ret;

	ret = test_mem_to_mem();
	uartlog("testcase dma memory to memory copy %s\n", ret?"failed":"passed");

	return ret;
}

static int do_uart_loop(int argc, char **argv)
{
	int ret;

	//dma_uart_init(1);
	dma_uart_init(2);
	ret = test_uart_loop();
	uartlog("testcase dma uart loop %s\n", ret?"failed":"passed");

	return ret;
}
#if 1
static int irq_handler(int irqn, void *priv)
{
	printf("irqn=%d\n", irqn);
	disable_irq(irqn);

    return 0;
}
#endif

static struct cmd_entry test_cmd_list[] __attribute__ ((unused)) = {
	{"mcpy", do_mcpy, 0, "memory copy"},
	{"uloop", do_uart_loop, 0, "uart sysdma loop"},
	{NULL, NULL, 0, NULL}
};

static void dma_channel_init(void)
{
	mmio_clrsetbits_32(TOP_DMA_CH_REMAP0, 0x3f<<DMA_REMAP_CH0_OFFSET, DMA_RX_REQ_N_UART2 << DMA_REMAP_CH0_OFFSET);
	mmio_clrsetbits_32(TOP_DMA_CH_REMAP0, 0x3f<<DMA_REMAP_CH1_OFFSET, DMA_TX_REQ_N_UART2 << DMA_REMAP_CH1_OFFSET);
	mmio_clrsetbits_32(TOP_DMA_CH_REMAP0, 0x3f<<DMA_REMAP_CH2_OFFSET, DMA_RX_REQ_N_UART2 << DMA_REMAP_CH2_OFFSET);
	mmio_clrsetbits_32(TOP_DMA_CH_REMAP0, 0x3f<<DMA_REMAP_CH3_OFFSET, DMA_TX_REQ_N_UART2 << DMA_REMAP_CH3_OFFSET);
	mmio_clrsetbits_32(TOP_DMA_CH_REMAP0, 0x1<<DMA_REMAP_UPDATE_OFFSET, 1 << DMA_REMAP_UPDATE_OFFSET);
}

int testdma_main(void)
{
	int i, ret = 0;

	uartlog("enter dma testcase \n");
	pinmux_config(PINMUX_UART2);
	request_irq(UART2_INTR, irq_handler, 0, "UART2_INTR int", NULL);
	dma_channel_init();
	//sysdma_init();
	for(i = 0;i < ARRAY_SIZE(test_cmd_list) - 1;i++) {
		command_add(&test_cmd_list[i]);
	}
	cli_simple_loop();

	return ret;
}

int testcase_main(void)
{
  int ret;

  ret = testdma_main();
 // uartlog("testcase dma %s\n", ret?"failed":"passed");
  return ret;
}
module_testcase("1", testdma_main);
