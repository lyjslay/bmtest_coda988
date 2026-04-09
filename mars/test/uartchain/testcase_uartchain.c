#include "boot_test.h"
#include "cli.h"
#include "command.h"
#include "uartchain.h"
#include "uart_dma.h"

static unsigned char msg1[1024];
static unsigned char msg2[1024];
static unsigned char msg3[1024];

int send(int set, int delay)
{
	char s[] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};
	char s2[] = {0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59};

	printf("send");
	if(delay)
	{
		printf(" delay");
	}
	printf("\n");

	for(int i = 0; i < 1024 / 64; i++)
	{
		for(int j = 0; j < 64; j++)
		{
			msg1[i * 64 + j] = (unsigned char)j + 64;
			msg2[i * 64 + j] = (unsigned char)j + 64 * 2;
			msg3[i * 64 + j] = (unsigned char)j + 64 * 3;
		}
	}
	msg1[0] = 1;
	msg2[0] = 2;
	msg3[0] = 3;
	msg1[1023] = 0x39;
	msg2[1023] = 0x39;
	msg3[1023] = 0x39;

	//while(1)
  for(int xxx = 0; xxx < 90000; xxx++)
	{
		//printf("start\n");
		uartchain_start_send();
		if(set == 2)
		{
			for(int i = 0; i < sizeof(msg1); i++)
			{
				uartchain_putc(msg1[i]);
			}
		}
		else if(set == 3)
		{
			for(int i = 0; i < sizeof(msg2); i++)
			{
				uartchain_putc(msg2[i]);
			}
		}
		else if(set == 4)
		{
			for(int i = 0; i < sizeof(msg3); i++)
			{
				uartchain_putc(msg3[i]);
			}
		}
		else
		{
			for(int i = 0; i < sizeof(s); i++)
			{
				if(set)
					uartchain_putc(s2[i]);
				else
					uartchain_putc(s[i]);
			}
		}
		//printf("end0\n");
		uartchain_end_send();
		//printf("end1\n");
		if(delay)
		{
			//timer_mdelay(100);
			timer_udelay(60);
		}
	}

	return 0;
}

int recv(void)
{
	char r[] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};
	char recv;
	int m = 0, n = 0;

	printf("recv\n");

	//while(1)
	for(int j = 0; j < 90000; j++)
	{
		for(int i = 0; i < sizeof(r); i++)
		{
			m = 0;
			while(!uartchain_tstc())
			{
				m++;
				if((m % 900000) == 0)
				{
					printf("r%d\n", i);
				}
			}
			recv = uartchain_getc();
			if (recv != r[i])
			{
				//printf("Error: recv 0x%02x, expect 0x%02x\n", (unsigned int)recv, r[i]);
				i++;
				n++;
			}
			//printf("0x%x\n", (unsigned int)recv);
		}
	}
	printf("%d\n", n);

	return 0;
}

void send_dma(void)
{
	char s[] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};
	//char recv;

	printf("send dma\n");
	uartdma_init();

	while(1)
	{
		for(int i = 0; i < sizeof(s); i++)
		{
			uartdma_putc(s[i]);
			//recv = uartdma_getc();
			//printf("0x%x\n", (unsigned int)recv);
		}
	}

}

void recv_dma(void)
{
	char r[] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};
	char recv;
	int m = 0;

	printf("recv mda\n");
	uartdma_init();

	//while(1)
	for(int j = 0; j < 90000; j++)
	{
		for(int i = 0; i < sizeof(r); i++)
		{
			recv = uartdma_getc();
			if (recv != r[i])
			{
				//printf("Error: recv 0x%02x, expect 0x%02x\n", (unsigned int)recv, r[i]);
				i++;
				m++;
			}
			//printf("0x%x\n", (unsigned int)recv);
		}
	}
	printf("%d\n", m);
}

int trans(int set)
{
	char s[] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};
	char r[] = {0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79};
	char s1[] = {0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59};
	char recv;

	printf("trans\n");

	while(1)
	{
		uartchain_start_send();
		for(int i = 0; i < sizeof(s); i++)
		{
			if(set)
				uartchain_putc(s1[i]);
			else
				uartchain_putc(s[i]);
			//timer_udelay(1);
		}
		uartchain_end_send();

		for(int i = 0; i < sizeof(r); i++)
		{
			recv = uartchain_getc();
			if (recv != r[i])
			{
				printf("Error: recv 0x%02x, expect 0x%02x\n", (unsigned int)recv, r[i]);
				i++;
			}
			printf("%x ", (unsigned int)recv);
		}
		printf("\n");
	}

	return 0;
}

static int do_send(int argc, char **argv)
{
	
	if(argv[1][0] == 'b')
	{
		printf("bottom ");
		uartchain_init();
	}
	else if(argv[1][0] == 'd')
	{
		send_dma();
		return 0;
	}
	else
	{
		printf("top ");
		uartchain_init_top();
	}

	return send(atoi(argv[2]), atoi(argv[3]));
}

static int do_recv(int argc, char **argv)
{
	if(argv[1][0] == 'b')
	{
		printf("bottom ");
		uartchain_init();
	}
	else if(argv[1][0] == 'd')
	{
		recv_dma();
	}
	else
	{
		printf("top ");
		uartchain_init_top();
	}

	return recv();
}

static int do_idle(int argc, char **argv)
{
	printf("bottom idle\n");
	uartchain_init();
	return 0;
}

static int do_trans(int argc, char **argv)
{
	
	if(argv[1][0] == 'b')
	{
		printf("bottom ");
		uartchain_init();
	}
	else
	{
		printf("top ");
		uartchain_init_top();
	}

	return trans(atoi(argv[2]));
}

static struct cmd_entry test_cmd_list[] __attribute__ ((unused)) = {
  {"send", do_send, 3, "t/b/d 0/1/2/3/4[msg set] 0/1[delay]"},
  {"recv", do_recv, 1, "t/b/d"},
  {"bidle", do_idle, 0, ""},
  {"trans", do_trans, 2, "t/b 0/1"},
  {NULL, NULL, 0, NULL}
};

int testcase_uart(void)
{
	int ret = 0;

	uartlog("%s\n", __func__);

#if 1
	command_add(&test_cmd_list[0]);
	command_add(&test_cmd_list[1]);
	command_add(&test_cmd_list[2]);
	command_add(&test_cmd_list[3]);

	cli_simple_loop();
#else
	send();
#endif

	return ret;
}

int testcase_uartchain(void)
{
	int ret = testcase_uart();
	uartlog("testcase uart %s\n", ret?"failed":"passed");
	return ret;
}

//module_testcase("1", testcase_uartchain);

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
	return testcase_uartchain();
}
#endif
