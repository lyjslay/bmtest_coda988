/**********************************************************************
 * Copyright (C) 2014-2015 Cadence Design Systems, Inc.
 * All rights reserved worldwide.
 ***********************************************************************
 * cps_bm.c
 *
 * Sample implementation of Cadence Platform Services for a bare-metal
 * system
 ***********************************************************************/
#ifdef __BARE_METAL__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <cps.h>

/* see cps.h */
uint8_t CPS_UncachedRead8(volatile uint8_t* address) {
    return *address;
}

/* see cps.h */
uint16_t CPS_UncachedRead16(volatile uint16_t* address) {
    return *address;
}

/* see cps.h */
uint32_t CPS_UncachedRead32(volatile uint32_t* address) {
    return *address;
}

/* see cps.h */
void CPS_UncachedWrite8(volatile uint8_t* address, uint8_t value) {
    *address = value;
}

/* see cps.h */
void CPS_UncachedWrite16(volatile uint16_t* address, uint16_t value) {
    *address = value;
}

/* see cps.h */
void CPS_UncachedWrite32(volatile uint32_t* address, uint32_t value) {
    *address = value;
}

/* see cps.h */
void CPS_WritePhysAddress32(volatile uint32_t* location, uint32_t addrValue) {
    *location = addrValue;
}

/* see cps.h */
void CPS_BufferCopy(volatile uint8_t *dst, volatile uint8_t *src, uint32_t size) {
    memcpy((void*)dst, (void*)src, size);
}

/* see cps.h */
uint32_t CPS_InitLock(CPS_LockHandle* lock) {
    return 0;
}

/* see cps.h */
void CPS_FreeLock(CPS_LockHandle lock) {
}

/* see cps.h */
uint32_t CPS_ProbeLocks(uint32_t lockCount) {
    return 0;
}

/* Since this is a bare-metal system, with no MMU in place, we expect that there will be no cache enabled */

void CPS_CacheInvalidate(uintptr_t address, size_t size, uintptr_t devInfo) {
#ifdef TENSILICA
    xthal_dcache_region_invalidate(address, size);
#endif
    return;
}

void CPS_CacheFlush(uintptr_t address, size_t size, uintptr_t devInfo) {
#ifdef TENSILICA
    xthal_dcache_region_writeback(address, size);
#endif
    return;
}

#endif /* __BARE_METAL__ */
