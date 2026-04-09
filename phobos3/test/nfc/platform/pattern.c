/**********************************************************************
 * copyright (C) 2014-2015 Cadence Design Systems
 * All rights reserved.
 ***********************************************************************
 * pattern.c
 *
 * Software to fill, clear and compare data buffers
 ***********************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include "pattern.h"

/*
 * Fill the buffer with a recognizable pattern.
 * @param buf The buffer to be filled
 * @param sizeWords How many 32-bit words to fill
 * @param data_pattern value that should be repeatedly filled into the buffer.
 */
void Clearbuf(void *buffer, uint32_t sizeWords,uint32_t data_pattern) {
    uint32_t *buf = buffer;
    uint32_t *bufEnd = buf + sizeWords;

    while (buf < bufEnd)
	*buf++ = data_pattern; //0xdeadbeef;
}

/**
 * Fill the buffer with an incrementing pattern
 * @param buf The buffer to be filled
 * @param sizeWords How many 32-bit words to fill
 * @param start value for the first word
 */
void Fillbuf(void *buffer, uint32_t sizeWords, uint32_t start) {
    uint32_t *buf = buffer;
    uint32_t *bufEnd = buf + sizeWords;

    srand(start);
    while (buf < bufEnd)
        //*buf++ = rand();
	*buf++ = ++start ;
}

/**
 * Compare two buffers
 * @param buf1 one buffer
 * @param buf2 other buffer
 * @param sizeWords how many (32-bit) words to compare
 * @return non-zero on failure (mis-compare)
 * @return zero on success
 */
uint32_t Comparebuf(void *buffer1, void *buffer2, uint32_t sizeWords) {
    uint32_t *buf1 = buffer1, *buf2 = buffer2;
    uint32_t *buf1_end = buf1 + sizeWords;
    uint32_t idx = 0;

    while (buf1 < buf1_end) {
	if (*buf1 != *buf2){
            printf("%08x: %x != %x\n", idx, *buf1, *buf2);
	    return 1;
        }
        buf1++; buf2++;
        idx += 4;
    }
    return 0;
}

void PrintCmpBuf(void *buffer1, void *buffer2, uint32_t sizeWords) {
    uint32_t *buf1 = buffer1, *buf2 = buffer2;
    uint32_t pos = 0, i = 0;
    printf("---- comparing buffers ----\n");

    while(sizeWords){
	switch(sizeWords){
	case 3:
	    printf("%04x: %08x,%08x,%08x -- %08x,%08x,%08x\n",
		    pos, buf1[i], buf1[i + 1], buf1[i + 2],
		    buf2[i], buf2[i + 1], buf2[i + 2]);
	    sizeWords -= 3;
	    break;
	case 2:
	    printf("%04x: %08x,%08x -- %08x,%08x\n",
		    pos, buf1[i], buf1[i + 1],
		    buf2[i], buf2[i + 1]);
	    sizeWords -= 2;
	    break;
	case 1:
	    printf("%04x: %08x -- %08x\n",
		    pos, buf1[i], buf2[i]);
	    sizeWords -= 1;
	    break;
	default:
	    printf("%04x: %08x,%08x,%08x,%08x -- %08x,%08x,%08x,%08x\n",
		    pos, buf1[i], buf1[i + 1], buf1[i + 2], buf1[i + 3],
		    buf2[i], buf2[i + 1], buf2[i + 2], buf2[i + 3]);
	    sizeWords -= 4;
	    break;
	}
	i += 4;
	pos += 32;
    }
    printf("----        end        ----\n");
}


