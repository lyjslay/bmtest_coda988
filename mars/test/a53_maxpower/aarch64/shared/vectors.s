//-----------------------------------------------------------------------------
// The confidential and proprietary information contained in this file may
// only be used by a person authorised under and to the extent permitted
// by a subsisting licensing agreement from ARM Limited.
//
//            (C) COPYRIGHT 2013-2014 ARM Limited.
//                ALL RIGHTS RESERVED
//
// This entire notice must be reproduced on all copies of this file
// and copies of this file may only be made by a person if such person is
// permitted to do so under the terms of a subsisting license agreement
// from ARM Limited.
//
//      SVN Information
//
//      Checked In          : $Date: 2014-07-02 16:52:21 +0100 (Wed, 02 Jul 2014) $
//
//      Revision            : $Revision: 283836 $
//
//      Release Information : CORTEXA53-r0p4-00rel1
//
//-----------------------------------------------------------------------------

        .section vectors, "ax", %progbits

        .global  vector_table

        // Weakly-import test-defined handlers.  The weak import means that if
        // a test does not define the handler then the default one will be
        // used.
        .weak curr_el_sp0_sync_vector
        .weak curr_el_sp0_irq_vector
        .weak curr_el_sp0_fiq_vector
        .weak curr_el_sp0_serror_vector
        .weak curr_el_spx_sync_vector
        .weak curr_el_spx_irq_vector
        .weak curr_el_spx_fiq_vector
        .weak curr_el_spx_serror_vector
        .weak lower_el_aarch64_sync_vector
        .weak lower_el_aarch64_irq_vector
        .weak lower_el_aarch64_fiq_vector
        .weak lower_el_aarch64_serror_vector
        .weak lower_el_aarch32_sync_vector
        .weak lower_el_aarch32_irq_vector
        .weak lower_el_aarch32_fiq_vector
        .weak lower_el_aarch32_serror_vector

        // Start of vectors
        .balign 0x800

        // Current EL with SP0
vector_table:
curr_el_sp0_sync:
        // Branch to the weakly-imported test-defined handler.  If the test
        // does not define a handler then this instruction behaves like a NOP.
        b       curr_el_sp0_sync_vector

        // Default code when a test does not define a replacement handler
        mov     x0, #0x13000000 // Tube address
        adr     x1, 1f
        bl      print
        b       terminate

1:      .asciz "Unexpected exception: curr_el_sp0_sync\n"

        // Each vector occupies a 128-byte space, aligned on a 128-byte
        // boundary.  The following directives provide an assembly-time check
        // that the instructions used in the vector do not exceed 128 bytes.
        .if (. > (curr_el_sp0_sync + 0x80))
        .error "curr_el_sp0_sync vector too large. Maximum is 128 bytes."
        .endif

        // Ensure next vector is aligned to the correct boundary
        .balign 0x80
curr_el_sp0_irq:
        b       curr_el_sp0_irq_vector
        mov     x0, #0x13000000 // Tube address
        adr     x1, 1f
        bl      print
        b       terminate

1:      .asciz "Unexpected exception: curr_el_sp0_irq\n"

        .if (. > (curr_el_sp0_irq + 0x80))
        .error "curr_el_sp0_irq vector too large. Maximum is 128 bytes."
        .endif

        .balign 0x80
curr_el_sp0_fiq:
        b       curr_el_sp0_fiq_vector
        mov     x0, #0x13000000 // Tube address
        adr     x1, 1f
        bl      print
        b       terminate

1:      .asciz "Unexpected exception: curr_el_sp0_fiq\n"

        .if (. > (curr_el_sp0_fiq + 0x80))
        .error "curr_el_sp0_fiq vector too large. Maximum is 128 bytes."
        .endif

        .balign 0x80
curr_el_sp0_serror:
        b       curr_el_sp0_serror_vector
        mov     x0, #0x13000000 // Tube address
        adr     x1, 1f
        bl      print
        b       terminate

1:      .asciz "Unexpected exception: curr_el_sp0_serror\n"

        .if (. > (curr_el_sp0_serror + 0x80))
        .error "curr_el_sp0_serror vector too large. Maximum is 128 bytes."
        .endif

        .balign 0x80
        // Current EL with SPx
curr_el_spx_sync:
        b       curr_el_spx_sync_vector
        mov     x0, #0x13000000 // Tube address
        adr     x1, 1f
        bl      print
        b       terminate

1:      .asciz "Unexpected exception: curr_el_spx_sync\n"

        .if (. > (curr_el_spx_sync + 0x80))
        .error "curr_el_spx_sync vector too large. Maximum is 128 bytes."
        .endif

        .balign 0x80
curr_el_spx_irq:
        b       curr_el_spx_irq_vector
        mov     x0, #0x13000000 // Tube address
        adr     x1, 1f
        bl      print
        b       terminate

1:      .asciz "Unexpected exception: curr_el_spx_irq\n"

        .if (. > (curr_el_spx_irq + 0x80))
        .error "curr_el_spx_irq vector too large. Maximum is 128 bytes."
        .endif

        .balign 0x80
curr_el_spx_fiq:
        b       curr_el_spx_fiq_vector
        mov     x0, #0x13000000 // Tube address
        adr     x1, 1f
        bl      print
        b       terminate

1:      .asciz "Unexpected exception: curr_el_spx_fiq\n"

        .if (. > (curr_el_spx_fiq + 0x80))
        .error "curr_el_spx_fiq vector too large. Maximum is 128 bytes."
        .endif

        .balign 0x80
curr_el_spx_serror:
        b       curr_el_spx_serror_vector
        mov     x0, #0x13000000 // Tube address
        adr     x1, 1f
        bl      print
        b       terminate

1:      .asciz "Unexpected exception: curr_el_spx_serror\n"

        .if (. > (curr_el_spx_serror + 0x80))
        .error "curr_el_spx_serror vector too large. Maximum is 128 bytes."
        .endif

        .balign 0x80
        // Lower EL using AArch64
lower_el_aarch64_sync:
        b       lower_el_aarch64_sync_vector
        mov     x0, #0x13000000 // Tube address
        adr     x1, 1f
        bl      print
        b       terminate

1:      .asciz "Unexpected exception: lower_el_aarch64_sync\n"

        .if (. > (lower_el_aarch64_sync + 0x80))
        .error "lower_el_aarch64_sync vector too large. Maximum is 128 bytes."
        .endif

        .balign 0x80
lower_el_aarch64_irq:
        b       lower_el_aarch64_irq_vector
        mov     x0, #0x13000000 // Tube address
        adr     x1, 1f
        bl      print
        b       terminate

1:      .asciz "Unexpected exception: lower_el_aarch64_irq\n"

        .if (. > (lower_el_aarch64_irq + 0x80))
        .error "lower_el_aarch64_irq vector too large. Maximum is 128 bytes."
        .endif

        .balign 0x80
lower_el_aarch64_fiq:
        b       lower_el_aarch64_fiq_vector
        mov     x0, #0x13000000 // Tube address
        adr     x1, 1f
        bl      print
        b       terminate

1:      .asciz "Unexpected exception: lower_el_aarch64_fiq\n"

        .if (. > (lower_el_aarch64_fiq + 0x80))
        .error "lower_el_aarch64_fiq vector too large. Maximum is 128 bytes."
        .endif

        .balign 0x80
lower_el_aarch64_serror:
        b       lower_el_aarch64_serror_vector
        mov     x0, #0x13000000 // Tube address
        adr     x1, 1f
        bl      print
        b       terminate

1:      .asciz "Unexpected exception: lower_el_aarch64_serror\n"

        .if (. > (lower_el_aarch64_serror + 0x80))
        .error "lower_el_aarch64_serror vector too large. Maximum is 128 bytes."
        .endif

        .balign 0x80
        // Lower EL using AArch32
lower_el_aarch32_sync:
        b       lower_el_aarch32_sync_vector
        mov     x0, #0x13000000 // Tube address
        adr     x1, 1f
        bl      print
        b       terminate

1:      .asciz "Unexpected exception: lower_el_aarch32_sync\n"

        .if (. > (lower_el_aarch32_sync + 0x80))
        .error "lower_el_aarch32_sync vector too large. Maximum is 128 bytes."
        .endif

        .balign 0x80
lower_el_aarch32_irq:
        b       lower_el_aarch32_irq_vector
        mov     x0, #0x13000000 // Tube address
        adr     x1, 1f
        bl      print
        b       terminate

1:      .asciz "Unexpected exception: lower_el_aarch32_irq\n"

        .if (. > (lower_el_aarch32_irq + 0x80))
        .error "lower_el_aarch32_irq vector too large. Maximum is 128 bytes."
        .endif

        .balign 0x80
lower_el_aarch32_fiq:
        b       lower_el_aarch32_fiq_vector
        mov     x0, #0x13000000 // Tube address
        adr     x1, 1f
        bl      print
        b       terminate

1:      .asciz "Unexpected exception: lower_el_aarch32_fiq\n"

        .if (. > (lower_el_aarch32_fiq + 0x80))
        .error "lower_el_aarch32_fiq vector too large. Maximum is 128 bytes."
        .endif

        .balign 0x80
lower_el_aarch32_serror:
        b       lower_el_aarch32_serror_vector
        mov     x0, #0x13000000 // Tube address
        adr     x1, 1f
        bl      print
        b       terminate

1:      .asciz "Unexpected exception: lower_el_aarch32_serror\n"

        .if (. > (lower_el_aarch32_serror + 0x80))
        .error "lower_el_aarch32_serror vector too large. Maximum is 128 bytes."
        .endif

        .balign 0x80

//------------------------------------------------------------------------------
// End of vectors
//------------------------------------------------------------------------------

// Print a string to the tube
//    Expects: x0 -> tube
//             x1 -> message
//    Modifies x2
.ifdef ORIG_VECTORS
print:          ldrb    w2, [x1], #1
                cbz     w2, 1f
                strb    w2, [x0]
                b       print
1:              ret
.else
print:          ret
.endif

// Print failure message and terminate simulation.
// Assumes x0 points to the tube
.ifdef ORIG_VECTORS
terminate:      adr     x1, fail_str
                bl      print
                mov     w2, #0x4  // EOT character
                strb    w2, [x0]  // Terminate simulation
                b       .
.else
terminate:      //ldr     x0, =0xffffff00
                ldr     x0, =0x04000020         // Shared region
                ldr     x1, =0x0b0b0b0b
                str     x1, [x0]
                b       .
.endif

// Failure string for unexpected exceptions
                .balign 4
fail_str:       .asciz "** TEST FAILED**\n"
                .end
