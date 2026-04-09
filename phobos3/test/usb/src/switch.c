/**********************************************************************
 * Copyright (C) 2014-2015 Cadence Design Systems, Inc.
 * All rights reserved worldwide.
 ***********************************************************************
 * switch.c
 * Selecting of USB device application
 *
 ***********************************************************************/

#include <stdio.h>

// example applications

#if TEST_INDEX==1
extern int ctrlApp();
extern void ctrlIsr();
#endif

#if TEST_INDEX==2
extern int bulkAppLB();
extern void bulkIsrLB();
#endif

#if TEST_INDEX==3
extern int multiBulkAppLB();
extern void multiBulkIsrLB();
#endif

#if TEST_INDEX==4
extern int interruptApp();
extern void interruptIsr();
#endif

#if TEST_INDEX==5
extern int isoApp();
extern void isoIsr();
#endif

#if TEST_INDEX==6
extern int mscApp();
extern void mscIsr();
#endif

#if TEST_INDEX==7
extern int hidApp();
extern void hidIsr();
#endif

#if TEST_INDEX==8
extern int micApp();
extern void micIsr();
#endif

#if TEST_INDEX==9
extern int AcmApp();
extern void AcmIsr();
#endif

#if TEST_INDEX==10
extern int bulkAppLB();
extern void bulkIsrLB();
#endif

#if TEST_INDEX==13
extern int devIsoApp();
extern void devIsoIsr;
#endif

void emptyIsr() {
    printf("Please select application\n");
}

int emptyApp() {
    printf("Please select application\n");
    return 1;
}

int (*getApp())() {
#ifndef TEST_INDEX
    return emptyApp;
#endif

#if TEST_INDEX==0
    return emptyApp;
#endif

#if TEST_INDEX==1
    return ctrlApp;
#endif

#if TEST_INDEX==2
    return bulkAppLB;
#endif

#if TEST_INDEX==3
    return multiBulkAppLB;
#endif

#if TEST_INDEX==4
    return interruptApp;
#endif

#if TEST_INDEX==5
    return isoApp;
#endif

#if TEST_INDEX==6
    return mscApp;
#endif

#if TEST_INDEX==7
    return hidApp;
#endif

#if TEST_INDEX==8
    return micApp;
#endif

#if TEST_INDEX==9
    return AcmApp;
#endif

#if TEST_INDEX==10
    return bulkAppLB;
#endif


#if TEST_INDEX==13
    return  devIsoApp;
#endif
}

void (*getIsr())() {
#ifndef TEST_INDEX
    return emptyIsr;

#endif

#if TEST_INDEX==0
    return emptyIsr;
#endif

#if TEST_INDEX==1
    return ctrlIsr;
#endif

#if TEST_INDEX==2
    return bulkIsrLB;
#endif

#if TEST_INDEX==3
    return multiBulkIsrLB;
#endif

#if TEST_INDEX==4
    return interruptIsr;
#endif

#if TEST_INDEX==5
    return isoIsr;
#endif

#if TEST_INDEX==6
    return mscIsr;
#endif

#if TEST_INDEX==7
    return hidIsr;
#endif

#if TEST_INDEX==8
    return micIsr;
#endif

#if TEST_INDEX==9
    return AcmIsr;
#endif

#if TEST_INDEX==10
    return bulkIsrLB;
#endif

#if TEST_INDEX==13
    return devIsoIsr;
#endif
}
