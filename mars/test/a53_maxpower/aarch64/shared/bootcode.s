//-----------------------------------------------------------------------------
// The confidential and proprietary information contained in this file may
// only be used by a person authorised under and to the extent permitted
// by a subsisting licensing agreement from ARM Limited.
//
//            (C) COPYRIGHT 2013-2016 ARM Limited.
//                ALL RIGHTS RESERVED
//
// This entire notice must be reproduced on all copies of this file
// and copies of this file may only be made by a person if such person is
// permitted to do so under the terms of a subsisting license agreement
// from ARM Limited.
//
//      SVN Information
//
//      Checked In          :  $
//
//      Revision            :  $
//
//      Release Information : CORTEXA53-r0p4-00rel1
//
//-----------------------------------------------------------------------------


                .section boot, "ax", %progbits
                .global bootcode
                .global irq_init

// Load stack definitions
.include "boot_defs.hs"

.equ PLAT_BM_TRUSTED_MAILBOX_BASE, 0x04000000
.equ PLAT_BM_HOLD_BASE,            (PLAT_BM_TRUSTED_MAILBOX_BASE + 8)


//-------------------------------------------------------------------------------
// Generic boot code
//-------------------------------------------------------------------------------

bootcode:
                // Disable I/D cache
                mrs     x0, sctlr_el3
                bic     x0, x0, #4096           // Icache
                bic     x0, x0, #4              // Dcache
                msr     sctlr_el3, x0
                isb

                // Clear mailbox region
                mov     x2,  xzr
                ldr     x3, =0x04000000         // secondary entrypoint
                str     x2, [x3]
                ldr     x3, =0x04000008         // core 0 status
                str     x2, [x3]
                ldr     x3, =0x04000000         // core 1 status
                str     x2, [x3]

                // Initialise the register bank
                mov     x0,  xzr
                mov     x1,  xzr
                mov     x2,  xzr
                mov     x3,  xzr
                mov     x4,  xzr
                mov     x5,  xzr
                mov     x6,  xzr
                mov     x7,  xzr
                mov     x8,  xzr
                mov     x9,  xzr
                mov     x10, xzr
                mov     x11, xzr
                mov     x12, xzr
                mov     x13, xzr
                mov     x14, xzr
                mov     x15, xzr
                mov     x16, xzr
                mov     x17, xzr
                mov     x18, xzr
                mov     x19, xzr
                mov     x20, xzr
                mov     x21, xzr
                mov     x22, xzr
                mov     x23, xzr
                mov     x24, xzr
                mov     x25, xzr
                mov     x26, xzr
                mov     x27, xzr
                mov     x28, xzr
                mov     x29, xzr
                mov     x30, xzr

                // Zero the stack pointers, link registers and status registers
                mov     sp,       x0
                msr     sp_el0,   x0
                msr     sp_el1,   x0
                msr     sp_el2,   x0
                msr     elr_el1,  x0
                msr     elr_el2,  x0
                msr     elr_el3,  x0
                msr     spsr_el1, x0
                msr     spsr_el2, x0
                msr     spsr_el3, x0

                // Initialise vector base address register for EL3
                adr     x1, vector_table
                msr     vbar_el3, x1

                // Initialize the stack pointer
                #adr     x1, stack_top
                ldr     x1, =stack_top
                add     x1, x1, #4
                mrs     x2, mpidr_el1
                and     x2, x2, #0xFF   // x2 == CPU number
                mov     x3, #CPU_STACK_SIZE
                mul     x3, x2, x3
                sub     x1, x1, x3
                mov     sp, x1

                // Enable NEON and initialize the register bank if this
                // feature has been implemented
                mrs     x0, ID_AA64PFR0_EL1
                ubfx    x0, x0, #16, #4         // Extract the floating-point field (x0 == 0x0 if present)
                //cbnz    x0, ttb_setup           // Skip FP initialization if not present

                mov     x1, #(0x3 << 20)
                msr     cpacr_el1, x1
                isb     sy

                fmov    d0,  xzr
                fmov    d1,  xzr
                fmov    d2,  xzr
                fmov    d3,  xzr
                fmov    d4,  xzr
                fmov    d5,  xzr
                fmov    d6,  xzr
                fmov    d7,  xzr
                fmov    d8,  xzr
                fmov    d9,  xzr
                fmov    d10, xzr
                fmov    d11, xzr
                fmov    d12, xzr
                fmov    d13, xzr
                fmov    d14, xzr
                fmov    d15, xzr
                fmov    d16, xzr
                fmov    d17, xzr
                fmov    d18, xzr
                fmov    d19, xzr
                fmov    d20, xzr
                fmov    d21, xzr
                fmov    d22, xzr
                fmov    d23, xzr
                fmov    d24, xzr
                fmov    d25, xzr
                fmov    d26, xzr
                fmov    d27, xzr
                fmov    d28, xzr
                fmov    d29, xzr
                fmov    d30, xzr
                fmov    d31, xzr

                // Create page tables
ttb_setup:      adr     x0, ttb0_base
                msr     ttbr0_el3, x0

                ldr     w1, =0x80803520
                msr     TCR_EL3, x1
                ldr     x1, mair_value
                msr     MAIR_EL3, x1

		// Set SMPEN before enabling caches and MMU
                mrs     x0, S3_1_C15_C2_1	// CPUECTLR_EL1
		orr	x0, x0, #0x40		// SMPEN
		msr	S3_1_C15_C2_1, x0

                // Enable caches and MMU
                mrs     x0, sctlr_el3
                orr     x0, x0, #(0x1 << 2)     // C bit (data cache)
                orr     x0, x0, #(0x1 << 12)    // I bit (instruction cache)
                orr     x0, x0, #0x1            // M bit (MMU)
                msr     sctlr_el3, x0

                // Enable interrupts
                msr     DAIFClr, #0xF

                // Configure FIQ/IRQ to be taken at EL3 by setting SCR.FIQ and SCR.IRQ.
                // This allows a FIQ to wake CPUs that wait in WFI in EL3
                // at the end of the boot code. Also enable CPU to take IRQ exception
                mrs     x0, scr_el3
                orr     x0, x0, #(1<<2)  // FIQ bit
                orr     x0, x0, #(1<<1)  // IRQ bit
                msr     scr_el3, x0

                // Ensure all writes to system registers have taken place
                dsb     sy
                isb     sy

                // Check if this is a primary CPU
                mrs     x0, mpidr_el1
                and     x0, x0, #0xFF           // x0 == CPU number
                cbnz    x0, 4f

1:
                // Primary core
                // Write address
                ldr     x2, =bootcode
                ldr     x3, =0x04000000         // PLAT_BM_TRUSTED_MAILBOX_BASE
                str     x2, [x3]

                // Notify secondary core
                mov     x2, #1
                ldr     x3, =0x04000010         // PLAT_BM_HOLD_BASE
                str     x2, [x3]
/*
                // Polling mailbox
2:
                ldr     x3, =0x04000008         // core 0 status
                ldr     x1, [x3]
                cbz     x1, 3f
                b       cpu_start               // Ready to go
3:
                b       2b
*/
                b       cpu_start

                .ifdef IK_MODE
                // All CPUs start the test code. The select_cpu function in the integration kit
                // will determine which CPU run
                b       cpu_start
                .else
                // Only CPU0 starts the test code.  Other CPUs sleep and will be enabled
                // in individual tests where required.
                mrs     x0, mpidr_el1
                and     x0, x0, #0xFF           // x0 == CPU number
                cbz     x0, cpu_start
                .endif

4:
                // Notify primary core
                mov     x2, 1
                ldr     x3, =0x04000008         // core 0 status
                str     x2, [x3]

                // Secondary core
                b       cpu_start               // Ready to go

/*
                // If the CPU is not CPU0 then enter WFI
wfi_loop:       wfi
                b       wfi_loop
*/

//-------------------------------------------------------------------------------
// Start the test
//-------------------------------------------------------------------------------
                // If IK_MODE is set, all CPUs can reach this cpu_start label.
                // If IK_MODE is not set (which is default for execution TB), only
                // CPU0 reaches this label (the others are in WFI).  The label
                // for the start of the test depends on whether it's a C or an
                // assember test.  Weakly import the labels for each and to
                // determine the correct one.  Note that the linker translates
                // any branches to non-existant weakly-imported labels to NOPs.
cpu_start:
                .weak _arm_start
                .weak test_start

                b       _arm_start
                b       test_start

                .end
                .balign 4
