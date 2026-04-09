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
//      Checked In          : $Date: 2016-04-29 14:18:08 +0100 (Fri, 29 Apr 2016) $
//
//      Revision            : $Revision: 340884 $
//
//      Release Information : CORTEXA53-r0p4-00rel1
//
//-------------------------------------------------------------------------------
// Description:
//
//   Maximum power test
//-------------------------------------------------------------------------------

                .section testcode, "ax", %progbits

                .global test_start

.equ NUM_ITERS, 50

.equ INTRID_WDT, 101

//-------------------------------------------------------------------------------
// FIQ handler
//
//   This test uses a FIQ to release CPUs from WFI.  The FIQ handler simply
//   clears the FIQ using the trickbox and returns.
//-------------------------------------------------------------------------------

// Make fiq_handler visible to linker so that it can override the default handler
.global curr_el_spx_fiq_vector

curr_el_spx_fiq_vector:
                mrs     x20, mpidr_el1          // Read MPIDR
                tst     w20, #0xFF
                b.ne    fiq_loop

.ifdef ORIG_MAX_POWER
                //ldr     x20, =0x13000000         // Trickbox base
.else
                //ldr     x20, =0xffffff00         // Trickbox base
.endif
                ldr     x20, =0x04000020         // Shared region

                str     w20, [x20, #0x10]        // Clear FIQ (nb. write value ignored)

                // Poll the ISR register until the FIQ has cleared
fiq_loop:       mrs     x21, isr_el1             // r21 == ISR
                tbnz    x21, #6, fiq_loop

                // Return using ERET.  This test uses WFI spin code (a WFI
                // instruction followed by a branch back to the WFI) to force
                // the CPUs to remain idle until the test breaks them out of the
                // WFI spin.  For asynchronous exceptions such as FIQ, the ELR
                // points to the first instruction that has not been fully
                // executed, which in the WFI spin loop is the branch back to
                // the WFI instruction.  Therefore we add 4 to the ELR before
                // returning, so that we break out of the loop and execute the
                // first instruction following the loop.
                mrs     x21, elr_el3
                add     x21, x21, #4
                msr     elr_el3, x21
                eret


//-------------------------------------------------------------------------------
// Main test code
//-------------------------------------------------------------------------------

test_start:
                mrs     x8, mpidr_el1           // Read MPIDR
                ands    x8, x8, #0xFF           // r8 == CPU number
                b.ne    setup

                // Set up a FIQ to wake all CPUs from WFI.  When the other CPUs
                // wake they will branch to setup, so the following code
                // is only executed by CPU0.
.ifdef ORIG_MAX_POWER
                //ldr     x9, =0x13000000         // Trickbox base
.else
                //ldr     x9, =0xffffff00         // Trickbox base
.endif

                ldr     x9, =0x04000020         // Shared region

                mov     w10, #0x400             // Counter value
                .ifdef BIGEND
                rev     w10, w10
                .endif
                str     w10, [x9, #0x8]         // Store to FIQ counter

                // CPU0 waits here for the interrupt.  Other CPUs are already
                // waiting in WFI in the boot code.  After the interrupt all CPUs
                // will execute the setup code below (CPUs 1-3 will first branch
                // to test_start and then the setup code).
//cpu0_wait:      wfi
//                b       cpu0_wait

setup:
                // Mask FIQ interrupts, so that the interrupt will wake the core
                // from WFI, but not cause an exception
                msr     daifset, #1

                // Enable debug then enable all breakpoint and watchpoint
                // comparators.  Enabling breakpoints and watchpoints
                // increases power consumption for the test because the
                // comparators have to compare addresses against the breakpoint
                // and watchpoint registers.

                // Enable halting debug by setting the HDE bit of the MSDCR
                // and then clear the OS lock to enable programming of the
                // breakpoint and watchpoint registers.
                mrs     x0, mdscr_el1
                orr     x0, x0, #(0x1 << 14)    // HDE bit
                msr     mdscr_el1, x0
                mov     x0, #0
                msr     oslar_el1, x0           // Clear OS lock
                isb

                // Set breakpoint and watchpoint value registers
                adr     x0, bkpt0
                msr     dbgbvr0_el1, x0   // Write DBGBVR0
                adr     x0, bkpt1
                msr     dbgbvr1_el1, x0   // Write DBGBVR1
                adr     x0, bkpt2
                msr     dbgbvr2_el1, x0   // Write DBGBVR2
                adr     x0, bkpt3
                msr     dbgbvr3_el1, x0   // Write DBGBVR3
                adr     x0, bkpt4
                msr     dbgbvr4_el1, x0   // Write DBGBVR4
                adr     x0, bkpt5
                msr     dbgbvr5_el1, x0   // Write DBGBVR5
                adr     x0, wpt0
                msr     dbgwvr0_el1, x0   // Write DBGWVR0
                adr     x0, wpt1
                msr     dbgwvr1_el1, x0   // Write DBGWVR1
                adr     x0, wpt2
                msr     dbgwvr2_el1, x0   // Write DBGWVR2
                adr     x0, wpt3
                msr     dbgwvr3_el1, x0   // Write DBGWVR3

                // Set breakpoint and watchpoint enable registers to enable
                // in all states.  The value written to the DBGBCRn registers
                // is:
                //   Bit[13]   (HMC) - 1'b1    - State matching (match all states)
                //   Bits[8:5] (BAS) - 4'b1111 - Match A32/A64 instructions
                //   Bits[2:1] (PMC) - 2'b11   - State matching (match all states)
                //   Bit[0]    (E)   - 1'b1    - Enable this breakpoint
                mov     x0, #0x21E7       // DBGBCRn value
                msr     dbgbcr0_el1, x0   // Write DBGBCR0
                msr     dbgbcr1_el1, x0   // Write DBGBCR1
                msr     dbgbcr2_el1, x0   // Write DBGBCR2
                msr     dbgbcr3_el1, x0   // Write DBGBCR3
                msr     dbgbcr4_el1, x0   // Write DBGBCR4
                msr     dbgbcr5_el1, x0   // Write DBGBCR5

                // The value written to the DBGWCRn registers is:
                //   Bit[13]    (HMC) - 1'b1  - State matching (match all states)
                //   Bits[12:5] (BAS) - 8'hFF - Watch all byte lanes
                //   Bits[2:1]  (PAC) - 2'b11 - State matching (match all states)
                //   Bit[0]     (E)   - 1'b1  - Enable this watchpoint
                mov     x0, #0x3FE7       // DBGWCRn value
                msr     dbgwcr0_el1, x0   // Write DBGWCR0
                msr     dbgwcr1_el1, x0   // Write DBGWCR1
                msr     dbgwcr2_el1, x0   // Write DBGWCR2
                msr     dbgwcr3_el1, x0   // Write DBGWCR3

                // Set initial register values
.ifndef ORIG_MAX_POWER
                ldr     x0, =0xFFFFFFFF55555555
                ldr     x1, =0x0000000055555555
                ldr     x2, =0xFFFFFFFDAAAAAAAA
                ldr     x3, =0x00000000AAAAAAAA
                ldr     x4, =0xFFFFFFFF55555555
                ldr     x5, =0x0000000055555555
                ldr     x6, =0xFFFFFFFDAAAAAAAA
                ldr     x7, =0x00000000AAAAAAAA
.else
                ldr     x0, =0xFFFFFFFF55555550
                ldr     x1, =0x0000000055555550
                ldr     x2, =0xFFFFFFFDAAAAAAAA
                ldr     x3, =0x00000000AAAAAAAA
                ldr     x4, =0xFFFFFFFF55555550
                ldr     x5, =0x0000000055555550
                ldr     x6, =0xFFFFFFFDAAAAAAAA
                ldr     x7, =0x00000000AAAAAAAA
.endif

                ldr     x12, =0x04000049  // sram shared region
                ldr     x13, =0x04000062  // sram shared region



                movi v1.16b, #0x66  // q1 = 0x66666666
                movi v2.16b, #0xAA  // q2 = 0xAAAAAAAA
                movi v3.16b, #0x66  // q3 = 0x66666666
                movi v4.16b, #0xAA  // q4 = 0xAAAAAAAA
                movi v5.16b, #0x66  // q5 = 0x66666666
                movi v6.16b, #0xAA  // q6 = 0xAAAAAAAA
                movi v7.16b, #0x66  // q7 = 0x66666666
                movi v8.16b, #0xAA  // q8 = 0xAAAAAAAA
                movi v9.16b, #0x66  // q9 = 0x66666666

                // Now CPU0 programs the trickbox to schedule another FIQ and
                // all CPUs enter WFI.  The CPUs will then all wake and execute
                // the power loop at the same time.
                cbnz    x8, finish_setup
                str     w10, [x9, #0x8]         // Setup FIQ counter as before

finish_setup:
                // Finish setting up registers before entering WFI
                ldr     x9,  =0xFFFFFFFD55555555
                ldr     x10, =0x0000000155555555
                //mov     w14, #2
                ldr     w14, =0xFFFFFFFFFFFFFFFF
                b       outer_loop

//-------------------------------------------------------------------------------
// Main power loop
//
// The intent is for the add/sub instructions to cause all input bits to change,
// yet create a result that can be used as a pointer within a 4GB address space.
// These instructions, along with the load register-offset shifted, run along
// behind the FMLA.
//-------------------------------------------------------------------------------

                // Align the loop to an 8-byte boundary so that two instructions
                // will be in the same instruction cache data bank.  This allows
                // us to dual-issue all pairs of instructions in the loop for
                // maximum power in the pipeline.
                .balign 8
outer_loop:
.ifdef ORIG_MAX_POWER
                //ldr     x8, =0x13000000        // Trickbox base
.else
                //ldr     x8, =0xffffff00        // Trickbox base
.endif
                ldr     x8, =0x04000020         // Shared region

//                wfi
                str     w8, [x8, #0x10]        // Clear FIQ (nb. write value ignored)
                mov     x11, #NUM_ITERS

power_loop:
                // Group 1
                fmla      v1.4s, v2.4s, v3.4s  // Dual Issue Pair 0, Instruction 0
                //ldr       w8, [x0, x1, lsl #2] // Dual Issue Pair 0, Instruction 1 // x0 = 0xFFFFFFFF55555555
                                                                                   // x1 = 0x0000000055555555
                                                                                   // 0x100000000aaaaaaa9
                ldr       w8, [x12]

                //ldr       w8, [x2, x3, lsl #2] // Dual Issue Pair 1, Instruction 0 // x2 = 0xFFFFFFFDAAAAAAAA
                                                                                   // x3 = 0x00000000AAAAAAAA
                                                                                   // 0x10000000055555552
                ldr       w8, [x13]

                add       x0, x1, x9           // Dual Issue Pair 1, Instruction 1 // x1 = 0x0000000055555555
                                                                                   // x9 = 0xFFFFFFFD55555555
                                                                                   // x0 = 0xfffffffdaaaaaaaa      <= x0

                sub       x2, x3, x10          // Dual Issue Pair 2, Instruction 0 // x3 = 0x00000000AAAAAAAA
                                                                                   // x10 = 0x0000000155555555
                                                                                   // x2 = -0xaaaaaaab             <= x2

                //ldr       w8, [x4, x5, lsl #2] // Dual Issue Pair 2, Instruction 1 // x4 = 0xFFFFFFFF55555555
                                                                                   // x5 = 0x0000000055555555
                                                                                   // 0x100000000aaaaaaa9
                ldr       w8, [x12]

                //ldr       w8, [x6, x7, lsl #2] // Dual Issue Pair 3, Instruction 0 // x6 = 0xFFFFFFFDAAAAAAAA
                                                                                   // x7 = 0x00000000AAAAAAAA
                                                                                   // 0x10000000055555552
                ldr        w8, [x13]

                add       x4, x9, x5           // Dual Issue Pair 3, Instruction 1 // x9 = 0xFFFFFFFD55555555
                                                                                   // x5 = 0x0000000055555555
                                                                                   // x4 = 0xfffffffdaaaaaaaa      <= x4

                // Group 2
                fmla      v4.4s, v5.4s, v6.4s  // Dual Issue Pair 0, Instruction 0
                //ldr       w8, [x0, x3, lsl #2] // Dual Issue Pair 0, Instruction 1 // x0 = 0xfffffffdaaaaaaaa
                                                                                   // x3 = 0x00000000AAAAAAAA
                                                                                   // 0x10000000055555552
                ldr       w8, [x13]

                //ldr       w8, [x2, x1, lsl #2] // Dual Issue Pair 1, Instruction 0 // x2 = -0xaaaaaaab
                                                                                   // x1 = 0x0000000055555555
                                                                                   // 0xaaaaaaa9
                ldr       w8, [x12]

                sub       x0, x3, x10          // Dual Issue Pair 1, Instruction 1 // x3 = 0x00000000AAAAAAAA
                                                                                   // x10 = 0x0000000155555555
                                                                                   // x0 = -0xaaaaaaab             <= x0

                add       x2, x1, x9           // Dual Issue Pair 2, Instruction 0 // x1 = 0x0000000055555555
                                                                                   // x9 = 0xFFFFFFFD55555555
                                                                                   // x2 = 0xfffffffdaaaaaaaa      <= x2

                //ldr       w8, [x4, x7, lsl #2] // Dual Issue Pair 2, Instruction 1 // x4 = 0xfffffffdaaaaaaaa
                                                                                   // x7 = 0x00000000AAAAAAAA
                                                                                   // 0x10000000055555552
                ldr       w8, [x13]

                //ldr       w8, [x6, x7, lsl #2] // Dual Issue Pair 3, Instruction 0 // x6 = 0xFFFFFFFDAAAAAAAA
                                                                                   // x7 = 0x00000000AAAAAAAA
                                                                                   // 0x10000000055555552
                ldr       w8, [x13]

                sub       x4, x7, x10          // Dual Issue Pair 3, Instruction 1 // x7 = 0x00000000AAAAAAAA
                                                                                   // x10 = 0x0000000155555555
                                                                                   // x4 = -0xaaaaaaab             <= x4

                // Group 3
                fmla      v7.4s, v8.4s, v9.4s  // Dual Issue Pair 0, Instruction 0
                //ldr       w8, [x0, x1, lsl #2] // Dual Issue Pair 0, Instruction 1 // x0 = -0xaaaaaaab
                                                                                   // x1 = 0x0000000055555555
                                                                                   // 0xaaaaaaa9
                ldr       w8, [x12]

                //ldr       w8, [x2, x3, lsl #2] // Dual Issue Pair 1, Instruction 0 // x2 = 0xfffffffdaaaaaaaa
                                                                                   // x3 = 0x00000000AAAAAAAA
                                                                                   // 0x10000000055555552
                ldr       w8, [x13]

                add       x0, x1, x9           // Dual Issue Pair 1, Instruction 1 // x1 = 0x0000000055555555
                                                                                   // x9 = 0xFFFFFFFD55555555
                                                                                   // x0 = 0xfffffffdaaaaaaaa      <= x0

                sub       x2, x3, x10          // Dual Issue Pair 2, Instruction 0 // x3 = 0x00000000AAAAAAAA
                                                                                   // x10 = 0x0000000155555555
                                                                                   // x2 = -0xaaaaaaab             <= x2

                //ldr       w8, [x4, x5, lsl #2] // Dual Issue Pair 2, Instruction 1 // x4 = -0xaaaaaaab
                                                                                   // x5 = 0x0000000055555555
                                                                                   // 0xaaaaaaa9
                ldr       w8, [x12]

                //ldr       w8, [x6, x7, lsl #2] // Dual Issue Pair 3, Instruction 0 // x6 = 0xFFFFFFFDAAAAAAAA
                                                                                   // x7 = 0x00000000AAAAAAAA
                                                                                   // 0x10000000055555552
                ldr       w8, [x13]

                add       x4, x9, x5           // Dual Issue Pair 3, Instruction 1 // x9 = 0xFFFFFFFD55555555
                                                                                   // x5 = 0x0000000055555555
                                                                                   // x4 = 0xfffffffdaaaaaaaa     <= x4

                // Group 4
                fmla      v4.4s, v5.4s, v6.4s  // Dual Issue Pair 0, Instruction 0
                //ldr       w8, [x0, x3, lsl #2] // Dual Issue Pair 0, Instruction 1 // x0 = 0xfffffffdaaaaaaaa
                                                                                   // x3 = 0x00000000AAAAAAAA
                                                                                   // 0x10000000055555552
                ldr       w8, [x13]

                //ldr       w8, [x2, x1, lsl #2] // Dual Issue Pair 1, Instruction 0 // x2 = -0xaaaaaaab
                                                                                   // x1 = 0x0000000055555555
                                                                                   // 0xaaaaaaa9
                ldr       w8, [x12]

                sub       x0, x3, x10          // Dual Issue Pair 1, Instruction 1 // x3 = 0x00000000AAAAAAAA
                                                                                   // x10 = 0x0000000155555555
                                                                                   // x0 = -0xaaaaaaab            <= x0

                add       x2, x1, x9           // Dual Issue Pair 2, Instruction 0 // x1 = 0x0000000055555555
                                                                                   // x9 = 0xFFFFFFFD55555555
                                                                                   // x2 = 0xfffffffdaaaaaaaa     <= x2

                //ldr       w8, [x4, x7, lsl #2] // Dual Issue Pair 2, Instruction 1 // x4 = 0xfffffffdaaaaaaaa
                                                                                   // x7 = 0x00000000AAAAAAAA
                                                                                   // 0x10000000055555552
                ldr       w8, [x13]

                //ldr       w8, [x6, x7, lsl #2] // Dual Issue Pair 3, Instruction 0 // x6 = 0xFFFFFFFDAAAAAAAA
                                                                                   // x7 = 0x00000000AAAAAAAA
                                                                                   // 0x10000000055555552
                ldr       w8, [x13]

                sub       x4, x7, x10          // Dual Issue Pair 3, Instruction 1 // x7 = 0x00000000AAAAAAAA
                                                                                   // x10 = 0x0000000155555555
                                                                                   // x4 = -0xaaaaaaab            <= x4

                // End of loop check
                subs    x11, x11, #1
                bne     power_loop

                // Outer loop check
                // Poll the ISR register until the FIQ has cleared
fiq_loop2:      mrs     x21, isr_el1
                tbnz    x21, #6, fiq_loop2

.ifdef ORIG_MAX_POWER
                //ldr     x8, =0x13000000         // Trickbox base
.else
                //ldr     x8, =0xffffff00         // Trickbox base
.endif
                ldr     x8, =0x04000020         // Shared region

                mov     w11, #0x400             // FIQ schedule value
                .ifdef BIGEND
                rev     w11, w11
                .endif
                str     w11, [x8, #0x8]         // Store to FIQ counter

                subs    w14, w14, #1
                bne     outer_loop
//-------------------------------------------------------------------------------
// End of test
//-------------------------------------------------------------------------------

                // All CPUs except CPU0 enter WFI, while CPU0 ends the test
                mrs     x0, mpidr_el1           // Read MPIDR
                and     x0, x0, #0xFF           // r0 == CPU number
                cbnz    x0, end_wfi
.ifdef ORIG_MAX_POWER
                ldr     x0, =pass_msg
                bl      print

end:            ldr     x0, =0x13000000         // Tube address
                mov     w2, #0x04               // EOT
                strb    w2, [x0]
.else
end:            //ldr     x0, =0xffffff00         // a53_testbench_control address
                ldr     x0, =0x04000020         // Shared region
                ldr     w2, =0x0a0a0a0a         // pass successfully
                str     w2, [x0]
1:
                b       1b
.endif
end_wfi:        
//wfi
                b       end_wfi

//-------------------------------------------------------------------------------
// Subroutines
//-------------------------------------------------------------------------------

                // Routine to print a string at the address in r0.
.type print, %function
print:          mov     x1, #0x13000000         // Tube address
                ldrb    w2, [x0], #0x1
                cbz     w2, 1f
                strb    w2, [x1]
                b       print
1:
                mov     w2, #'\n'
                strb    w2, [x1]
                ret                             // Return on NULL character


//-------------------------------------------------------------------------------
// Breakpoint and watchpoint labels
//-------------------------------------------------------------------------------

// These labels are used to set the breakpoint and watchpoint registers.
// There is no real code at these labels and they are never executed: the
// breakpoints and watchpoints are set purely to increase power consumption.
bkpt0:          .word   0
bkpt1:          .word   0
bkpt2:          .word   0
bkpt3:          .word   0
bkpt4:          .word   0
bkpt5:          .word   0

// Align watchpoints to double-word boundaries so that all byte address select
// bits can be set in the watchpoint control register
                .balign 8
wpt0:           .word   0
                .word   0
wpt1:           .word   0
                .word   0
wpt2:           .word   0
                .word   0
wpt3:           .word   0
                .word   0

//-------------------------------------------------------------------------------
// Strings
//-------------------------------------------------------------------------------
                .balign 4
pass_msg:       .asciz "** TEST PASSED OK **"

                .end

