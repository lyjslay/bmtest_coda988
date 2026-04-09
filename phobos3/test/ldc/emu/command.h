#ifndef _COMMAND_H_
#define _COMMAND_H_

#define u8  uint8_t
#define u16  uint16_t
#define u32  uint32_t
#define u64  uint64_t

#define uartlog     printf
#define uart_puts(x) printf("%s",x)
#define uart_putc(x) 
#define timer_get_tick() clock()
#define TIMER_CNT_MS 1000

#define mdelay(x)   sleep(x)
#define request_irq(a,b,c,d,f)

typedef int (*cmd_func_t)(int argc, char **argv);

struct cmd_entry {
  char *name;
  cmd_func_t func;
  int arg_count;
  char *usage;
};

int command_main(int argc, char **argv);

int command_add(struct cmd_entry *cmd);

#endif
