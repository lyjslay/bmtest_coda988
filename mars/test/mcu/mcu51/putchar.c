#include "config.h"

#ifdef CONFIG_PUTCHAR_51_SERIAL

#include <reg51.h>

/*
 * putchar (basic version): expands '\n' into CR LF
 */
char putchar(char c)
{
	if (c == '\n') {
		while (!TI)
			;
		TI = 0;
		SBUF = 0x0d; /* output CR  */
	}
	while (!TI)
		;
	TI = 0;
	return (SBUF = c);
}

#endif