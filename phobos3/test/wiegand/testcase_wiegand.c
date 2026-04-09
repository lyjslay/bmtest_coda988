#include "module_testcase.h"
#include "system_common.h"
#include "mmio.h"
#include "timer.h"
#include "testcase_wiegand.h"

#define WGN_MAX_NUM 3
static char flag0, flag1;
struct cvi_wiegand_device wiegand_device[3] = {{WIEGAND0, 0x03030000, WGN0_IRQ, 0x0},
	{WIEGAND1, 0x03031000, WGN1_IRQ, 0x0}, {WIEGAND2, 0x03032000, WGN2_IRQ, 0x0}};

void wiegand_set_pinmux(void)
{
		mmio_write_32(0x3001008, 6); /* WG0_D0 */
		mmio_write_32(0x300100c, 6); /* WG0_D1 */
		mmio_write_32(0x3001010, 6); /* WG1_D0 */
		mmio_write_32(0x3001014, 6); /* WG1_D1 */
		//68 pin chip just exist two wiegand module	
		//mmio_write_32(0x3001008, 7); /* WG2_D0 */
		//mmio_write_32(0x3001008, 7); /* WG2_D1 */

		mmio_write_32(0x3001a08, 0x1 << 2 | 0x01 << 6);
		mmio_write_32(0x3001a0c, 0x1 << 2 | 0x01 << 6);
		mmio_write_32(0x3001a10, 0x1 << 2 | 0x01 << 6);
		mmio_write_32(0x3001a14, 0x1 << 2 | 0x01 << 6);
}

static int cvi_wiegand_set_tx_cfg(struct cvi_wiegand_device *ndev, struct wgn_tx_cfg *tx_cfg_ptr)
{
	uint32_t value = 0;

	printf("cvi_wiegand_set_tx_cfg\n");
	printf("set tx config tx_lowtime=%x, tx_hightime=%x, tx_bitcount=%d, tx_msb1st=%d, tx_opendrain=%d\n", \
		tx_cfg_ptr->tx_lowtime, tx_cfg_ptr->tx_hightime, tx_cfg_ptr->tx_bitcount, \
		tx_cfg_ptr->tx_msb1st, tx_cfg_ptr->tx_opendrain);

	if (tx_cfg_ptr->tx_lowtime > 0xFFFFFF)
		tx_cfg_ptr->tx_lowtime = 0xFFFFFF;
	mmio_write_32(ndev->wiegand_vaddr + TX_CONFIG0, tx_cfg_ptr->tx_lowtime);

	if (tx_cfg_ptr->tx_hightime > 0xFFFFFF)
		tx_cfg_ptr->tx_hightime = 0xFFFFFF;
	mmio_write_32(ndev->wiegand_vaddr + TX_CONFIG1, tx_cfg_ptr->tx_hightime);

	if (tx_cfg_ptr->tx_bitcount > 128)
		tx_cfg_ptr->tx_bitcount = 128;
	if (tx_cfg_ptr->tx_msb1st > 1)
		tx_cfg_ptr->tx_msb1st = 1;
	if (tx_cfg_ptr->tx_opendrain > 1)
		tx_cfg_ptr->tx_opendrain = 1;

	value = (tx_cfg_ptr->tx_opendrain << 16) | (tx_cfg_ptr->tx_msb1st << 8) | tx_cfg_ptr->tx_bitcount;
	mmio_write_32(ndev->wiegand_vaddr + TX_CONFIG2, value);

	return 0;
}

static int cvi_wiegand_set_rx_cfg(struct cvi_wiegand_device *ndev, struct wgn_rx_cfg *rx_cfg_ptr)
{
	uint32_t value = 0;

	printf("cvi_wiegand_set_rx_cfg\n");
	printf("set rx config rx_debounce=%x, rx_idle_timeout=%x, rx_bitcount=%d, rx_msb1st=%d\n", rx_cfg_ptr->rx_debounce, \
		rx_cfg_ptr->rx_idle_timeout, rx_cfg_ptr->rx_bitcount, rx_cfg_ptr->rx_msb1st);

	if (rx_cfg_ptr->rx_debounce > 0xFFFF)
		rx_cfg_ptr->rx_debounce = 0xFFFFF;
	mmio_write_32(ndev->wiegand_vaddr + RX_CONFIG0, rx_cfg_ptr->rx_debounce);

	if (rx_cfg_ptr->rx_idle_timeout > 0xFFFFFFFF)
		rx_cfg_ptr->rx_idle_timeout = 0xFFFFFFFF;
	mmio_write_32(ndev->wiegand_vaddr + RX_CONFIG1, rx_cfg_ptr->rx_idle_timeout);

	if (rx_cfg_ptr->rx_bitcount > 128)
		rx_cfg_ptr->rx_bitcount = 128;
	if (rx_cfg_ptr->rx_msb1st > 1)
		rx_cfg_ptr->rx_msb1st = 1;

	value = (rx_cfg_ptr->rx_msb1st << 8) | rx_cfg_ptr->rx_bitcount;
	mmio_write_32(ndev->wiegand_vaddr + RX_CONFIG2, value);

	return 0;
}

static int cvi_wiegand_get_tx_cfg(struct cvi_wiegand_device *ndev, struct wgn_tx_cfg *tx_cfg_ptr)
{
	uint32_t value;

	printf("cvi_wiegand_get_tx_cfg\n");

	value = mmio_read_32(ndev->wiegand_vaddr + TX_CONFIG0);
	tx_cfg_ptr->tx_lowtime = value & 0xFFFFFF;
	value = mmio_read_32(ndev->wiegand_vaddr + TX_CONFIG1);
	tx_cfg_ptr->tx_hightime = value & 0xFFFFFF;
	value = mmio_read_32(ndev->wiegand_vaddr + TX_CONFIG2);
	tx_cfg_ptr->tx_bitcount = value & 0x7F;
	tx_cfg_ptr->tx_msb1st = (value & 0x100) >> 8;
	tx_cfg_ptr->tx_opendrain = (value & 0x10000) >> 16;

	printf("get tx config tx_lowtime=%x, tx_hightime=%x, tx_bitcount=%d, tx_msb1st=%d, tx_opendrain=%d\n", \
		tx_cfg_ptr->tx_lowtime, tx_cfg_ptr->tx_hightime, tx_cfg_ptr->tx_bitcount, \
		tx_cfg_ptr->tx_msb1st, tx_cfg_ptr->tx_opendrain);

	return 0;
}

static int cvi_wiegand_get_rx_cfg(struct cvi_wiegand_device *ndev, struct wgn_rx_cfg *rx_cfg_ptr)
{
	uint32_t value;

	printf("cvi_wiegand_get_rx_cfg\n");
	value = mmio_read_32(ndev->wiegand_vaddr + RX_CONFIG0);
	rx_cfg_ptr->rx_debounce = value & 0xFFFF;
	value = mmio_read_32(ndev->wiegand_vaddr + RX_CONFIG1);
	rx_cfg_ptr->rx_idle_timeout = value & 0xFFFFFFFF;
	value = mmio_read_32(ndev->wiegand_vaddr + RX_CONFIG2);
	rx_cfg_ptr->rx_bitcount = value & 0x7F;
	rx_cfg_ptr->rx_msb1st = (value & 0x100) >> 8;

	printf("get rx config rx_debounce=%x, rx_idle_timeout=%x, rx_bitcount=%d, rx_msb1st=%d\n", rx_cfg_ptr->rx_debounce, \
		rx_cfg_ptr->rx_idle_timeout, rx_cfg_ptr->rx_bitcount, rx_cfg_ptr->rx_msb1st);

	return 0;
}

static int cvi_wiegand_tx(struct cvi_wiegand_device *ndev, unsigned long arg)
{
	printf("cvi_wiegand_tx\n");
	printf("low tx_data: %#X\n", (uint32_t)(ndev->tx_data));
	printf("high tx_data: %#X\n", (uint32_t)(ndev->tx_data >> 32));

	mmio_write_32(ndev->wiegand_vaddr + TX_BUFFER, (uint32_t)(ndev->tx_data));
	mmio_write_32(ndev->wiegand_vaddr + TX_BUFFER1, (uint32_t)(ndev->tx_data >> 32));
	while (mmio_read_32(ndev->wiegand_vaddr + TX_BUSY));
	mmio_write_32(ndev->wiegand_vaddr + TX_TRIGGER, 0x1);
	return 0;
}

static int cvi_wiegand_rx(struct cvi_wiegand_device *ndev, unsigned long arg)
{
	uint32_t value;

	// Clear RX buffer
	mmio_write_32(ndev->wiegand_vaddr + RX_BUFFER_CLEAR, 0x1);

	// Enable RX receive interrupt
	mmio_write_32(ndev->wiegand_vaddr + IRQ_ENABLE, 0x100);
	mmio_write_32(ndev->wiegand_vaddr + IRQ_CLEAR, 0x111);

	// Enable RX
	value = mmio_read_32(ndev->wiegand_vaddr + RX_CONFIG2);
	value |= 0x1000;
	mmio_write_32(ndev->wiegand_vaddr + RX_CONFIG2, value);

	printf("cvi_wiegand_rx trig\n");
	return 0;
}



static int cvi_wiegand_rx_wait(void)
{
	while(flag0 != 'y' && flag1 != 'y' ){};

	printf("rx get data\n");

	return 0;
}
/*
ssize_t cvi_wiegand_write(struct file *filp, const char *buff, size_t count, loff_t *offp)
{
	if (copy_from_user(&flag, buff, 1))
		return -EFAULT;

	pr_debug("cvi_wiegand_write\n");

	wake_up_interruptible(&wq);

	return count;
}
*/

int testcase_wiegand(void)
{

	struct cvi_wiegand_device *ndev = wiegand_device;
	long ret = 0, i; 
	struct wgn_tx_cfg tx_cfg[2] = {{0x00186A, 0x00C350, 26, 1, 0},   //250 us,2ms,
		{0x00186A, 0x00C350, 34, 1, 0}}; // Hex : 22_85_EA 

	struct wgn_rx_cfg rx_cfg[2] ={{100, 0x2625A0, 26,1}, {100, 0x2625A0, 34,1}}; //100ms timeout
	uint64_t tx_data[2] = {0x2070002, 0x11108d5};//f=1,user=0x8001; Hex : 22_85_EA 
	uint64_t rx_data;

	struct wgn_tx_cfg tx_cfg_temp;
	struct wgn_rx_cfg rx_cfg_temp;


	printf("\n------------ wiegand test start ------------\n\n");

	ret = cvi_wiegand_set_tx_cfg(&ndev[0], &tx_cfg[0]);
	ret = cvi_wiegand_get_tx_cfg(&ndev[0], &tx_cfg_temp);
	wiegand_device[0].tx_data = tx_data[0];

	ret = cvi_wiegand_set_tx_cfg(&ndev[1], &tx_cfg[1]);
	ret = cvi_wiegand_get_tx_cfg(&ndev[1], &tx_cfg_temp);
	wiegand_device[1].tx_data = tx_data[1];
#if 1
	for(i = 0; i < 10; i++){
		
		ret = cvi_wiegand_tx(&ndev[0] , 0); //set tx data and trig
		mdelay(10000);
	}

	printf("\n------------ wiegand dev0 tx test end ------------\n\n");

	printf("\n------------ wiegand dev1 tx test start ------------\n\n");

	for(i = 0; i < 10; i++){
		
		ret = cvi_wiegand_tx(&ndev[1] , 0); //set tx data and trig
		mdelay(10000);
	}
#endif
	printf("\n------------ wiegand rx test start ------------\n\n");

	ret = cvi_wiegand_set_rx_cfg(&ndev[0], &rx_cfg[0]);
	ret = cvi_wiegand_get_rx_cfg(&ndev[0], &rx_cfg_temp);//set wiegand0 rx 26 bit type

	ret = cvi_wiegand_set_rx_cfg(&ndev[1], &rx_cfg[1]);
	ret = cvi_wiegand_get_rx_cfg(&ndev[1], &rx_cfg_temp); //set wiegand0 rx 34 bit type

	ret = cvi_wiegand_rx(&ndev[0], 0); //enable wg1 rx, and then trig wg0 to tx
	ret = cvi_wiegand_rx(&ndev[1], 0); //enable wg1 rx, and then trig wg0 to tx

	for(i=0; i < 10;i++){

		cvi_wiegand_rx_wait();
		if(flag0 == 'y') {

			flag0 = 'n';
			if ((mmio_read_32(ndev[0].wiegand_vaddr + RX_BUFFER_VALID) & 0x1) == 0) {
				printf("RX buffer is INVALID\n");
				rx_data = 0;
			} else {
					//retrieve data
				printf("RX buffer is VALID\n");
				rx_data = mmio_read_32(ndev[0].wiegand_vaddr + RX_BUFFER1);
				rx_data <<= 32;
				rx_data |= mmio_read_32(ndev[0].wiegand_vaddr + RX_BUFFER);

				printf("rx data=%lx\n", rx_data);
			}
		} else {

			flag1 = 'n';
			if ((mmio_read_32(ndev[1].wiegand_vaddr + RX_BUFFER_VALID) & 0x1) == 0) {
				printf("RX buffer is INVALID\n");
				rx_data = 0;
			} else {
					//retrieve data
				printf("RX buffer is VALID\n");
				rx_data = mmio_read_32(ndev[1].wiegand_vaddr + RX_BUFFER1);
				rx_data <<= 32;
				rx_data |= mmio_read_32(ndev[1].wiegand_vaddr + RX_BUFFER);

				printf("rx data=%lx\n", rx_data);
			}
		}		
	}
	return ret;
}

static int wiegand_clk_init(void)
{
	return 0;
}

int phobos_wiegand0_callback(int irq, void *priv)
{

	//clear irq
	mmio_write_32(wiegand_device[0].wiegand_vaddr + IRQ_CLEAR, 0x111);
	printf("%s, get irq=%d\n", __func__, irq);

	flag0 = 'y';

	return 0;
}

int phobos_wiegand1_callback(int irq, void *priv)
{

	//clear irq
	mmio_write_32(wiegand_device[1].wiegand_vaddr + IRQ_CLEAR, 0x111);
	printf("%s, get irq=%d\n", __func__, irq);

	flag1 = 'y';

	return 0;
}

static int wiegand_init(void)
{
    //init interrupt
    wiegand_set_pinmux();

    request_irq(64, phobos_wiegand0_callback, 0, "wiegand0", NULL);

    request_irq(65, phobos_wiegand1_callback, 0, "wiegand1", NULL);

	mmio_write_32(wiegand_device[0].wiegand_vaddr + IRQ_CLEAR, 0x111);
	mmio_write_32(wiegand_device[1].wiegand_vaddr + IRQ_CLEAR, 0x111);
	enable_irq(64);
	enable_irq(65);

    wiegand_clk_init();
	return 0;
}

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
	int ret;

	ret = wiegand_init();
	ret = testcase_wiegand();

	printf("testcase_wiegand(): %s\n", ret ? "failed" : "passed");

	while (1)
		;

	return 0;
}
#endif
